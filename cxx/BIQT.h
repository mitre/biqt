// #######################################################################
// NOTICE
//
// This software (or technical data) was produced for the U.S. Government
// under contract, and is subject to the Rights in Data-General Clause
// 52.227-14, Alt. IV (DEC 2007).
//
// Copyright 2019 The MITRE Corporation. All Rights Reserved.
// #######################################################################

#ifndef APPLICATION_H
#define APPLICATION_H

#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <vector>

#include "ProviderInterface.h"

#define __BIQT_VERSION__ BIQT_VERSION

#ifdef _WIN32
#include <windows.h>
#define LIB_HANDLE HINSTANCE
#else
#define LIB_HANDLE void *
#endif

typedef char *(*evaluator)(const char *filePath);

class DLL_EXPORT ProviderInfo {
  public:
    ProviderInfo(std::string modulePath, std::string lib);
    ~ProviderInfo();
    const char *evaluate(std::string filename) const;
    std::string name;
    std::string version;
    std::string description;
    std::string modality;
    std::string sourceLanguage;
    std::string className;
    evaluator eval;

  private:
#ifdef BIQT_JAVA_SUPPORT
    std::string getClassPath(std::string modulePath);
    std::string classPath;
#endif
    std::string soPath;
    LIB_HANDLE handle = nullptr;
};

class DLL_EXPORT BIQT {

  public:
    BIQT();
    ~BIQT();

    std::string version() const;
    std::string modulePath;

    std::vector<ProviderInfo *> getProviders();
    Provider::EvaluationResult runProvider(const std::string &pName,
                                           const std::string &filePath);
    Provider::EvaluationResult runProvider(const ProviderInfo *p,
                                           const std::string &filePath);
    std::map<std::string, Provider::EvaluationResult>
    runModality(const std::string &modality, const std::string &filePath);
    static bool fileExists(const std::string &filename);

  private:

    const ProviderInfo *getProvider(const std::string &p);
    std::vector<ProviderInfo *> providers;
    std::set<std::string> providerLibs();
};

#endif
