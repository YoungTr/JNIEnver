#include <jni.h>
#include <string>
#include <android/log.h>

#define TAG "JNIEnver"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

extern "C" JNIEXPORT jstring JNICALL
Java_com_youngtr_jnievner_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}
extern "C"
JNIEXPORT void JNICALL
Java_com_youngtr_jnievner_MainActivity_visitField(JNIEnv *env, jobject thiz) {
    // 获取类 jclass
    jclass j_cls = env->GetObjectClass(thiz);
    // 获取实例变量id jfieldId
    jfieldID j_fieldId = env->GetFieldID(j_cls, "name", "Ljava/lang/String;");
    // 获取实例变量值
    jstring j_name = static_cast<jstring>(env->GetObjectField(thiz, j_fieldId));
    // 打印字符串
    char *name = const_cast<char *>(env->GetStringUTFChars(j_name, nullptr));
    LOGD("native gets field name: %s", name);
    // 设置实例变量新值
    jstring newName = env->NewStringUTF("Dog");
    env->SetObjectField(thiz, j_fieldId, newName);

    // 获取静态变量 id
    jfieldID j_staticFieldId = env->GetStaticFieldID(j_cls, "staticName", "Ljava/lang/String;");
    // 获取静态变量值
    jstring j_staticName = static_cast<jstring>(env->GetStaticObjectField(j_cls, j_staticFieldId));
    char *staticName = const_cast<char *>(env->GetStringUTFChars(j_staticName, nullptr));
    LOGD("native gets static field name: %s", staticName);
    // 设置静态变量新值
    jstring newStaticName = env->NewStringUTF("Static Dog");
    env->SetStaticObjectField(j_cls, j_staticFieldId, newStaticName);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_youngtr_jnievner_MainActivity_visitMethod(JNIEnv *env, jobject thiz) {
    // 获取类
    jclass j_cls = env->GetObjectClass(thiz);
    // 获取实例方法ID
    jmethodID j_methodId = env->GetMethodID(j_cls, "minus", "(I)I");
    // 调用实例方法
    jint minus = env->CallIntMethod(thiz, j_methodId, 1);
    LOGD("native call minus: %d", minus);

    // 获取静态方法ID
    jmethodID j_staticMethodId = env->GetStaticMethodID(j_cls, "add", "(II)I");
    // 调用静态
    jint add = env->CallStaticIntMethod(j_cls, j_staticMethodId, 10, 20);
    LOGD("native call add: %d", add);
}