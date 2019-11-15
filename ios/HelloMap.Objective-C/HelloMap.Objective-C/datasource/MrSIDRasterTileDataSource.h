/*
 * Copyright (c) 2019 CARTO. All rights reserved.
 * Copying and using this code is allowed only according
 * to license terms, as given in https://cartodb.com/terms/
 */

#pragma once

#import <CartoMobileSDK/CartoMobileSDK.h>

/*
 * A custom raster tile data source that uses MrSID file.
 */
@interface MrSIDRasterTileDataSource : NTTileDataSource

/* Initialization */
-(id)initWithPath:(NSString*)path;

/* Overridden methods */
-(void)dealloc;

-(int)getMaxZoom;

-(NTMapBounds*)getDataExtent;

-(NTTileData*)loadTile:(NTMapTile*)tile;

/* Custom API methods */
-(int)getTileResolution;
-(void)setTileResolution:(int)resolution;

-(float)getReprojectionScale;
-(void)setReprojectionScale:(float)scale;

@end
