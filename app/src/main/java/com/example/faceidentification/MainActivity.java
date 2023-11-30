package com.example.faceidentification;

import androidx.activity.result.ActivityResultCallback;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.core.content.FileProvider;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Matrix;

import androidx.exifinterface.media.ExifInterface;

import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.provider.Settings;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.Toast;

import com.example.faceidentification.databinding.ActivityMainBinding;
import com.example.faceidentification.face.Face;
import com.example.faceidentification.face.FaceShapes;

import org.json.JSONException;
import org.opencv.android.Utils;
import org.opencv.core.Mat;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;


public class MainActivity extends AppCompatActivity {

    // 用于在应用程序启动时加载“人脸识别”库。
    static {
        System.loadLibrary("dlib");
        System.loadLibrary("opencv_java4");
        System.loadLibrary("faceidentification");
    }

    private final String TAG = "MainActivity";

    private ImageView faceImage;  // 原始图view
    private ImageView faceDetect;  // 人脸检测图view

    private Bitmap faceBitmap;   // 原始图片Bitmap
    private Button cameraButton; // 拍照
    private Button localButton;  // 相册

    private Button faceDetectButton; // 开始识别
    private File currentImageFile = null; // 拍照存储的文件路径
    private Uri takePictureUri; // 拍照拿到的Uri


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        ActivityMainBinding binding = ActivityMainBinding.inflate(getLayoutInflater());
        setContentView(binding.getRoot());

        initItems();

        ActivityResultLauncher<Intent> getLocalPhotoLauncher = registerForActivityResult(new ActivityResultContracts.StartActivityForResult(), result -> {
            if (result.getResultCode() == RESULT_OK) {
                if (result.getData() != null) {
                    Log.i(TAG, "获取照片成功");
                    Uri uri = result.getData().getData();
                    faceImage.setImageURI(uri);
                    try {
                        faceBitmap = BitmapFactory.decodeStream(getContentResolver().openInputStream(uri));
                    } catch (FileNotFoundException e) {
                        throw new RuntimeException(e);
                    }
                } else {
                    Log.e(TAG, "获取照片失败");
                }
            }
        });

        ActivityResultLauncher<Uri> getCameraPhotoLauncher = registerForActivityResult(new ActivityResultContracts.TakePicture(), new ActivityResultCallback<Boolean>() {
            @Override
            public void onActivityResult(Boolean result) {
                if (result) {
                    Log.e(TAG, "拍照成功");
                    String imgPath = currentImageFile.getPath();
                    int degree = readPictureDegree(imgPath);
                    Log.d(TAG, "图片旋转角度: " + degree);
                    faceBitmap = rotateBitmap(degree, BitmapFactory.decodeFile(imgPath));
                    faceImage.setImageBitmap(faceBitmap);
                } else {
                    Log.e(TAG, "拍照失败");
                }
                takePictureUri = null;
            }
        });

        // 本地照片
        localButton.setOnClickListener(v -> {
            Log.i(TAG, "获取照片");
            Intent intent = choosePhoto();
            getLocalPhotoLauncher.launch(intent);
        });
        // 拍照
        cameraButton.setOnClickListener(v -> {
            Log.i(TAG, "开始拍照");
            int checkPermission = ContextCompat.checkSelfPermission(MainActivity.this, Manifest.permission.CAMERA);
            if (checkPermission != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(MainActivity.this, new String[]{Manifest.permission.CAMERA}, 1);
            } else {
                takePictureUri = getTakePictureUri();
                getCameraPhotoLauncher.launch(takePictureUri);
            }
        });
        // 开始检查
        faceDetectButton.setOnClickListener(v -> {
            Mat faceMat = new Mat();
            Bitmap bitmap = Bitmap.createBitmap(faceBitmap);
            Utils.bitmapToMat(bitmap, faceMat);
            Log.d(TAG, "人脸检测开始....................");
            int[] arr = faceDetect(faceMat.getNativeObjAddr());
            Bitmap newImage = Bitmap.createBitmap(arr, faceMat.width(), faceMat.height(), Bitmap.Config.ARGB_8888);
            faceDetect.setImageBitmap(newImage);
            faceImage.setVisibility(View.GONE);
            faceDetect.setVisibility(View.VISIBLE);
            int num = getFaceNum();
            if (num <= 0) {
                Toast.makeText(MainActivity.this, "未检测到人脸，请重新上传文件", Toast.LENGTH_SHORT).show();
            } else {
                Toast.makeText(MainActivity.this, "检测到人脸个数:" + num, Toast.LENGTH_SHORT).show();
                FaceShapes[][] landMarks = getFaceShapes();
                Log.d(TAG, "获取面孔功能......");
                double[][] faceDes = getFacesDescriptors();
                Log.i(TAG, "获取人脸描述符...........");
                Log.d(TAG, "faceDes 长度为 :" + faceDes.length);
                Face[] faces = new Face[faceDes.length];
                int i = 0;
                for (double[] facedes : faceDes) {
                    Face face = null;
                    try {
                        face = new Face(facedes);
                    } catch (JSONException e) {
                        throw new RuntimeException(e);
                    }
                    faces[i++] = face;
                }
            }
            Log.d(TAG, "人脸检测 end....................");
        });
    }

    private void initItems() {
        //加载默认照片
        faceBitmap = BitmapFactory.decodeStream(getClass().getResourceAsStream("/res/drawable/zhou.jpg"));
        //获取button
        faceDetectButton = findViewById(R.id.faceDetectButton);
        localButton = findViewById(R.id.detectLocalButton);
        cameraButton = findViewById(R.id.detectCameraButton);

        faceImage = findViewById(R.id.faceImage);
        faceImage.setImageBitmap(faceBitmap);
        faceImage.setVisibility(View.VISIBLE);
        faceDetect = findViewById(R.id.faceDetectRes);
        faceDetect.setVisibility(View.GONE);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            if (!Environment.isExternalStorageManager()) {
                Intent intent = new Intent(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION);
                startActivity(intent);
            }
        }
        if (Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED)) {
            Log.d(TAG, "SD卡 is OK");
        }
        //初始化人脸识别引擎 C++
        initEngine(getAssets());
    }

    //读取图片旋转角度
    public int readPictureDegree(String path) {
        int degree = 0;
        try {
            ExifInterface exifInterface = new ExifInterface(path);
            int orientation = exifInterface.getAttributeInt(ExifInterface.TAG_ORIENTATION, ExifInterface.ORIENTATION_NORMAL);
            Log.w(TAG, "readPictureDegree：方向 = " + orientation);
            if (orientation == ExifInterface.ORIENTATION_ROTATE_90) {
                degree = 90;
            } else if (orientation == ExifInterface.ORIENTATION_ROTATE_180) {
                degree = 180;
            } else if (orientation == ExifInterface.ORIENTATION_ROTATE_270) {
                degree = 270;
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        return degree;
    }

    //旋转图片
    public Bitmap rotateBitmap(int angle, Bitmap bitmap) {
        Matrix matrix = new Matrix();
        matrix.postRotate(angle);
        Bitmap rotation = Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight(),
                matrix, true);
        return rotation;
    }

    /**
     * 打开选择图片的界面
     */
    private Intent choosePhoto() {
        if (Build.VERSION.SDK_INT >= 30) {// Android 11 (API level 30)
            return new Intent(Intent.ACTION_PICK, android.provider.MediaStore.Images.Media.EXTERNAL_CONTENT_URI);
        } else {
            Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
            intent.setType("image/*");
            return Intent.createChooser(intent, null);
        }
    }

    /**
     * 拍照
     */
    private Uri getTakePictureUri() {
        File dir = new File(Environment.getExternalStorageDirectory(), "pictures");
        if (dir.exists()) {
            dir.mkdirs();//在根路径下建子目录，子目录名是"pictures"
        }
        //命名临时图片的文件名
        currentImageFile = new File(dir, System.currentTimeMillis() + ".jpg");
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            //test.xxx.com.myapplication.fileprovider 是在清单文件配置的 android:authorities
            return FileProvider.getUriForFile(MainActivity.this, "com.qihao.faceidentification.fileprovider", currentImageFile);
        } else {
            return Uri.fromFile(currentImageFile);
        }
    }

    /**
     * 由“faceidentification”原生库实现的原生方法，
     * 与此应用程序打包在一起。
     */
    public native void convertColor(long srcRGBMatAddr, long dstGrayMatAddr);

    public native int[] faceDetect(long srcFaceAddr);

    public native void initEngine(AssetManager assetManager);

    public native int getFaceNum();

    public native FaceShapes[][] getFaceShapes();

    public native double[][] getFacesDescriptors();

}