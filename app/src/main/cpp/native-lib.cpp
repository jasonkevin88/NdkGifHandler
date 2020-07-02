#include <jni.h>
#include <string>
#include <android/log.h>
#include <android/bitmap.h>
#include "gif_lib.h"

#define  LOG_TAG    "BAO"
#define  argb(a,r,g,b) ( ((a) & 0xff) << 24 ) | ( ((b) & 0xff) << 16 ) | ( ((g) & 0xff) << 8 ) | ((r) & 0xff)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

typedef struct GifBean{
    int current_frame;
    int total_frame;
    int *delays;
} GifBean;

extern "C"
JNIEXPORT jlong JNICALL
Java_com_jason_ndk_gifdemo_GifHandler_loadGif(JNIEnv *env, jobject thiz, jstring path_) {

    const char * path = env->GetStringUTFChars(path_, NULL);

    int error;
    //以文件流来打开gif文件，读取gif文件信息，并且返回一个结构体
    GifFileType *gifFileType = DGifOpenFileName(path, &error);
    //将整个GIF读入内核，挂起它所有的状态信息 GifFileType指针
    DGifSlurp(gifFileType);

    //给自定义的结构体GifBean分配空间
    GifBean *gifBean = (GifBean*)(malloc(sizeof(GifBean)));
    //初始化，清空内存
    memset(gifBean, 0, sizeof(GifBean));
    //将我们创建的gifbean的数据指针传到GifFileType的用户数据指针变量
    gifFileType->UserData = gifBean;

    //给gif所有帧播放的时间，分配内存空间
    gifBean->delays = (int *)malloc(sizeof(int) * gifFileType->ImageCount);
    //清空初始化分配好的内存
    memset(gifBean->delays, 0, sizeof(int) * gifFileType->ImageCount);

    //存储gif文件一共有多少帧
    gifBean->total_frame = gifFileType->ImageCount;
    //定义拓展块结构体
    ExtensionBlock *extensionBlock;
    for (int i = 0; i < gifFileType->ImageCount; ++i) {
        // SaveImages变量用来存储已经读取过得图像数据。
        // 获取每一帧图像的资源
        SavedImage savedImage = gifFileType->SavedImages[i];
        // 遍历当前这一帧的图片中的拓展块的数据
        for (int j = 0; j < savedImage.ExtensionBlockCount; ++j) {
            //拿到图形拓展的内容
            if (savedImage.ExtensionBlocks[j].Function == GRAPHICS_EXT_FUNC_CODE) {
                extensionBlock = &savedImage.ExtensionBlocks[j];
                break;
            }
        }
        if (extensionBlock) {
            //拿到图形拓展块后，计算每一帧的播放时长
            int frame_delay = 10 * (extensionBlock->Bytes[2] << 8 | extensionBlock->Bytes[1]);
            LOGE("时间  %d   ",frame_delay);
            gifBean->delays[i] = frame_delay;
        }
    }

    LOGE("gif  长度大小    %d  ",gifFileType->ImageCount);
    env->ReleaseStringUTFChars(path_, path);
    //到这做了两件事：一是计算gif图片中，每一帧图片的播放时长，二是保持总共多少帧图片
    //返回结构体的首地址
    return (jlong) gifFileType;
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_jason_ndk_gifdemo_GifHandler_getGifWidth(JNIEnv *env, jobject thiz, jlong gif_point) {
    GifFileType *gifFileType = (GifFileType *) gif_point;
    //返回gif的宽度
    return gifFileType->SWidth;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_jason_ndk_gifdemo_GifHandler_getGifHeight(JNIEnv *env, jobject thiz, jlong gif_point) {
    GifFileType *gifFileType = (GifFileType *) gif_point;
    //返回gif的高度
    return gifFileType->SHeight;
}

//绘制一张图片
void drawFrame(GifFileType *gifFileType, GifBean *gifBean, AndroidBitmapInfo info, void *pixels) {
    //播放底层代码
    //拿到当前帧
    SavedImage savedImage = gifFileType->SavedImages[gifBean->current_frame];
    //
    GifImageDesc frameInfo = savedImage.ImageDesc;
    //整幅图片的首地址
    int* px = (int *)pixels;
    //每一行的首地址
    int *line;

    //其中一个像素的位置  不是指针  在颜色表中的索引
    int pointPixel;
    //gif的字节类型
    GifByteType  gifByteType;
    //gif图片的像素颜色类型
    GifColorType gifColorType;
    //获取gif图片的颜色表
    ColorMapObject* colorMapObject=frameInfo.ColorMap;
    //计算像素，对每一个像素进行颜色填充
    px = (int *) ((char*)px + info.stride * frameInfo.Top);
    for (int y = frameInfo.Top; y < frameInfo.Top + frameInfo.Height; ++y) {
        //更新行数
        line = px;
        for (int x = frameInfo.Left; x < frameInfo.Left + frameInfo.Width; ++x) {
            pointPixel = (y - frameInfo.Top) * frameInfo.Width + (x - frameInfo.Left);
            //获取gif这个像素点的颜色类型，是red，green，blue中的一个，GifByteType 本身是无符号的字符 unsigned char
            gifByteType = savedImage.RasterBits[pointPixel];
            //获取到当前像素的颜色RGB的结构体
            gifColorType = colorMapObject->Colors[gifByteType];
            line[x] = argb(255,gifColorType.Red, gifColorType.Green, gifColorType.Blue);
        }
        //像素位置更新
        px = (int *) ((char*)px + info.stride);  //info.stride 表示是每行字节数的数量
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_jason_ndk_gifdemo_GifHandler_updateFrame(JNIEnv *env, jobject thiz, jlong gif_point,
                                                  jobject bitmap) {
    GifFileType *gifFileType = (GifFileType *) gif_point;
    //拿出我们所保持在GifFileType的GIFBean数据
    GifBean *gifBean = (GifBean *)gifFileType->UserData;
    AndroidBitmapInfo info;
    //代表一幅图片的像素数组
    void *pixel;
    //获取bitmap的图片信息
    AndroidBitmap_getInfo(env, bitmap, &info);
    //锁定bitmap
    AndroidBitmap_lockPixels(env, bitmap, &pixel);
    //绘制每一帧
    drawFrame(gifFileType, gifBean, info, pixel);
    //播放完成之后，循环到下一帧
    gifBean->current_frame += 1;
    LOGE("当前帧  %d  ",gifBean->current_frame);
    if (gifBean->current_frame >= gifBean->total_frame-1) {
        //当前播放到最后一帧，重置为第一帧，开始重新播放
        gifBean->current_frame = 0;
        LOGE("重新过来  %d  ",gifBean->current_frame);
    }
    //接触锁定bitmap
    AndroidBitmap_unlockPixels(env, bitmap);
    return gifBean->delays[gifBean->current_frame];
}