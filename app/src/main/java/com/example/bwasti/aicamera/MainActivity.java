package com.example.aicamera;

import android.app.Activity;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.graphics.Matrix;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.hardware.Camera;

import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import java.io.ByteArrayOutputStream;


public class MainActivity extends Activity {

    // JNI Declaration
    public static native String classificationFromImage(Bitmap data, int w, int h);
    public static native void setUpNets(AssetManager mgr);

    static {
        System.loadLibrary("neuralnet-lib");
    }

    private SurfaceView preview = null;
    private SurfaceHolder previewHolder = null;
    private TextView labelView = null;
    private ImageView imageView = null;
    private String currentLabel = "Loading...";
    private Camera camera = null;
    private boolean inPreview = false;
    private boolean cameraConfigured = false;
    private AssetManager mgr;

    private class SetUpNeuralNetwork extends AsyncTask {
        @Override
        protected Object doInBackground(Object[] obj) {
            setUpNets(mgr);
            currentLabel = "Neural net loaded! Inferring...";
            return obj;
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mgr = getResources().getAssets();

        new SetUpNeuralNetwork().execute();

        setContentView(R.layout.activity_main);

        preview = (SurfaceView) findViewById(R.id.preview);
        previewHolder = preview.getHolder();
        previewHolder.addCallback(surfaceCallback);
        previewHolder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
        labelView = (TextView) findViewById(R.id.label);
        imageView = (ImageView) findViewById(R.id.image);
        imageView.setBackgroundColor(Color.BLACK);
    }

    @Override
    public void onResume() {
        super.onResume();

        camera = Camera.open();
        startPreview();
    }

    @Override
    public void onPause() {
        if (inPreview) {
            camera.stopPreview();
        }

        camera.release();
        camera = null;
        inPreview = false;

        super.onPause();
    }

    private Camera.Size getBestPreviewSize(int width, int height,
                                           Camera.Parameters parameters) {
        Camera.Size result = null;

        for (Camera.Size size : parameters.getSupportedPreviewSizes()) {
            if (size.width <= width && size.height <= height) {
                if (result == null) {
                    result = size;
                } else {
                    int resultArea = result.width * result.height;
                    int newArea = size.width * size.height;

                    if (newArea > resultArea) {
                        result = size;
                    }
                }
            }
        }

        return (result);
    }

    private void initPreview(int width, int height) {
        if (camera != null && previewHolder.getSurface() != null) {
            try {
                camera.setPreviewDisplay(previewHolder);
            } catch (Throwable t) {
                Log.e("PreviewDemo-surfaceCallback",
                        "Exception in setPreviewDisplay()", t);
                Toast
                        .makeText(MainActivity.this, t.getMessage(), Toast.LENGTH_LONG)
                        .show();
            }

            if (!cameraConfigured) {
                Camera.Parameters parameters = camera.getParameters();
                Camera.Size size = getBestPreviewSize(width, height,
                        parameters);

                if (size != null) {
                    parameters.setPreviewSize(size.width, size.height);
                    parameters.set("rotation", 90);
                    camera.setParameters(parameters);
                    camera.setDisplayOrientation(90);
                    cameraConfigured = true;
                }
            }
        }
    }

    private class ImageObject {
        public Bitmap b;
        public int h;
        public int w;
    }

    protected boolean inferring = false;

    private class InferFromImage extends AsyncTask<ImageObject, Void, Void> {

        @Override
        protected Void doInBackground(ImageObject... obj) {
            long time = System.currentTimeMillis();
            currentLabel = classificationFromImage(obj[0].b, obj[0].w, obj[0].h);
            currentLabel += "\ntime: " + (System.currentTimeMillis() - time) + "ms";
            obj[0].b = null;
            obj[0] = null;
            return null;
        }

        @Override
        protected void onPostExecute(Void v) {
            super.onPostExecute(v);
            inferring = false;
        }
    }

    class NewFrameCallback implements Camera.PreviewCallback {
        @Override
        public void onPreviewFrame(byte[] data, Camera camera) {
            if (!inferring) {
                Camera.Parameters parameters = camera.getParameters();
                Camera.Size size = parameters.getPreviewSize();
                YuvImage yuv = new YuvImage(data, parameters.getPreviewFormat(), size.width, size.height, null);

                ByteArrayOutputStream _out = new ByteArrayOutputStream();
                yuv.compressToJpeg(new Rect(0, 0, size.width, size.height), 50, _out);

                byte[] bytes = _out.toByteArray();
                final Bitmap bitmap = BitmapFactory.decodeByteArray(bytes, 0, bytes.length);
                Matrix matrix = new Matrix();
                matrix.postRotate(90);
                final Bitmap rotatedBitmap = Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight(), matrix, true);
                int h = size.height;
                int w = size.width;
                ImageObject i = new ImageObject();
                i.h = h;
                i.w = w;
                i.b = rotatedBitmap;
                imageView.setImageBitmap(rotatedBitmap);
                inferring = true;
                new InferFromImage().execute(i);
                labelView.setText(currentLabel);
            }
        }
    }

    private void startPreview() {
        if (cameraConfigured && camera != null) {
            camera.startPreview();
            inPreview = true;
            camera.setPreviewCallback(new NewFrameCallback());
        }
    }

    SurfaceHolder.Callback surfaceCallback = new SurfaceHolder.Callback() {
        public void surfaceCreated(SurfaceHolder holder) {
        }

        public void surfaceChanged(SurfaceHolder holder,
                                   int format, int width,
                                   int height) {
            initPreview(width, height);
            startPreview();
        }

        public void surfaceDestroyed(SurfaceHolder holder) {
        }
    };

    @Override
    public void onStart() {
        super.onStart();
    }

    @Override
    public void onStop() {
        super.onStop();
    }
}
