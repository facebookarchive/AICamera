#include <jni.h>
#include <string>
#include <iostream>
#include <fstream>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/bitmap.h>
#include "caffe2/core/predictor.h"
#include "caffe2/utils/proto_utils.h"
#include "classes.h"

static caffe2::NetDef _initNet, _predictNet;
static caffe2::Predictor *_predictor;

class BitmapPixelsLock {
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
    CAFFE_ENFORCE(net->ParseFromArray(data, len));
    AAsset_close(asset);
}

extern "C"
void
Java_com_example_aicamera_MainActivity_setUpNets(
        JNIEnv* env,
        jobject /* this */,
        jobject assetManager) {
    AAssetManager *mgr = AAssetManager_fromJava(env, assetManager);
    loadToNetDef(mgr, &_initNet,   "squeeze_init_net.pb");
    loadToNetDef(mgr, &_predictNet,"squeeze_predict_net.pb");
    std::string name("PredictNet");
    _predictNet.set_name(name); // We need to ensure a name is set to run.
    _predictor = new caffe2::Predictor(_initNet, _predictNet);

}

static bool busyWithInference = false;
static std::string outStr = "NN loading...";

extern "C"
jstring
Java_com_example_aicamera_MainActivity_classificationFromImage(
        JNIEnv* env,
        jobject /* this */,
        jobject bitmap,
        jint width,
        jint height) {
    caffe2::Predictor::TensorVector output_vec;

    if (busyWithInference) {
        return env->NewStringUTF(outStr.c_str());
    } else {
        busyWithInference = true;
    }

    BitmapPixelsLock bmpl = BitmapPixelsLock(env, bitmap);
    uint8_t *pixels = bmpl.getPixelsPtr();

    if (_predictor && pixels) {
        caffe2::TensorCPU input;

        // Reasonable dimensions to feed the predictor.
        const int predHeight = 128;
        const int predWidth = 128;
        const int crops = 1;
        const int channels = 3;
        const int size = predHeight * predWidth;
        const float hscale = ((float)height) / predHeight;
        const float wscale = ((float)width) / predWidth;
        const float scale = std::min(hscale, wscale);
        std::vector<float> inputPlanar(crops * channels * predHeight * predWidth);
        // Scale down the input to a reasonable predictor size.
        for (auto i = 0; i < predHeight; ++i) {
            const int _i = (int) (scale * i);
            for (auto j = 0; j < predWidth; ++j) {
                const int _j = (int) (scale * j);
                // The input is of the form BGRA, we only need the RGB part.
                inputPlanar[i * predWidth + j + 0 * size] = (float) pixels[(_i * width + _j) * 4 + 2];
                inputPlanar[i * predWidth + j + 1 * size] = (float) pixels[(_i * width + _j) * 4 + 1];
                inputPlanar[i * predWidth + j + 2 * size] = (float) pixels[(_i * width + _j) * 4 + 0];
            }
        }

        input.Resize(std::vector<int>({crops, channels, predHeight, predWidth}));
        input.ShareExternalPointer(inputPlanar.data());

        caffe2::Predictor::TensorVector input_vec{&input};
        _predictor->run(input_vec, &output_vec);
        constexpr int k = 5;
        float max[k] = {0};
        int max_index[k] = {0};
        // Find the top-k results manually.
        if (output_vec.capacity() > 0) {
            for (auto output : output_vec) {
                for (auto i = 0; i < output->size(); ++i) {
                    for (auto j = 0; j < k; ++j) {
                        if (output->template data<float>()[i] > max[j]) {
                            for (auto _j = k - 1; _j > j; --_j) {
                                max[_j - 1] = max[_j];
                                max_index[_j - 1] = max_index[_j];
                            }
                            max[j] = output->template data<float>()[i];
                            max_index[j] = i;
                            goto skip;
                        }
                    }
                    skip:;
                }
            }
        }
        std::ostringstream stringStream;
        for (auto j = 0; j < k; ++j) {
            stringStream << j << ": " << imagenet_classes[max_index[j]] << " - " << max[j] * 100 << "%\n";
        }
        outStr = stringStream.str();
        busyWithInference = false;
        return env->NewStringUTF(outStr.c_str());
    }

    busyWithInference = false;
    return env->NewStringUTF(outStr.c_str());
}
