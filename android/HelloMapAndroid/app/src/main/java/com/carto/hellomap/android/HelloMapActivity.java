package com.carto.hellomap.android;

import android.app.Activity;
import android.content.res.AssetManager;
import android.graphics.Color;
import android.os.Bundle;
import android.util.Log;

import com.carto.components.RenderProjectionMode;
import com.carto.core.MapPos;
import com.carto.datasources.LocalVectorDataSource;

import com.carto.datasources.MrSIDRasterTileDataSource;
import com.carto.layers.CartoBaseMapStyle;
import com.carto.layers.CartoOnlineVectorTileLayer;
import com.carto.layers.RasterTileLayer;
import com.carto.layers.VectorLayer;
import com.carto.projections.EPSG4326;
import com.carto.projections.Projection;
import com.carto.styles.MarkerStyle;
import com.carto.styles.MarkerStyleBuilder;
import com.carto.ui.MapClickInfo;
import com.carto.ui.MapEventListener;
import com.carto.ui.MapView;
import com.carto.vectorelements.Marker;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Random;

public class HelloMapActivity extends Activity {

    static final String LICENSE = "XTUMwQ0ZDNGo4cFZKMklMZHlFQVdZditGYzduazV4QzZBaFVBbkJzRUExMmhqVnFxSEY3bkpTUFVyM0M2NzdRPQoKYXBwVG9rZW49YzQxYTM5ZjktN2I5MC00MThhLTkyZjUtN2I0ODljZDYxZmFhCnBhY2thZ2VOYW1lPWNvbS5jYXJ0by5oZWxsb21hcC5hbmRyb2lkCm9ubGluZUxpY2Vuc2U9MQpwcm9kdWN0cz1zZGstYW5kcm9pZC00LioKd2F0ZXJtYXJrPWNhcnRvZGIK";

    private MapView mapView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        MapView.registerLicense(LICENSE, getApplicationContext());

        // Set view from layout resource
        setContentView(R.layout.activity_hello_map);
        setTitle("Hello Map");
        mapView = (MapView) this.findViewById(R.id.map_view);

        // Set map view options
        Projection proj = new EPSG4326();
        mapView.getOptions().setBaseProjection(proj);
        mapView.getOptions().setZoomGestures(true);
        //mapView.getOptions().setRenderProjectionMode(RenderProjectionMode.RENDER_PROJECTION_MODE_SPHERICAL);

        // Add base map
        CartoOnlineVectorTileLayer baseLayer = new CartoOnlineVectorTileLayer(CartoBaseMapStyle.CARTO_BASEMAP_STYLE_VOYAGER);
        mapView.getLayers().add(baseLayer);

        // Set default location and zoom
        MapPos focusPos = new MapPos(10.5, 59.9);
        mapView.setFocusPos(focusPos, 0);
        mapView.setZoom(10, 0);

        // Copy MrSID file to accessible location
        String file = "Oslo.sid";
        try {
            String localDir = getExternalFilesDir(null).toString();
            copyAssetToSDCard(getAssets(), file, localDir);
            String path = localDir + "/" + file;

            // Create MrSIDRasterTileDataSource and attach it to a raster layer
            MrSIDRasterTileDataSource mrsidDataSource = new MrSIDRasterTileDataSource(path);
            RasterTileLayer mrsidLayer = new RasterTileLayer(mrsidDataSource);
            mapView.getLayers().add(mrsidLayer);
        } catch (IOException e) {
            Log.e("HelloMap", "Failed to copy asset to file system");
        }
    }

    static void copyAssetToSDCard(AssetManager assetManager, String fileName, String toDir) throws IOException {
        File outFile = new File(toDir, fileName);
        if (outFile.exists()) {
            return;
        }

        InputStream in = assetManager.open(fileName);
        OutputStream out = new FileOutputStream(outFile);

        byte[] buffer = new byte[1024];
        int read;
        while ((read = in.read(buffer)) != -1) {
            out.write(buffer, 0, read);
        }

        in.close();
        out.flush();
        out.close();
    }
}
