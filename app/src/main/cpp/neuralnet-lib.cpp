#include <jni.h>
#include <string>
#include <iostream>
#include <fstream>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <android/bitmap.h>
#include "caffe2/core/predictor.h"

static caffe2::NetDef _initNet, _predictNet;
static caffe2::Predictor *_predictor;

class BitmapPixelsLock {
    // members
    JNIEnv* mEnv;
    jobject mBitmap;
    uint8_t* mPtr;

public:
    BitmapPixelsLock(JNIEnv* env, jobject bitmap) : mEnv(env), mBitmap(bitmap), mPtr(NULL) {
        if (AndroidBitmap_lockPixels(mEnv, mBitmap, (void**)&mPtr) != ANDROID_BITMAP_RESULT_SUCCESS) {
            mPtr = NULL; // ensure we mark lock failure
        }
    }

    ~BitmapPixelsLock() {
        if (mPtr) {
            jthrowable pendingException = mEnv->ExceptionOccurred();
            if (!pendingException) {
                AndroidBitmap_unlockPixels(mEnv, mBitmap);
                return;
            }
            // unlockPixels by itself should not throw, but it can't run if
            // there's an exception pending
            mEnv->ExceptionClear();
            AndroidBitmap_unlockPixels(mEnv, mBitmap);
            mEnv->Throw(pendingException);
        }
    }

    uint8_t* getPixelsPtr() { return mPtr; }
};

// A function to load the NetDefs from protobufs.
void loadToNetDef(AAssetManager* mgr, caffe2::NetDef* net, const char *filename) {
    AAsset* asset = AAssetManager_open(mgr, filename, AASSET_MODE_BUFFER);
    assert(asset != nullptr);
    const void *data = AAsset_getBuffer(asset);
    assert(data != nullptr);
    off_t len = AAsset_getLength(asset);
    assert(len != 0);
    net->ParseFromArray(data, len);
    AAsset_close(asset);
}

extern "C"
void
Java_com_example_aicamera_MainActivity_setUpNets(
        JNIEnv* env,
        jobject /* this */,
        jobject assetManager) {
    AAssetManager *mgr = AAssetManager_fromJava(env, assetManager);
    loadToNetDef(mgr, &_initNet,   "init_net.pb");
    loadToNetDef(mgr, &_predictNet,"predict_net.pb");
    _predictNet.set_name("PredictNet"); // We need to ensure a name is set to run.
    _predictor = new caffe2::Predictor(_initNet, _predictNet);
}

static bool busyWithInference;

extern "C"
jstring
Java_com_example_aicamera_MainActivity_classificationFromImage(
        JNIEnv* env,
        jobject /* this */,
        jobject bitmap,
        jint width,
        jint height) {
    caffe2::Predictor::TensorVector output_vec;

    std::string outStr = "thinking...";

    if (busyWithInference) {
        return env->NewStringUTF(outStr.c_str());
    } else {
        busyWithInference = true;
    }

    BitmapPixelsLock bmpl = BitmapPixelsLock(env, bitmap);

    if (_predictor) {
        caffe2::TensorCPU input;

        const int predHeight = 244;
        const int predWidth = 244;
        std::vector<float> inputPlanar(3 * predHeight * predWidth);
        auto _i = 0, _j = 0;
        for (auto i = 0; i < predHeight; i += height / predHeight) {
            _i++;
            for (auto j = 0; j < predWidth; j += width / predWidth) {
                _j++;
                inputPlanar[_i * predWidth + _j + 0] = (float) bmpl.getPixelsPtr()[i * width + j + 0];
                inputPlanar[_i * predWidth + _j + 1] = (float) bmpl.getPixelsPtr()[i * width + j + 1];
                inputPlanar[_i * predWidth + _j + 2] = (float) bmpl.getPixelsPtr()[i * width + j + 2];
            }
        }

        input.Resize(std::vector<int>({1, 3, height, width}));
        input.ShareExternalPointer(inputPlanar.data());

        caffe2::Predictor::TensorVector input_vec{&input};
        _predictor->run(input_vec, &output_vec);
        caffe2::TensorCPU *output = output_vec.front();
    }


    if (output_vec.capacity() > 0) {
        outStr = "Predicted";
    }

    busyWithInference = false;
    return env->NewStringUTF(outStr.c_str());
}
