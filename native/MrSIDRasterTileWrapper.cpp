#include "MrSIDRasterTileWrapper.h"

#include <cmath>
#include <memory>
#include <utility>
#include <string>
#include <regex>
#include <algorithm>

#include "MrSIDImageReader.h"
#include "lt_fileSpec.h"
#include "lti_geoCoord.h"
#include "lti_utils.h"
#include "lti_pixel.h"
#include "lti_sample.h"
#include "lti_navigator.h"
#include "lti_sceneBuffer.h"

#define MRSID_EXPORT __attribute__ ((visibility("default")))

// Projection code

typedef std::pair<double, double> Point;

Point WMtoWGS84(double x, double y) {
    const double radd = 45.0 / std::atan(1.0);
    const double a = 6378137.0;
    const double halfpi = std::atan(1.0) * 2;

    double lng = x / a;
    double lat = halfpi - (2.0 * std::atan(std::exp((-y) / a)));
    return Point(lng * radd, lat * radd);
}

Point WGS84toUTM(int utmz, double latd, double lngd) {
    const double drad = std::atan(1.0) / 45.0;
    const double k0 = 0.9996;
    const double a = 6378137.0;
    const double f = 1 / 298.2572236;
    const double b = a * (1 - f);
    const double e = std::sqrt(1 - (b / a) * (b / a));
    const double esq = (1 - (b / a) * (b / a));
    const double e0sq = e * e / (1 - e * e);

    int zcm = 3 + 6 * (utmz - 1) - 180;
    double phi = latd * drad;
    double cos_phi = std::cos(phi);
    double sin_phi = std::sin(phi);
    double tan_phi = sin_phi / cos_phi;
    double N = a / std::sqrt(1 - e * e * sin_phi * sin_phi);
    double T = tan_phi * tan_phi;
    double C = e0sq * cos_phi * cos_phi;
    double A = (lngd - zcm) * drad * cos_phi;
    double M0 = 0;
    double M = phi * (1 - esq * (1 / 4.0 + esq * (3 / 64.0 + 5 * esq / 256.0)));
    M = M - std::sin(2 * phi) * (esq * (3 / 8.0 + esq * (3 / 32.0 + 45 * esq / 1024.0)));
    M = M + std::sin(4 * phi) * (esq * esq * (15 / 256.0 + esq * 45 / 1024.0));
    M = M - std::sin(6 * phi) * (esq * esq * esq * (35 / 3072.0));
    M = M * a;
    double x = k0 * N * A * (1 + A * A * ((1 - T + C) / 6.0 + A * A * (5 - 18 * T + T * T + 72 * C - 58 * e0sq) / 120.0));
    x = x + 500000;
    double y = k0 * (M - M0 + N * tan_phi * (A * A * (1 / 2.0 + A * A * ((5 - T + 9 * C + 4 * C * C) / 24.0 + A * A * (61 - 58 * T + T * T + 600 * C - 330 * e0sq) / 720.0))));
    if (y < 0) {
        y = 10000000 + y;
    }
    return Point(x, y);
}

Point reprojectPoint(const double invTransform[], int utmz, const Point& wm) {
    Point wgs = WMtoWGS84(wm.first, wm.second);
    Point utm = WGS84toUTM(utmz, wgs.second, wgs.first);
    return Point((invTransform[0] + utm.first + invTransform[4] * utm.second) * invTransform[2], (invTransform[1] + utm.second + invTransform[5] * utm.first) * invTransform[3]);
}

Point reprojectTilePixel(const double invTransform[], int utmz, int tileZ, int tileX, int tileY, Point pixel) {
    const double pi = std::atan(1.0) * 4;
    const double wmProjBound = 6378137.0 * pi;

    double wmx = -wmProjBound + (2 * wmProjBound * (tileX + pixel.first))  / (1 << tileZ);
    double wmy = -wmProjBound + (2 * wmProjBound * (tileY + pixel.second)) / (1 << tileZ);
    return reprojectPoint(invTransform, utmz, Point(wmx, wmy));
}

void calculateInvTransform(LizardTech::MrSIDImageReader& reader, double invTransform[]) {
    double transform[6] = { 0, 0, 1, 1, 0, 0 };
    reader.getGeoCoord().get(transform[0], transform[1], transform[2], transform[3], transform[4], transform[5]);
    // TODO: apply half-pixel offset?
    // transform[0] -= transform[2] * 0.5;
    // transform[1] -= transform[3] * 0.5;

    double det = transform[2] * transform[3] - transform[4] * transform[5];
    invTransform[0] = -transform[0];
    invTransform[1] = -transform[1];
    invTransform[2] =  transform[3] / det;
    invTransform[3] =  transform[2] / det;
    invTransform[4] = -transform[4] / det;
    invTransform[5] = -transform[5] / det;
}

// Public API

struct MrSIDTileWrapperContext {
    std::shared_ptr<LizardTech::MrSIDImageReader> reader;
    int utmz = -1;
    
    explicit MrSIDTileWrapperContext(std::shared_ptr<LizardTech::MrSIDImageReader> reader, int utmz) : reader(reader), utmz(utmz) { }
};

MRSID_EXPORT void* createMrSIDContext(const char* path) {
    using namespace LizardTech;

    LTFileSpec fileSpec(path);
    std::shared_ptr<MrSIDImageReader> reader(MrSIDImageReader::create(), [](MrSIDImageReader* readerPtr) { readerPtr->release(); });
    LT_STATUS status = reader->initialize(fileSpec);
    if (status != LT_STS_Success) {
        return nullptr;
    }
    
    if (reader->isGeoCoordImplicit()) {
        return nullptr;
    }
    
    int utmz = 0;
    if (auto wkt = reader->getGeoCoord().getWKT()) {
        static std::regex projcsRe("PROJCS\\[\"([^\"]*)\".*");
        std::smatch projcsMatch;
        if (std::regex_match(std::string(wkt), projcsMatch, projcsRe)) {
            std::string projcsStr = projcsMatch[1];
            static std::regex utmzRe(".*UTM_Zone_(\\d+)N.*");
            std::smatch utmzMatch;
            if (std::regex_match(projcsStr, utmzMatch, utmzRe)) {
                std::string utmzStr = utmzMatch[1];
                utmz = std::atoi(utmzStr.c_str());
            } else {
                return nullptr;
            }
        } else {
            return nullptr;
        }
    } else {
        return nullptr;
    }
    
    return new MrSIDTileWrapperContext { reader, utmz };
}

MRSID_EXPORT void freeMrSIDContext(void* context) {
    using namespace LizardTech;

    delete reinterpret_cast<MrSIDTileWrapperContext*>(context);
}

MRSID_EXPORT int calculateMrSIDMaxZoom(void* context, int tileRes) {
    using namespace LizardTech;

    std::shared_ptr<MrSIDImageReader> reader = reinterpret_cast<MrSIDTileWrapperContext*>(context)->reader;
    int utmz = reinterpret_cast<MrSIDTileWrapperContext*>(context)->utmz;
    
    double invTransform[6];
    calculateInvTransform(*reader, invTransform);

    int maxZoom = 0;
    while (maxZoom < 24) {
        Point p[2][2];
        for (int dx = 0; dx < 2; dx++) {
            for (int dy = 0; dy < 2; dy++) {
                p[dy][dx] = reprojectTilePixel(invTransform, utmz, maxZoom, (1 << maxZoom) / 2, (1 << maxZoom) / 2, Point(dx, dy));
            }
        }
        double x0 = std::min(std::min(p[0][0].first,  p[0][1].first),  std::min(p[1][0].first,  p[1][1].first));
        double x1 = std::max(std::max(p[0][0].first,  p[0][1].first),  std::max(p[1][0].first,  p[1][1].first));
        double y0 = std::min(std::min(p[0][0].second, p[0][1].second), std::min(p[1][0].second, p[1][1].second));
        double y1 = std::max(std::max(p[0][0].second, p[0][1].second), std::max(p[1][0].second, p[1][1].second));
        if ((x1 - x0) <= tileRes && y1 - y0 <= tileRes) {
            break;
        }

        maxZoom++;
    }
    return maxZoom;
}

MRSID_EXPORT int loadMrSIDTile(void* context, int tileZ, int tileX, int tileY, int tileRes, double reprojScale, unsigned char* tileBuf) {
    using namespace LizardTech;

    std::shared_ptr<MrSIDImageReader> reader = reinterpret_cast<MrSIDTileWrapperContext*>(context)->reader;
    int utmz = reinterpret_cast<MrSIDTileWrapperContext*>(context)->utmz;

    double invTransform[6];
    calculateInvTransform(*reader, invTransform);

    int imgWidth = reader->getWidth();
    int imgHeight = reader->getHeight();

    Point p[2][2];
    for (int dx = 0; dx < 2; dx++) {
        for (int dy = 0; dy < 2; dy++) {
            p[dy][dx] = reprojectTilePixel(invTransform, utmz, tileZ, tileX, tileY, Point(dx, dy));
        }
    }

    double x0 = std::min(std::min(p[0][0].first,  p[0][1].first),  std::min(p[1][0].first,  p[1][1].first));
    double x1 = std::max(std::max(p[0][0].first,  p[0][1].first),  std::max(p[1][0].first,  p[1][1].first));
    double y0 = std::min(std::min(p[0][0].second, p[0][1].second), std::min(p[1][0].second, p[1][1].second));
    double y1 = std::max(std::max(p[0][0].second, p[0][1].second), std::max(p[1][0].second, p[1][1].second));

    double x0c = std::max(x0, (double)0);
    double x1c = std::min(x1, (double)imgWidth);
    double y0c = std::max(y0, (double)0);
    double y1c = std::min(y1, (double)imgHeight);

    if (x0c >= x1c || y0c >= y1c) {
        return 0;
    }

    double mag = 1.0;
    while (mag > reader->getMinMagnification()) {
        if (std::max(x1 - x0, y1 - y0) * mag < tileRes * reprojScale) {
            break;
        }
        mag = mag * 0.5;
    }

    LTIPixel pixel(LTI_COLORSPACE_RGBA, 4, LTI_DATATYPE_UINT8);
    LTISceneBuffer buf(pixel, (x1c - x0c) * mag + 1, (y1c - y0c) * mag + 1, nullptr);

    LTINavigator nav(*reader);
    LT_STATUS status = nav.setSceneAsULWH(x0c * mag, y0c * mag, (x1c - x0c) * mag, (y1c - y0c) * mag, mag);
    if (status != LT_STS_Success) {
        return -1;
    }
    status = reader->read(nav.getScene(), buf);
    if (status != LT_STS_Success) {
        return -1;
    }
    
    for (int y = 0; y < tileRes; y++) {
        for (int x = 0; x < tileRes; x++) {
            Point p = reprojectTilePixel(invTransform, utmz, tileZ, tileX, tileY, Point((double)x / tileRes, (double)y / tileRes));
            std::int64_t u = (std::int64_t)std::floor((p.first  - x0c) * mag + 0.5f);
            std::int64_t v = (std::int64_t)std::floor((p.second - y0c) * mag + 0.5f);

            if (u >= 0 && u < buf.getNumCols() && v >= 0 && v < buf.getNumRows()) {
                for (std::int16_t band = 0; band < 4; band++) {
                    void* sample = buf.getSample((int)u, (int)v, band);
                    tileBuf[((tileRes - 1 - y) * tileRes + x) * 4 + band] = *reinterpret_cast<unsigned char*>(sample);
                }
            }
        }
    }

    return 1;
}
