package com.jason.ndk.gifdemo;

import android.graphics.Bitmap;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.widget.ImageView;
import android.widget.Toast;

import java.io.File;
import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.Map;

/**
 * Description:GifLoader
 *
 * @author 陈宝阳
 * @create 2020/7/1 10:00
 */
public class GifLoader {

  private Map<String, GifHandler> gifHandlerMap = new HashMap<>();

  private Map<String, Bitmap> gifBitmapMap = new HashMap<>();

  private GifHandler gifHandler;
  private Bitmap bitmap;
  private WeakReference<ImageView> imageViewWeakReference;

  public void load(ImageView imageView, final String path) {
    imageViewWeakReference = new WeakReference<ImageView>(imageView);
    new Thread() {
      @Override
      public void run() {
        load(path);
      }
    }.start();
  }

  private void load(String path) {
    gifHandler = gifHandlerMap.get(path);

    File file = new File(path);
    if(!file.exists()) {
      System.err.println("not found gif file");
      return;
    }


    if(gifHandler == null) {
      gifHandler = new GifHandler(file.getAbsolutePath());
      gifHandlerMap.put(path, gifHandler);
    }

    bitmap = gifBitmapMap.get(path);
    if(bitmap == null) {
      int width = gifHandler.getWidth();
      int height = gifHandler.getHeight();
      bitmap= Bitmap.createBitmap(width,height, Bitmap.Config.ARGB_8888);
      gifBitmapMap.put(path, bitmap);
    }

    int nextFrame = gifHandler.updateFrame(bitmap);
    handler.sendEmptyMessageDelayed(1,nextFrame);
  }

  private Handler handler = new Handler(Looper.getMainLooper()){

    @Override
    public void handleMessage(Message msg) {
      ImageView gifIv = imageViewWeakReference.get();
      if(gifIv == null) {
        return;
      }
      int mNextFrame = gifHandler.updateFrame(bitmap);
      handler.sendEmptyMessageDelayed(1,mNextFrame);
      gifIv.setImageBitmap(bitmap);
    }
  };
}
