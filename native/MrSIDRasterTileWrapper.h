/*
 * Copyright (c) 2019 CARTO. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#pragma once

extern "C" {
    void* createMrSIDContext(const char* path);
    void freeMrSIDContext(void* context);
    int calculateMrSIDMaxZoom(void* context, int tileRes);
    int loadMrSIDTile(void* context, int tileZ, int tileX, int tileY, int tileRes, double reprojScale, unsigned char* tileBuf);
}
