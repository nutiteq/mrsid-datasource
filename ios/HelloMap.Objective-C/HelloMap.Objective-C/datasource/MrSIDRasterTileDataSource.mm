#include "MrSIDRasterTileDataSource.h"
#include "MrSIDRasterTileWrapper.h"

@interface MrSIDRasterTileDataSource() { }

@property (atomic) int tileRes;
@property (atomic) int maxZoom;
@property (atomic) float reprojScale;
@property (nonatomic) void* nativeContext;

@end

@implementation MrSIDRasterTileDataSource

-(id)initWithPath:(NSString*)path {
    self = [super initWithMinZoom:0 maxZoom:24];
    if (self != nil) {
        self.tileRes = 512;
        self.reprojScale = 1.5f;
        self.nativeContext = createMrSIDContext([path UTF8String]);
        if (!self.nativeContext) {
            [NTLog error:@"MrSIDRasterTileDataSource: Failed to open specified file"];
            @throw [NSException exceptionWithName:@"FileNotFoundException" reason:@"File Not Found on System" userInfo:nil];
        }
        self.maxZoom = calculateMrSIDMaxZoom(self.nativeContext, self.tileRes);
    }
    return self;
}

-(void)dealloc {
    if (self.nativeContext) {
        freeMrSIDContext(self.nativeContext);
        self.nativeContext = NULL;
    }
}

-(int)getTileResolution {
    return self.tileRes;
}

-(void)setTileResolution:(int)resolution {
    self.tileRes = resolution;
    self.maxZoom = calculateMrSIDMaxZoom(self.nativeContext, self.tileRes);
    [self notifyTilesChanged:NO];
}

-(float)getReprojectionScale {
    return self.reprojScale;
}

-(void)setReprojectionScale:(float)scale {
    self.reprojScale = scale;
    [self notifyTilesChanged:NO];
}

-(int)getMaxZoom {
    return self.maxZoom;
}

-(NTMapBounds*)getDataExtent {
    return [[self getProjection] getBounds];
}

-(NTTileData *)loadTile:(NTMapTile*)tile {
    if (!self.nativeContext) {
        return nil;
    }

    const int tileRes = self.tileRes;

    int tileBufSize = tileRes * tileRes * 4;
    unsigned char* tileBuf = static_cast<unsigned char*>(malloc(tileBufSize));
    if (!tileBuf) {
        return nil;
    }
    memset(tileBuf, 0, tileBufSize);
    int status = loadMrSIDTile(self.nativeContext, [tile getZoom], [tile getX], (1 << [tile getZoom]) - [tile getY] - 1, tileRes, self.reprojScale, tileBuf);
    if (status <= 0) {
        if (status < 0) {
            [NTLog error:[NSString stringWithFormat:@"MrSIDRasterTileDataSource: Failed to read tile %@", [tile description]]];
        }
        free(tileBuf);
        return nil;
    }
    NTBinaryData* binaryData = [[NTBinaryData alloc] initWithDataPtr:tileBuf size:tileBufSize];
    free(tileBuf);

    NTBitmap* tileBitmap = [[NTBitmap alloc] initWithPixelData:binaryData width:tileRes height:tileRes colorFormat:NT_COLOR_FORMAT_RGBA bytesPerRow:tileRes * 4];
    return [[NTTileData alloc] initWithData:[tileBitmap compressToInternal]];
}

@end
