package com.jason.ndk.gifdemo;

import android.graphics.Bitmap;

/**
 * Description:GifHandler
 *
 * @author 陈宝阳
 * @create 2020/6/30 10:53
 */
public class GifHandler {

  static {
    System.loadLibrary("native-lib");
  }

  private long gifAddress;

  public GifHandler(String path) {
    gifAddress = loadGif(path);
  }

  public int getWidth() {
    return getGifWidth(gifAddress);
  }

  public int getHeight() {
    return getGifHeight(gifAddress);
  }

  public int updateFrame(Bitmap bitmap) {
    return updateFrame(gifAddress, bitmap);
  }

  private native long loadGif(String path);

  private native int getGifWidth(long gifPoint);

  private native int getGifHeight(long gifPoint);

  private native int updateFrame(long gifPoint, Bitmap bitmap);
}
