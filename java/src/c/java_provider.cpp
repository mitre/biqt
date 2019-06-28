// #######################################################################
// NOTICE
//
// This software (or technical data) was produced for the U.S. Government
// under contract, and is subject to the Rights in Data-General Clause
// 52.227-14, Alt. IV (DEC 2007).
//
// Copyright 2019 The MITRE Corporation. All Rights Reserved.
// #######################################################################

#include <iostream>

#include "jnihelper.h"
#include "org_mitre_biqt_BIQT.h"
#include "java_provider.h"

#define CLASSPATH "providers/BIQTDummy/biqt-0.1-dev.jar:providers/BIQTDummy/json-simple-1.1.1.jar"

/**
 * Initializes the Java Virtual Machine.
 *
 * @param jvm An uninitialized pointer to a pointer to the JVM
 * @param env An uninitialized pointer to a pointer to the JNI environment
 * @param cls An uninitialized pointer for the Java class
 * @param className The name of the java class to load into the JVM
 * @return
 */
int init_jvm(JavaVM **jvm, JNIEnv **env, jclass *cls, std::string className,
             std::string classPath)
{
    char classPathOption[2048] = "-Djava.class.path=";
    strncpy(classPathOption+18, classPath.c_str(), 2030);
    /* ================= prepare loading of Java VM ========================== */
    JavaVMInitArgs vm_args;                        // Initialization arguments
    JavaVMOption *options = new JavaVMOption[1];   // JVM invocation options
    options[0].optionString = classPathOption;
    vm_args.version = JNI_VERSION_1_6;             // minimum Java version
    vm_args.nOptions = 1;                          // number of options
    vm_args.options = options;
    vm_args.ignoreUnrecognized = false;  // invalid options make the init fail
    /* ============ load and initialize Java VM and JNI interface =========== */
    jint rc = JNI_CreateJavaVM(jvm, (void**)env, &vm_args);  // YES !!
    delete options;    // we then no longer need the initialization options.
    /* ============== Check for initialization errors ======================= */
    if (rc != JNI_OK) {
        // TO DO: error processing...
        std::cerr << "Unable to initialize the JVM" << std::endl;
        return -1;
    }

    *cls = (*env)->FindClass(className.c_str());
    if (*cls == nullptr) {
        std::cerr << "Unable to locate the class " << className << std::endl;
        return -1;
    }

//    /* ============== Display JVM version ================================= */
//    std::cout << "JVM load succeeded: Version ";
//    jint ver = env->GetVersion();
//    std::cout << ((ver>>16)&0x0f) << "."<<(ver&0x0f) << std::endl;

    return 0;
}

void destroy_jvm(JavaVM *jvm)
{
    jvm->DestroyJavaVM();
}

/**
 * A function to begin provider analysis.
 *
 * @param filePath The path to the input file.
 * @param providerName This should be the provider *directory* name where the
 * jar files can be found.
 * @param className The name of the Provider class to use for evaluation. This
 * should be the fully qualified class name, e.g.
 * org/mitre/biqt/IrisProvider
 * @param classPath The java class path required for this provider.
 *
 * @return The return status of the provider.
 */
const char *java_provider_eval(const char *filePath, const char *providerName,
                               const char *className, const char *classPath)
{
    JavaVM *jvm;
    JNIEnv *env;
    jclass cls, jsonObject;
    jobject provider, jsonResult;
    jstring jfilename, jresultStr;
    jmethodID constructor, execMethod, toStringMethod;
    const char *result;
    char *returnvalue;

    if (init_jvm(&jvm, &env, &cls, className, classPath)) {
        return nullptr;
    }
    if (!(constructor = jni_get_method(env, cls, "<init>", "()V"))) {
        // log the error;
        std::cerr << "Unable to locate the constructor for" << className
                  << std::endl;
        return nullptr;
    }
    if (!(execMethod = jni_get_method(env, cls, "evaluate",
                                      "(Ljava/lang/String;)"
                                      "Lorg/json/simple/JSONObject;"))) {
        // log error
        std::cerr << "Unable to locate the evaluate method for " << className
                  << std::endl;
        return nullptr;
    }
    jfilename = env->NewStringUTF(filePath);
    provider = env->NewObject(cls, constructor);
    jsonResult = env->CallObjectMethod(provider, execMethod, jfilename);
    env->ReleaseStringUTFChars(jfilename, NULL);

    jsonObject = env->FindClass("org/json/simple/JSONObject");
    if (!jsonObject) {
        std::cerr << "Unable to locate the class org/json/simple/JSONObject"
                  << std::endl;
        return nullptr;
    }

    if (!(toStringMethod = jni_get_method(env, jsonObject, "toString",
                                          "()Ljava/lang/String;"))) {
        // log error
        std::cerr << "Unable to locate the toString method for"
                " org/json/simple/JSONObject" << std::endl;
        return nullptr;
    }
    jresultStr = (jstring)(env->CallObjectMethod(jsonResult, toStringMethod));

    result = env->GetStringUTFChars(jresultStr, NULL);
    returnvalue = new char[strlen(result)];
    strcpy(returnvalue, result);
    env->ReleaseStringUTFChars(jresultStr, NULL);
    destroy_jvm(jvm);
    return returnvalue;
}

