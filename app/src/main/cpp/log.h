//
// Created by 天涯路 on 2020/12/29.
//

#ifndef NEHUFFMANENCODING_LOG_H
#define NEHUFFMANENCODING_LOG_H

#include <android/log.h>

#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR, "sty", FORMAT, ## __VA_ARGS__);
#define LOGE2(...) __android_log_print(ANDROID_LOG_ERROR, "NEPLAYER_NATIVE",__VA_ARGS__)

#endif //NEHUFFMANENCODING_LOG_H
