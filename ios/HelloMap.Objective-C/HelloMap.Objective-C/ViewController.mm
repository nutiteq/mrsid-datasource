//
//  ViewController.mm
//  HelloMap.Objective-C
//

#import "ViewController.h"

#import "datasource/MrSIDRasterTileDataSource.h"

@interface ViewController ()

@end

@implementation ViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // Minimal map definition code follows with some tweaks
    
    // Smoother UI/animations
    [self setPreferredFramesPerSecond:60];

    // The storyboard has NTMapView connected as a view
    NTMapView* mapView = (NTMapView*) self.view;
    
    // Set common map view options. Use EPSG4326 projection for coordinates. This allows us to use longitude/latitude coordinates directly.
    NTProjection* proj = [[NTEPSG4326 alloc] init];
    [[mapView getOptions] setBaseProjection:proj];
    [[mapView getOptions] setZoomGestures:YES]; // enable zooming on clicks/double clicks
    //[[mapView getOptions] setRenderProjectionMode:NT_RENDER_PROJECTION_MODE_SPHERICAL]; // comment this line to switch to default planar mode

    // Add base layer
    NTVectorTileLayer* baseLayer = [[NTCartoOnlineVectorTileLayer alloc] initWithStyle:NT_CARTO_BASEMAP_STYLE_VOYAGER];
    [[mapView getLayers] add:baseLayer];
    
    // Zoom to Oslo, Norway
    NTMapPos* focusPos = [[NTMapPos alloc] initWithX:10.5 y:59.9];
    [mapView setFocusPos:focusPos durationSeconds:0];
    [mapView setZoom:10 durationSeconds:0];
    
    // Add MrSID datasource
    NSString* path = [[NSBundle mainBundle] pathForResource:@"Oslo" ofType:@"sid"];
    MrSIDRasterTileDataSource* mrsidDataSource = [[MrSIDRasterTileDataSource alloc] initWithPath:path];
    NTRasterTileLayer* mrsidLayer = [[NTRasterTileLayer alloc] initWithDataSource:mrsidDataSource];
    [[mapView getLayers] add:mrsidLayer];
}

@end
