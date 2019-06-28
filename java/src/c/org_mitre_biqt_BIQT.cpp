// #######################################################################
// NOTICE
//
// This software (or technical data) was produced for the U.S. Government
// under contract, and is subject to the Rights in Data-General Clause
// 52.227-14, Alt. IV (DEC 2007).
//
// Copyright 2019 The MITRE Corporation. All Rights Reserved.
// #######################################################################

#include <memory>
#include "org_mitre_biqt_BIQT.h"
#include "BIQT.h"
#include "ProviderInterface.h"
#include "jnihelper.h"

/**
 * Initializes a Java BIQT object.
 *
 * @param env The Java environment
 * @param biqt The Java BIQT object
 */
JNIEXPORT void JNICALL
Java_org_mitre_biqt_BIQT_initialize(JNIEnv *env, jobject biqt)
{
    BIQT *app = new BIQT(); // create the object on the heap.
    jni_set_pointer(env, biqt, "biqt_ptr", (void *)app);
}

/**
 * Gets a list of available providers
 *
 * @param env The Java environment
 * @param biqt The Java BIQT object
 */
JNIEXPORT jobject JNICALL
Java_org_mitre_biqt_BIQT_getProviders(JNIEnv *env, jobject biqt)
{
    jstring name, version, description, modality;
    jclass piClass, listClass;
    jmethodID piConstructor, listConstructor, listAdd;
    jobject providerInfo, list;

    BIQT *app = (BIQT *)jni_get_pointer(env, biqt, "biqt_ptr");
    std::vector<ProviderInfo *> providers = app->getProviders();

    listClass = jni_get_class(env, "java/util/ArrayList");
    piClass = jni_get_class(env, "org/mitre/biqt/ProviderInfo");

    if (!(listConstructor = jni_get_method(env, listClass, "<init>", "()V")))
        return nullptr;
    if (!(listAdd =
              jni_get_method(env, listClass, "add", "(Ljava/lang/Object;)Z")))
        return nullptr;
    if (!(piConstructor =
              jni_get_method(env, piClass, "<init>",
                             "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/"
                             "String;Ljava/lang/String;)V")))
        return nullptr;

    list = env->NewObject(listClass, listConstructor);

    for (const auto p : providers) {
        // iterate through each provider and insert them into an ArrayList
        name = env->NewStringUTF(p->name.c_str());
        version = env->NewStringUTF(p->version.c_str());
        description = env->NewStringUTF(p->description.c_str());
        modality = env->NewStringUTF(p->modality.c_str());
        providerInfo = env->NewObject(piClass, piConstructor, name, version,
                                      description, modality);
        env->CallBooleanMethod(list, listAdd, providerInfo);
    }
    return list;
}

/**
 * Runs a provider with the provided name.
 *
 * @param env The Java environment
 * @param biqt The Java BIQT object
 * @param jprovider A Java String containing the provider name.
 * @param jinputFile A Java String containing the input file for the provider.
 */
JNIEXPORT jstring JNICALL Java_org_mitre_biqt_BIQT_runProvider(
    JNIEnv *env, jobject biqt, jstring jprovider, jstring jinputFile)
{
    const char *provider;
    const char *inputFile;
    Provider::EvaluationResult result;
    BIQT *app = (BIQT *)jni_get_pointer(env, biqt, "biqt_ptr");

    /* Get the strings from the java types */
    provider = env->GetStringUTFChars(jprovider, NULL);
    inputFile = env->GetStringUTFChars(jinputFile, NULL);

    result = app->runProvider(std::string(provider), std::string(inputFile));
    std::unique_ptr<char[]> resultStr(Provider::serializeResult(result));
    jstring jresult = env->NewStringUTF(resultStr.get());

    /* Release the strings back to java */
    env->ReleaseStringUTFChars(jprovider, provider);
    env->ReleaseStringUTFChars(jinputFile, inputFile);

    return jresult;
}

/**
 * Runs all providers with the given modality
 *
 * @param env The Java environment
 * @param biqt The Java BIQT object
 * @param jmodality A Java String containing the modality of the provider(s) to
 * run
 * @param jinputFile A Java String containing the path to the input file to
 * analyze
 */
JNIEXPORT jstring JNICALL Java_org_mitre_biqt_BIQT_runModality(
    JNIEnv *env, jobject biqt, jstring jmodality, jstring jinputFile)
{
    const char *modality;
    const char *inputFile;
    std::string resultStr;
    std::map<std::string, Provider::EvaluationResult> result;
    BIQT *app = (BIQT *)jni_get_pointer(env, biqt, "biqt_ptr");

    /* Get the strings from the java types */
    modality = env->GetStringUTFChars(jmodality, NULL);
    inputFile = env->GetStringUTFChars(jinputFile, NULL);

    result = app->runModality(std::string(modality), std::string(inputFile));
    for (const auto &iter : result) {
        if (resultStr.length()) {
            resultStr = resultStr + ",";
        }
        char* serialized_result = Provider::serializeResult(iter.second);
        resultStr = resultStr + serialized_result;
        delete[] serialized_result;
    }
    jstring jresult = env->NewStringUTF(resultStr.c_str());

    /* Release the strings back to java */
    env->ReleaseStringUTFChars(jmodality, modality);
    env->ReleaseStringUTFChars(jinputFile, inputFile);
    return jresult;
}

/**
 * Cleans up any allocated memory. This should only be called when you are
 * finished with the Java BIQT object.
 *
 * @param env The Java environment
 * @param biqt The Java BIQT object
 */
JNIEXPORT void JNICALL Java_org_mitre_biqt_BIQT_cleanup(JNIEnv *env,
                                                                 jobject biqt)
{
    BIQT *app = (BIQT *)jni_get_pointer(env, biqt, "biqt_ptr");
    delete app;
    jni_set_pointer(env, biqt, "biqt_ptr", (void *)NULL);
}
