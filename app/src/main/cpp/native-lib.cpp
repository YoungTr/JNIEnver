#include <jni.h>
#include <string>
#include <sys/shm.h>
#include <dlfcn.h>
#include "mbedtls/aes.h"
#include "log.h"
#include "zlib_example.h"
#include "zlib_util.h"
#include "ptracer/ptracer.h"
#include "xdl/xdl.h"

#define TAG "JNIEnver"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

#define NUM_METHODS(x) ((int) (sizeof(x) / sizeof((x)[0]))) //获取方法的数量


void nativeRegister(JNIEnv *env, jobject thiz, jstring j_name) {
    char *name = const_cast<char *>(env->GetStringUTFChars(j_name, nullptr));
    LOGD("The name is %s", name);
}


extern "C" JNIEXPORT jstring JNICALL
Java_com_youngtr_jnievner_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++12";
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
extern "C"
JNIEXPORT jobject JNICALL
Java_com_youngtr_jnievner_MainActivity_getPerson(JNIEnv *env, jobject thiz) {

    // 获取 Person class
    jclass p_cls = env->FindClass("com/youngtr/jnievner/Person");
    // 获取构造函数id
    jmethodID p_initMethod = env->GetMethodID(p_cls, "<init>", "(Ljava/lang/String;I)V");
    // 调用构造函数
    jobject person = env->NewObject(p_cls, p_initMethod, env->NewStringUTF("Cat"), 20);
    return person;
}
extern "C"
JNIEXPORT jobjectArray JNICALL
Java_com_youngtr_jnievner_MainActivity_getPersons(JNIEnv *env, jobject thiz, jobjectArray names) {
    // 获取 Person class
    jclass p_cls = env->FindClass("com/youngtr/jnievner/Person");
    // 获取构造函数id
    jmethodID p_initMethod = env->GetMethodID(p_cls, "<init>", "(Ljava/lang/String;)V");
    // 获取数组长度
    jsize length = env->GetArrayLength(names);
    // 创建 jni 数组 jObjectArray
    jobjectArray j_array = env->NewObjectArray(length, p_cls, nullptr);
    int i;
    for (i = 0; i < length; ++i) {
        jstring name = static_cast<jstring>(env->GetObjectArrayElement(names, i));
        if (name == nullptr) {
            return nullptr;
        }
        jobject obj = env->NewObject(p_cls, p_initMethod, name);
        env->SetObjectArrayElement(j_array, i, obj);

    }

//    char *key = "0123456789abcdef";
//    char *iv = "fedcba0987654321";
//
//    static unsigned char KEY[16] = {0};
//    static unsigned char IV[16] = {0};
//
//    memcpy(KEY, key, 16);
//    memcpy(IV, iv, 16);
//
//    char *pwd = "2134567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef";
//    unsigned char buf[64];
//
//    LOGD("KEY: %s", KEY);
//    LOGD("IV: %s", IV);
//
//
//    mbedtls_aes_context context;
//    mbedtls_aes_setkey_enc(&context, (unsigned char *) KEY, 128);
//
//    LOGD("start encrypt...");
//
//    LOGD("pwd len: %d", strlen(pwd));
//
//    int result = mbedtls_aes_self_test(1);
//    LOGD("result: %d", result);
//
//    mbedtls_aes_crypt_cbc(&context, MBEDTLS_AES_ENCRYPT, 64, (unsigned char *) iv,
//                          (unsigned char *) pwd,
//                          buf);


    return j_array;
}

void mbedtls_aes_test(void) {
    int i;

    mbedtls_aes_context aes_ctx;

    //密钥数值
    unsigned char key[16] = {'c', 'b', 'c', 'p', 'a', 's', 's', 'w', 'o', 'r', 'd', '1', '2', '3',
                             '4'};
    //iv
    unsigned char iv[16];

    //明文空间
    unsigned char plain[64] = "hello_worled1234567890abcdefghijklmnopqrstuvwxyz12389";
    //解密后明文的空间
    unsigned char dec_plain[64] = {0};
    //密文空间
    unsigned char cipher[64] = {0};


    mbedtls_aes_init(&aes_ctx);

    //设置加密密钥
    LOGD("plain:%s\r\n", plain);
    mbedtls_aes_setkey_enc(&aes_ctx, key, 128);
    for (i = 0; i < 16; i++) {
        iv[i] = 0x01;
    }
    mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_ENCRYPT, 64, iv, plain, cipher);
    printf("cipher:%s\r\n", cipher);

    //设置解密密钥
    mbedtls_aes_setkey_dec(&aes_ctx, key, 128);
    for (i = 0; i < 16; i++) {
        iv[i] = 0x01;
    }
    mbedtls_aes_crypt_cbc(&aes_ctx, MBEDTLS_AES_DECRYPT, 64, iv, cipher, dec_plain);
    printf("dec_plain:%s\r\n", dec_plain);
    printf("\r\n");
    mbedtls_aes_free(&aes_ctx);
}


extern "C"
JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = nullptr;
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    jclass j_cls = env->FindClass("com/youngtr/jnievner/MainActivity");
    JNINativeMethod method[] = {
            {"dynamicRegister", "(Ljava/lang/String;)V", (void *) nativeRegister}};

    // 注册方法
    jint r = env->RegisterNatives(j_cls, method, NUM_METHODS(method));
    if (r != JNI_OK) {
        return JNI_ERR;
    }

    env->DeleteLocalRef(j_cls);

    mbedtls_aes_test();

    return JNI_VERSION_1_6;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_youngtr_jnievner_MainActivity_compress(JNIEnv *env, jobject thiz, jstring source) {
    char *str = const_cast<char *>(env->GetStringUTFChars(source, nullptr));
    compress_stream(str);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_youngtr_jnievner_MainActivity_initFile(JNIEnv *env, jobject thiz, jstring path) {
    char *file_path = const_cast<char *>(env->GetStringUTFChars(path, nullptr));
    init_file(file_path);
}
extern "C"
JNIEXPORT jstring JNICALL
Java_com_youngtr_jnievner_MainActivity_getStream(JNIEnv *env, jobject thiz) {
    char stream[1024];
    uncompress_stream(stream);
    return env->NewStringUTF(stream);

}
extern "C"
JNIEXPORT void JNICALL
Java_com_youngtr_jnievner_MainActivity_zlib(JNIEnv *env, jobject thiz, jstring source,
                                            jstring dest) {
    char *s = const_cast<char *>(env->GetStringUTFChars(source, nullptr));
    char *d = const_cast<char *>(env->GetStringUTFChars(dest, nullptr));
    FILE *sp = fopen(s, "ab+");
    FILE *dp = fopen(d, "ab+");
    def(sp, dp, Z_DEFAULT_COMPRESSION);

    inf(dp, sp);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_youngtr_jnievner_DLActivity_nativeDLOpen(JNIEnv *env, jobject thiz) {

//    void *handle = dlopen("libdl.so", RTLD_NOW);
//    LOGD("handler: %d", (uintptr_t) handle);
//    if (handle != NULL) {
//        void *open = dlsym(handle, "__loader_dlopen");
//        LOGD("open: 0x%u", (uintptr_t) open);
//    }

//    xdl_iterate_by_link("/apex/com.android.art/lib/libart.so");
    void *handler = xdl_open("/apex/com.android.art/lib/libart.so");
    LOGD("handler: 0x%X", (uintptr_t) handler);
    void *suspend = nullptr;
    if (nullptr != handler) {
       suspend =  xdl_sym(handler, "_ZN3art16ScopedSuspendAllC1EPKcb");
    }
    LOGD("suspend: 0x%X", (uintptr_t) suspend);


//xdl_iterate_by_maps("/apex/com.android.art/lib/libart.so");
}