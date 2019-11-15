### Compiling for Android

```
export ANDROID_NDK=<Location of Android NDK>
export ANDROID_MRSID_DSDK=<Location of MrSID Android DSDK>
export ANDROID_ABI=arm64-v8a
mkdir -p android/$ANDROID_ABI
cd android/$ANDROID_ABI
cmake -G "Unix Makefiles" -DCMAKE_SYSTEM_NAME=Android -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK/build/cmake/android.toolchain.cmake" -DMRSID_DSDK="$ANDROID_MRSID_DSDK" -DCMAKE_BUILD_TYPE=Release -DANDROID_STL="c++_static" -DANDROID_ABI=$ANDROID_ABI ../..
cmake --build . --config Release
cd ../..

... same for other abis (armeabi-v7a, x86, x86_64) ...

... zip generated .so files (under jni), AndroidManifest.xml and classes.jar into aar file ...
```


### Compiling for iOS

```
export IOS_MRSID_DSDK=<Location of MrSID Android DSDK>
export IOS_ARCH=x86_64
export IOS_PLATFORM=iphonesimulator
mkdir -p ios/$IOS_ARCH
cd ios/$IOS_ARCH
cmake -G "Xcode" -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_ARCHITECTURES=$IOS_ARCH -DCMAKE_OSX_SYSROOT=$IOS_PLATFORM -DCMAKE_OSX_DEPLOYMENT_TARGET=8.0 -DMRSID_DSDK=$IOS_MRSID_DSDK ../..
cmake --build . --config Release
cd ../..

... same for other archs/platforms (i386, armv7, arm64)  ...

lipo -output ios/libmrsid_rastertile_datasource.dylib -create ios/x86_64/Release-$IOS_PLATFORM/libmrsid_rastertile_datasource.dylib
```
