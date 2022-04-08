//
// Created by YoungTr on 2022/4/4.
//

#ifndef JNIEVNER_LOG_H
#define JNIEVNER_LOG_H

#include <android/log.h>

#define TAG "JNIEnver"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

#endif //JNIEVNER_LOG_H
