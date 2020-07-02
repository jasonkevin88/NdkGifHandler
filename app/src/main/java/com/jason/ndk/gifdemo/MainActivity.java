package com.jason.ndk.gifdemo;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;

import android.Manifest;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.bumptech.glide.Glide;

import java.io.File;

public class MainActivity extends AppCompatActivity {

  private static int REQ_PERMISSION_CODE = 1001;
  private static final String[] PERMISSIONS = { Manifest.permission.WRITE_EXTERNAL_STORAGE };

  private static String SD_CARD_PATH = Environment.getExternalStorageDirectory().getAbsolutePath();

  private ImageView gifIv;

  private GifHandler gifHandler;

  private Bitmap bitmap;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);
    checkAndRequestPermissions();

    gifIv = findViewById(R.id.iv_gif);
  }

  public void loadGifByJava(View view) {
    File file = new File(SD_CARD_PATH, "demo.gif");
    if(!file.exists()) {
      Toast.makeText(MainActivity.this, "not found gif file", Toast.LENGTH_SHORT).show();
      return;
    }
    Glide.with(MainActivity.this).load(file).into(gifIv);
  }

  public void loadGifByNdk(View view) {
    GifLoader loader = new GifLoader();
    loader.load(gifIv, SD_CARD_PATH +File.separator+ "demo.gif");
    /*File file = new File(SD_CARD_PATH, "demo.gif");
    if(!file.exists()) {
      Toast.makeText(MainActivity.this, "not found gif file", Toast.LENGTH_SHORT).show();
      return;
    }

    gifHandler = new GifHandler(file.getAbsolutePath());

    //得到gif   width  height  生成Bitmap
    int width=gifHandler.getWidth();
    int height=gifHandler.getHeight();
    bitmap= Bitmap.createBitmap(width,height, Bitmap.Config.ARGB_8888);
    int nextFrame = gifHandler.updateFrame(bitmap);
    handler.sendEmptyMessageDelayed(1,nextFrame);*/
  }

  Handler handler = new Handler(){

    @Override
    public void handleMessage(Message msg) {
      int mNextFrame = gifHandler.updateFrame(bitmap);
      handler.sendEmptyMessageDelayed(1,mNextFrame);
      gifIv.setImageBitmap(bitmap);
    }
  };

  /**
   * 权限检测以及申请
   */
  private void checkAndRequestPermissions() {
    if (!hasPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
      ActivityCompat.requestPermissions(this, PERMISSIONS, REQ_PERMISSION_CODE);
    }
  }

  /**
   * 权限判断
   */
  private boolean hasPermission(String permissionName) {
    return ActivityCompat.checkSelfPermission(this, permissionName)
        == PackageManager.PERMISSION_GRANTED;
  }

  @Override
  public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions,
      @NonNull int[] grantResults) {

    if (requestCode == REQ_PERMISSION_CODE) {
      checkAndRequestPermissions();
    }
    super.onRequestPermissionsResult(requestCode, permissions, grantResults);
  }


}
