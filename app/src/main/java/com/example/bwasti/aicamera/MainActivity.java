package com.example.aicamera;


        import android.app.Activity;
        import android.content.res.AssetManager;
        import android.graphics.Bitmap;
        import android.graphics.BitmapFactory;
        import android.graphics.ImageFormat;
        import android.graphics.Rect;
        import android.graphics.YuvImage;
        import android.hardware.Camera;
        import android.graphics.Canvas;
        import android.graphics.Color;
        import android.graphics.Paint;

        import android.net.Uri;
        import android.os.AsyncTask;
        import android.os.Bundle;
        import android.util.Log;
        import android.view.SurfaceHolder;
        import android.view.SurfaceView;
        import android.widget.Toast;

        import com.google.android.gms.appindexing.Action;
        import com.google.android.gms.appindexing.AppIndex;
        import com.google.android.gms.appindexing.Thing;
        import com.google.android.gms.common.api.GoogleApiClient;

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
    private Camera camera = null;
    private boolean inPreview = false;
    private boolean cameraConfigured = false;
    private AssetManager mgr;

    private class SetUpNeuralNetwork extends AsyncTask {
        @Override
        protected Object doInBackground(Object[] obj) {
            setUpNets(mgr);
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
                //size.height = 224;
                //size.width = 224;

                if (size != null) {
                    parameters.setPreviewSize(size.width, size.height);
                    //parameters.setPreviewFormat(ImageFormat.FLEX_RGB_888);
                    parameters.set("orientation", "portrait");
                    parameters.set("rotation", 90);
                    camera.setParameters(parameters);
                    camera.setDisplayOrientation(90);
                    cameraConfigured = true;
                }
            }
        }
    }

    class NewFrameCallback implements Camera.PreviewCallback {
        @Override
        public void onPreviewFrame(byte[] data, Camera camera) {
            Camera.Parameters parameters = camera.getParameters();
            Camera.Size size = parameters.getPreviewSize();
            YuvImage yuv = new YuvImage(data, parameters.getPreviewFormat(), size.width, size.height, null);

            ByteArrayOutputStream _out = new ByteArrayOutputStream();
            yuv.compressToJpeg(new Rect(0, 0, size.width, size.height), 50, _out);

            byte[] bytes = _out.toByteArray();
            final Bitmap bitmap = BitmapFactory.decodeByteArray(bytes, 0, bytes.length);
            int h = size.height;
            int w = size.width;
            String out = classificationFromImage(bitmap, w, h);
            //Log.println(Log.INFO, "CLASSIFICATION", out);

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
            // no-op -- wait until surfaceChanged()
        }

        public void surfaceChanged(SurfaceHolder holder,
                                   int format, int width,
                                   int height) {
            initPreview(width, height);
            startPreview();
/*            Canvas canvas = previewHolder.lockCanvas();
            if (canvas != null) {
                Paint paint = new Paint();
                paint.setColor(Color.RED);
                canvas.drawText("XXXX", 200, 100, paint);
                previewHolder.unlockCanvasAndPost(canvas);

            }*/
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
