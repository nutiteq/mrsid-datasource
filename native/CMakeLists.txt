cmake_minimum_required(VERSION 3.1)
project(mrsid_rastertile_datasource)

if(ANDROID)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -ftemplate-depth=1024 -fexceptions -frtti -fvisibility=hidden -fvisibility-inlines-hidden")
endif(ANDROID)

if(IOS)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -std=c++11 -ftemplate-depth=1024 -fexceptions -frtti -fvisibility=hidden -fvisibility-inlines-hidden")
set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH YES)
endif(IOS)

include_directories("${MRSID_DSDK}/Raster_DSDK/include")

if(ANDROID)
add_library(mrsid_rastertile_datasource SHARED "MrSIDRasterTileDataSourceWrapperJNI.cpp" "MrSIDRasterTileWrapper.cpp")
endif(ANDROID)

if(IOS)
add_library(mrsid_rastertile_datasource SHARED "MrSIDRasterTileWrapper.cpp")
endif(IOS)

if(ANDROID)
target_link_libraries(mrsid_rastertile_datasource "${MRSID_DSDK}/Raster_DSDK/lib/${ANDROID_ABI}/libltidsdk.so")
endif(ANDROID)

if(IOS)
target_link_libraries(mrsid_rastertile_datasource iconv "${MRSID_DSDK}/Raster_DSDK/3rd-party/lib/libgdal.a" "${MRSID_DSDK}/Raster_DSDK/lib/libltidsdk.a")
endif(IOS)

if(ANDROID)
add_custom_command(TARGET mrsid_rastertile_datasource POST_BUILD COMMAND ${CMAKE_STRIP} "libmrsid_rastertile_datasource.so")
endif(ANDROID)

if(IOS)
set_property(TARGET mrsid_rastertile_datasource PROPERTY XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer")
set_property(TARGET mrsid_rastertile_datasource PROPERTY XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "XULUY39T35")
endif(IOS)
