/*
 * Copyright (c) 2019 CARTO. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

package com.carto.datasources;

import com.carto.core.BinaryData;
import com.carto.core.MapTile;
import com.carto.core.MapBounds;
import com.carto.datasources.components.TileData;
import com.carto.graphics.Bitmap;
import com.carto.graphics.ColorFormat;

import java.io.IOException;

/*
 * A custom raster tile data source that uses MrSID file.
 */
public class MrSIDRasterTileDataSource extends TileDataSource {
    private transient long nativeContext;
    private int tileRes = 512;
    private float reprojScale = 1.5f;
    private int maxZoom = 0;

    private final static native long createContext(String path);
    private final static native void freeContext(long context);
    private final static native int calculateMaxZoom(long context, int tileRes);
    private final static native byte[] loadTile(long context, int tileZ, int tileX, int tileY, int tileRes, float reprojScale);

    static {
        try {
            System.loadLibrary("mrsid_rastertile_datasource");
        } catch (Exception e) {
            com.carto.utils.Log.error("MrSIDRasterTileDataSource: Failed to load native library");
            throw e;
        }
    }

    /* Initialization */
    public MrSIDRasterTileDataSource(String path) throws IOException {
        super(0, 24);
        nativeContext = createContext(path);
        if (nativeContext == 0) {
            throw new IOException("Failed to open specified file");
        }
        maxZoom = calculateMaxZoom(nativeContext, tileRes);
    }

    protected void finalize() {
        freeContext(nativeContext);
        nativeContext = 0;
    }

    /* Overridden methods */
    @Override
    public int getMaxZoom() {
        synchronized (this) {
            return maxZoom;
        }
    }

    @Override
    public MapBounds getDataExtent() {
        return getProjection().getBounds();
    }

    @Override
    public TileData loadTile(MapTile tile) {
        int tileRes;
        float reprojScale;
        synchronized (this) {
            tileRes = this.tileRes;
            reprojScale = this.reprojScale;
        }

        byte[] arr = loadTile(nativeContext, tile.getZoom(), tile.getX(), (1 << tile.getZoom()) - 1 - tile.getY(), tileRes, reprojScale);
        if (arr == null) {
            com.carto.utils.Log.error("MrSIDRasterTileDataSource: Failed to load tile " + tile.toString());
            return null;
        }
        if (arr.length == 0) {
            return null;
        }
        BinaryData binaryData = new BinaryData(arr);
        Bitmap bitmap = new Bitmap(binaryData, tileRes, tileRes, ColorFormat.COLOR_FORMAT_RGBA, 4 * tileRes);
        return new TileData(bitmap.compressToInternal());
    }

    /* Custom API methods */
    public int getTileResolution() {
        synchronized (this) {
            return tileRes;
        }
    }

    public void setTileResolution(int tileRes) {
        synchronized (this) {
            this.tileRes = tileRes;
            this.maxZoom = calculateMaxZoom(nativeContext, tileRes);
        }
        notifyTilesChanged(false);
    }

    public float getReprojectionScale() {
        synchronized (this) {
            return reprojScale;
        }
    }

    public void setReprojectionScale(float reprojScale) {
        synchronized (this) {
            this.reprojScale = reprojScale;
        }
        notifyTilesChanged(false);
    }
}
