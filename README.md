# JNIEnver

## JNI 访问 Java 中的成员变量和方法

### 访问成员变量

访问实例变量或者静态变量，一般都需要三个步骤：

1. 获取类
2. 获取变量 ID
3. 获取变量域值

Java 中的变量定义
```Java
private String name = "Cat";
private static String staticName = "Static Cat";

public native void visitField();
```

jni中获取时，GetFieldID方法的第二个参数为变量的名称，第三个参数为变量的签名描述符

```C++
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
```

原生代码访问 Java 变量需要调用两到三个 jni 函数，给程序增加了额外的负担，导致性能的下降。一般情况下，建议将所有需要的参数传递给原生方法调用。


### 访问成员方法

访问实例方法或者静态方法，一般都需要三个步骤：

1. 获取类
2. 获取方法 ID
3. 调用方法

Java 中定义的方法

```Java
public static int add(int a, int b) {
    return a + b;
}
    
public int minus(int total) {
    return total - 100;
}
```

在jni中调用，并获得其返回值，GetMethodID第二个参数为方法名称，第三个参数为方法的签名描述符

```C++
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
```

Java 和原生代码之间的转换代价较大，程序设计上要最小化在原生代码调用java方法，以提高程序的性能。

### 签名描述符


| Java Language Type | Field Desciptor |
|--------------------|-----------------|
| boolean            | Z               |
| byte               | B               |
| char               | C               |
| short              | S               |
| int                | I               |
| long               | J               |
| float              | F               |
| double             | D               |
| void               | V               |

对于引用类型，描述符是以"L"开头，";"结尾，对于数组类型，使用"["和对应的类型描述符来表述

| Java Language Type | Field Desciptor     |
|--------------------|---------------------|
| String             | Ljava/lang/String;  |
| Object[]           | [Ljava/lang/Object; |
| int[][]            | [[[D                |

方法描述符一般格式为：(参数desciptor)返回值desciptor


| Java Language Method                  | Method Descriptor       |
|---------------------------------------|-------------------------|
| void fun(long v1, String v2, long v3) | (JLjava/lang/String;J)V |
| String f();                           | ()Ljava/lang/String;    |
| void fun(byte[] bytes);               | ([B)V                   |

