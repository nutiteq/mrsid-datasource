#pragma once

extern "C" {
    void* createMrSIDContext(const char* path);
    void freeMrSIDContext(void* context);
    int calculateMrSIDMaxZoom(void* context, int tileRes);
    int loadMrSIDTile(void* context, int tileZ, int tileX, int tileY, int tileRes, double reprojScale, unsigned char* tileBuf);
}
