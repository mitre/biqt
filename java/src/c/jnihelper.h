// #######################################################################
// NOTICE
//
// This software (or technical data) was produced for the U.S. Government
// under contract, and is subject to the Rights in Data-General Clause
// 52.227-14, Alt. IV (DEC 2007).
//
// Copyright 2019 The MITRE Corporation. All Rights Reserved.
// #######################################################################

#ifndef JNI_HELPER_H
#define JNI_HELPER_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

void *jni_get_pointer(JNIEnv *, jobject, const char *name);
void jni_set_pointer(JNIEnv *, jobject, const char *name, void *ptr);
jint jni_throw_exception(JNIEnv *, const char *exception, const char *message);
jclass jni_get_class(JNIEnv *, const char *class_name);
jmethodID jni_get_method(JNIEnv *, jclass, const char *method_name,
                         const char *signature);

#ifdef __cplusplus
}
#endif

#endif
