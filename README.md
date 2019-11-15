## MrSID Raster Datasource for CARTO Mobile SDK


  

This project contains a custom offline raster datasource for CARTO Mobile SDK based on LizardTech MrSID files.

  
The datasource is provided for iOS and Android platforms.

  

### Usage

  
The project contains basic sample projects for both iOS and Android platforms.

For Android, the relevant *Android Studio* project is placed in 'android' folder. Note that the project depends on native component (embedded into 'mrsid-rastertile-datasource.aar' package).

The actual datasource class is implemented in Java and can be modified, if needed. It connects to the native component to do actual tile loading and reprojection.

iOS sample is placed in 'ios' folder and depends on *CocoaPod* package manager. Once `pod update` is executed, the relevant project workspace is created and the project can be loaded into *XCode* IDE. Note that the 'native' component is embedded into dynamic library ('libmrsid_rastertile_datasource.dylib') that is linked to the project.

The actual datasource class is implemented in Objective C and can be modified, if needed.


Both iOS and Android datasource classes support 2 custom settings: tile size and reprojection scale.

The default values (512 for tile size and 1.75 for reprojection scale) should work quite well, but it is possible to increase the rendering quality by increasing both values. Note that this should come with heavy performance penalty (slower tile loading times), though.


### Limitations


Current implementation assumes that MrSID files use **UTM** coordinate system (and should work with *any* UTM zones in northern hemisphere). It can be generalized to support any projection by adding support to *proj 6.x* library.

Also, the current implementation uses nearest-neighbour resampling method, which does not provide optimal rendering quality.
