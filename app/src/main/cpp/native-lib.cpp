#include <jni.h>
#include <string>
#include <android/bitmap.h>
#include <jpeglib.h>
#include "log.h"

void write_jpeg_file(uint8_t *temp, int w, int h, jint quality, const char *path);

extern "C" JNIEXPORT jstring JNICALL
Java_com_sty_ne_huffmanencoding_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

void write_jpeg_file(uint8_t *data, int w, int h, jint quality, const char *path) {
    //3.1 创建JPEG压缩对象
    jpeg_compress_struct jcs;
    //错误回调
    jpeg_error_mgr error;
    jcs.err = jpeg_std_error(&error);
    //创建压缩对象
    jpeg_create_compress(&jcs);
    //3.2 指定存储文件 write binary
    FILE *f = fopen(path, "wb");
    jpeg_stdio_dest(&jcs, f);
    //3.3 设置压缩参数
    jcs.image_width = w;
    jcs.image_height = h;
    LOGE("222 width: %d, height: %d",jcs.image_width, jcs.image_height);
    //bgr
    jcs.input_components = 3;
    jcs.in_color_space = JCS_RGB;
    jpeg_set_defaults(&jcs);
    //开启哈夫曼功能
    jcs.optimize_coding = true;
    jpeg_set_quality(&jcs, quality, 1);
    //3.4 开始压缩
    jpeg_start_compress(&jcs, 1);
    //3.5 循环写入每一行数据
    int row_stride = w * 3; //一行的字节数
    JSAMPROW row[1];
    while(jcs.next_scanline < jcs.image_height) {
        //取一行数据
        uint8_t *pixels = data + jcs.next_scanline * row_stride;
        row[0] = pixels;
        jpeg_write_scanlines(&jcs, row, 1);
    }
    //3.6 压缩完成
    jpeg_finish_compress(&jcs);
    //3.7 释放JPEG对象
    fclose(f);
    jpeg_destroy_compress(&jcs);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_sty_ne_huffmanencoding_MainActivity_compressImage(JNIEnv *env, jobject thiz,
                                                           jobject bitmap, jstring path_,
                                                           jint quality) {
    const char *path = env->GetStringUTFChars(path_, 0);
    AndroidBitmapInfo info; //new Info();
    //获取bitmap中的数据
    int ret = AndroidBitmap_getInfo(env, bitmap, &info); //返回0表示成功，负数表示失败
    if(ret < 0) {
        LOGE("compressImage AndroidBitmap_getInfo() failed! error=%d", ret);
        return;
    }
    //定义一个pixels，得到所有的图片像素
    uint8_t *pixels; //java new byte[]
    ret = AndroidBitmap_lockPixels(env, bitmap, (void **)&pixels); //(void**)：万能指针  //锁定像素缓存以确保像素的内存不会被移动，返回0表示成功，负数表示失败
    if(ret < 0) {
        LOGE("compressImage AndroidBitmap_lockPixels() failed! error=%d", ret);
        return ;
    }
    int w = info.width;
    int h = info.height;
    LOGE("111 width: %d, height: %d", w, h);
    int color;
    //开辟一块内存用来存放RGB信息
    uint8_t *data = static_cast<uint8_t *>(malloc(w * h * 3));

    uint8_t *temp = data;
    uint8_t r,g,b;
    //循环获取图片中的每个像素
    for(int i = 0; i < h; i++) {
        for(int j = 0; j < w; j++) {
            color = *(int*)pixels;  //指针转型
            //argb
            r = (color >> 16) & 0xFF;
            g = (color >> 8) & 0xFF;
            b = color & 0xFF;

            //存放：jpeg-->bgr
            *data = b;
            *(data + 1) = g;
            *(data + 2) = r;

            data += 3;
            pixels += 4; //抛弃alpha通道
        }
    }

    //将得到的新的图片存入到新文件
    write_jpeg_file(temp, w, h, quality, path);
    free(temp);
    //free(data);

    AndroidBitmap_unlockPixels(env, bitmap); //返回0表示成功，1表示失败
    env->ReleaseStringUTFChars(path_, path);
}

/**
 * 生成一个bitmap
 * 参考：https://www.cnblogs.com/glumes/p/12492886.html
 * @param env
 * @param width
 * @param height
 * @return
 */
jobject generateBitmap(JNIEnv *env, uint32_t width, uint32_t height) {

    jclass bitmapCls = env->FindClass("android/graphics/Bitmap");
    jmethodID createBitmapFunction = env->GetStaticMethodID(bitmapCls,
                                                            "createBitmap",
                                                            "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
    jstring configName = env->NewStringUTF("ARGB_8888");
    jclass bitmapConfigClass = env->FindClass("android/graphics/Bitmap$Config");
    jmethodID valueOfBitmapConfigFunction = env->GetStaticMethodID(
            bitmapConfigClass, "valueOf",
            "(Ljava/lang/String;)Landroid/graphics/Bitmap$Config;");

    jobject bitmapConfig = env->CallStaticObjectMethod(bitmapConfigClass,
                                                       valueOfBitmapConfigFunction, configName);

    jobject newBitmap = env->CallStaticObjectMethod(bitmapCls,
                                                    createBitmapFunction,
                                                    width,
                                                    height, bitmapConfig);

    return newBitmap;
}

extern "C"
JNIEXPORT jobject JNICALL
Java_com_sty_ne_huffmanencoding_MainActivity_rotateBitmap(JNIEnv *env, jobject thiz,
                                                          jobject bitmap) {
    LOGE2("rotate bitmap");

    AndroidBitmapInfo bitmapInfo;
    int ret;
    if ((ret = AndroidBitmap_getInfo(env, bitmap, &bitmapInfo)) < 0) {
        LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
        return NULL;
    }

    // 读取 bitmap 的像素内容到 native 内存
    void *bitmapPixels;
    if ((ret = AndroidBitmap_lockPixels(env, bitmap, &bitmapPixels)) < 0) { //锁定像素缓存以确保像素的内存不会被移动，返回0表示成功，负数表示失败
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
        return NULL;
    }

    // 旋转操作，新 Bitmap 的宽等于原来的高，新 Bitmap 的高等于原来的宽
    uint32_t newWidth = bitmapInfo.height;
    uint32_t newHeight = bitmapInfo.width;

    // 创建一个新的数组指针，把这个新的数组指针填充像素值
    uint32_t *newBitmapPixels = new uint32_t[newWidth * newHeight];
    int whereToGet = 0;
    //顺时针旋转90°操作
    // 弄明白 bitmapPixels 的排列，这里不同于二维数组了。
    for (int x = newWidth; x >= 0; --x) {
        for (int y = 0; y < newHeight; ++y) {
            uint32_t pixel = ((uint32_t *) bitmapPixels)[whereToGet++];
            newBitmapPixels[newWidth * y + x] = pixel;
        }
    }

    //上下翻转操作
//    int whereToGet = 0;
//    for (int y = 0; y < newHeight; ++y) {
//        for (int x = 0; x < newWidth; x++) {
//            uint32_t pixel = ((uint32_t *) bitmapPixels)[whereToGet++];
//            newBitmapPixels[newWidth * (newHeight - 1 - y) + x] = pixel;
//        }
//    }

    //左右镜像操作
//    int whereToGet = 0;
//    for (int y = 0; y < newHeight; ++y) {
//        for (int x = newWidth - 1; x >= 0; x--) {
//            uint32_t pixel = ((uint32_t *) bitmapPixels)[whereToGet++];
//            newBitmapPixels[newWidth * y + x] = pixel;
//        }
//    }

    jobject newBitmap = generateBitmap(env, newWidth, newHeight);

    void *resultBitmapPixels;
    if ((ret = AndroidBitmap_lockPixels(env, newBitmap, &resultBitmapPixels)) < 0) { //锁定像素缓存以确保像素的内存不会被移动，返回0表示成功，负数表示失败
        LOGE("AndroidBitmap_lockPixels() failed ! error=%d", ret);
        return NULL;
    }
    int pixelsCount = newWidth * newHeight;
    memcpy((uint32_t *) resultBitmapPixels, newBitmapPixels, sizeof(uint32_t) * pixelsCount);

    AndroidBitmap_unlockPixels(env, newBitmap);
    AndroidBitmap_unlockPixels(env, bitmap);

    delete[] newBitmapPixels;
    return newBitmap;
}

