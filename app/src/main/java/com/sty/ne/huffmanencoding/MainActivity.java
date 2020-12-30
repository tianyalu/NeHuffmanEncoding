package com.sty.ne.huffmanencoding;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;

import android.Manifest;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.sty.ne.huffmanencoding.util.ImageUtils;
import com.sty.ne.huffmanencoding.util.PermissionUtils;

import java.io.File;

public class MainActivity extends AppCompatActivity {
    private final static String PIC_PATH = Environment.getExternalStorageDirectory() + File.separator + "sty";
    private static final String srcFileName = "yyt1.jpeg"; //该图片被旋转过，导致压缩后还原到原来的状态了
//    private static final String srcFileName = "test.jpeg";
    private static final String dstFileName = "yyt2.jpeg";
//    private static final String dstFileName = "test2.jpeg";
    private String[] needPermissions = {Manifest.permission.WRITE_EXTERNAL_STORAGE};
    private Button btnCompress;

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());

        initView();
        requestPermission();
    }

    private void initView() {
        btnCompress = findViewById(R.id.btn_compress);
        btnCompress.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                onBtnCompressClicked();
            }
        });
    }

    private void onBtnCompressClicked() {
        try {
            File input = new File(PIC_PATH, srcFileName);
            Log.e("sty", "picture path: " + input.getAbsolutePath());
            Bitmap inputBitmap = BitmapFactory.decodeFile(input.getAbsolutePath());
            Log.e("sty", "bitmap width: " + inputBitmap.getWidth() + " height: " + inputBitmap.getHeight());

            boolean isRotated = ImageUtils.isRotated(PIC_PATH + File.separator + srcFileName);
            Log.e("sty", "isRotated: " + isRotated);
            if(isRotated) { //如果旋转过，那就将其再旋转一下，否则压缩后会还原到原来未旋转的状态
                inputBitmap = rotateBitmap(inputBitmap);
            }

            compressImage(inputBitmap, PIC_PATH + "/" + dstFileName, 50);
            Toast.makeText(this, "压缩图片完成", Toast.LENGTH_SHORT).show();
        }catch (Exception e) {
            e.printStackTrace();
            Toast.makeText(this, e.getMessage(), Toast.LENGTH_SHORT).show();
        }
    }

    private void requestPermission() {
        if (!PermissionUtils.checkPermissions(this, needPermissions)) {
            PermissionUtils.requestPermissions(this, needPermissions);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions,
                                           @NonNull int[] grantResults) {
        if (requestCode == PermissionUtils.REQUEST_PERMISSIONS_CODE) {
            if (!PermissionUtils.verifyPermissions(grantResults)) {
                PermissionUtils.showMissingPermissionDialog(this);
            } else {
                Bitmap.Config config = Bitmap.Config.ARGB_8888;
                Bitmap.Config.valueOf("ll");
                Bitmap.createBitmap(1,1, config);
            }
        }
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();

    public native void compressImage(Bitmap bitmap, String path, int quality);

    public native Bitmap rotateBitmap(Bitmap bitmap);

}
