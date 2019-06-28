// #######################################################################
// NOTICE
//
// This software (or technical data) was produced for the U.S. Government
// under contract, and is subject to the Rights in Data-General Clause
// 52.227-14, Alt. IV (DEC 2007).
//
// Copyright 2019 The MITRE Corporation. All Rights Reserved.
// #######################################################################

#include <stdio.h>

#include "jnihelper.h"

#ifdef WIN32
#define snprintf _snprintf
#endif

/**
 *	Gets a pointer from the java object (stored as a long).
 *
 * @param env The java environment
 * @param obj The object to retrieve the pointer from
 * @param name The name of the java long storing the pointer value
 *
 * @return The retrieved pointer.
 */
void *jni_get_pointer(JNIEnv *env, jobject obj, const char *name)
{
    void *ptr;

    jclass jcls = (*env)->GetObjectClass(env, obj);
    jfieldID ptr_id = (*env)->GetFieldID(env, jcls, name, "J");
    ptr = (void *)((*env)->GetLongField(env, obj, ptr_id));

    return ptr;
}

/**
 * Sets a pointer in the java object (stored as a long.
 *
 * @param env The java environment
 * @param obj The object to set the pointer inside
 * @param name The name of the long which will store the pointer value.
 * @param ptr The pointer to be stored.
 */
void jni_set_pointer(JNIEnv *env, jobject obj, const char *name, void *ptr)
{
    jclass jcls = (*env)->GetObjectClass(env, obj);
    jfieldID ptr_id = (*env)->GetFieldID(env, jcls, name, "J");
    (*env)->SetLongField(env, obj, ptr_id, (long)ptr);
}

/**
 * Throws a java exception once java regains control. Best practice is to return
 * from native code immediately after calling this method. Not doing so may have
 * undesired results.
 *
 * @param env The java environment
 * @param exception The type of exception to be thrown, e.g.
 * "java/io/IOException"
 * @param message The accompanying message for the exception
 *
 * @return 0 on success, a negative value on failure.
 */
jint jni_throw_exception(JNIEnv *env, const char *exception,
                         const char *message)
{
    jclass ex = NULL;

    if (exception != NULL) {
        ex = (*env)->FindClass(env, exception);
    }
    if (ex == NULL) {
        ex = (*env)->FindClass(env, "java/lang/Exception");
    }

    return (*env)->ThrowNew(env, ex, message);
}

/**
 * Gets a reference to a java class with the provided path, e.g.
 * java/lang/String
 *
 * @param env The Java environment
 * @param class_name The name of the class to retrieve a reference to
 * @return
 */
jclass jni_get_class(JNIEnv *env, const char *class_name)
{
    jclass class_ptr;
    char message[1024];

    class_ptr = (*env)->FindClass(env, class_name);
    if (!class_ptr) {
        snprintf(message, 1024, "Unable to locate class %s", class_name);
        jni_throw_exception(env, "java/lang/ClassNotFoundException", message);
        return NULL;
    }
    return class_ptr;
}

jmethodID jni_get_method(JNIEnv *env, jclass class_ptr, const char *method_name,
                         const char *signature)
{
    jmethodID method;
    char message[1024];

    if (!class_ptr) {
        return NULL;
    }
    method = (*env)->GetMethodID(env, class_ptr, method_name, signature);
    if (!method) {
        snprintf(message, 1024, "Unable to locate method %s with signature %s",
                 method_name, signature);
        jni_throw_exception(env, "java/lang/NoSuchMethodException", message);
        return NULL;
    }
    return method;
}
