#include "MrSIDRasterTileWrapper.h"

#include <jni.h>
#include <stdlib.h>
#include <memory.h>

#define MRSID_JNIEXPORT __attribute__ ((visibility("default")))

extern "C" {

MRSID_JNIEXPORT jlong Java_com_carto_datasources_MrSIDRasterTileDataSource_createContext(JNIEnv* jenv, jclass jcls, jstring jpath) {
    const char* pathPtr = jenv->GetStringUTFChars(jpath, NULL);
    if (!pathPtr) {
        return 0;
    }
    void* context = createMrSIDContext(pathPtr);
    jenv->ReleaseStringUTFChars(jpath, pathPtr);
    return reinterpret_cast<jlong>(context);
}

MRSID_JNIEXPORT void Java_com_carto_datasources_MrSIDRasterTileDataSource_freeContext(JNIEnv* jenv, jclass jcls, jlong jcontext) {
    return freeMrSIDContext(reinterpret_cast<void*>(jcontext));
}

MRSID_JNIEXPORT jint Java_com_carto_datasources_MrSIDRasterTileDataSource_calculateMaxZoom(JNIEnv* jenv, jclass jcls, jlong jcontext, jint jtileRes) {
    return calculateMrSIDMaxZoom(reinterpret_cast<void*>(jcontext), jtileRes);
}

MRSID_JNIEXPORT jbyteArray Java_com_carto_datasources_MrSIDRasterTileDataSource_loadTile(JNIEnv* jenv, jclass jcls, jlong jcontext, jint jtileZ, jint jtileX, jint jtileY, jint jtileRes, jfloat jreprojScale) {
    int tileBufSize = jtileRes * jtileRes * 4;
    unsigned char* tileBuf = (unsigned char*)malloc(tileBufSize);
    if (!tileBuf) {
        return NULL;
    }
    memset(tileBuf, 0, tileBufSize);
    int status = loadMrSIDTile(reinterpret_cast<void*>(jcontext), jtileZ, jtileX, jtileY, jtileRes, jreprojScale, tileBuf);
    if (status < 0) {
        free(tileBuf);
        return NULL;
    }
    jbyteArray jarr = jenv->NewByteArray(status > 0 ? tileBufSize : 0);
    if (jarr && status > 0) {
        jenv->SetByteArrayRegion(jarr, 0, tileBufSize, (const jbyte*)tileBuf);
    }
    free(tileBuf);
    return jarr;
}

}
