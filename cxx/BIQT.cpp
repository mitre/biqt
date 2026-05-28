// #######################################################################
// NOTICE
//
// This software (or technical data) was produced for the U.S. Government
// under contract, and is subject to the Rights in Data-General Clause
// 52.227-14, Alt. IV (DEC 2007).
//
// Copyright 2019 The MITRE Corporation. All Rights Reserved.
// #######################################################################

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <json/json.h>
#include <map>
#include <stdexcept>
#include <sstream>
#include <sys/stat.h>

#ifdef _WIN32
#include "windows/dirent.h"
#include "windows/libgen.h"
#include <windows.h>
#define dlopen(name, flag) LoadLibraryEx(name, NULL, LOAD_WITH_ALTERED_SEARCH_PATH)
#define dlhandle(name) GetModuleHandle(name)
#define dlsym(handle, symbol) GetProcAddress(handle, symbol)
#define dlclose(name) FreeLibrary(name)
#define dlerror() GetLastError()
#define setenv(key, val, int) _putenv_s(key, val)
#define PATHSEP ";"
#define DIRSEP "\\"
#else
#define dlhandle(name) dlopen(name, RTLD_NOLOAD)
#include <dirent.h>
#include <dlfcn.h>
#include <libgen.h>
#define PATHSEP ":"
#define DIRSEP "/"
#endif

#include "BIQT.h"
#include "ProviderInterface.h"
#ifdef BIQT_JAVA_SUPPORT
#include "java_provider.h"
#endif

ProviderInfo::ProviderInfo(std::string modulePath, std::string lib)
{
    Json::Value desc;
    this->soPath = modulePath + DIRSEP + "providers" DIRSEP + lib + DIRSEP +
#if defined(_WIN32)
       lib + ".dll";
#elif defined(__APPLE__)
       "lib" + lib + ".dylib";
#else
       "lib" + lib + ".so";
#endif
    if (BIQT::fileExists(this->soPath)) {
        this->handle = dlopen(this->soPath.c_str(), RTLD_NOW);
        if (!this->handle) {
            throw std::runtime_error(
                    "Provider Read error: Unable to load the shared object: " + std::string(dlerror()));
        }
    }

    std::string desc_path =
        modulePath + "/providers/" + lib + "/descriptor.json";
    std::ifstream desc_file(desc_path.c_str(), std::ifstream::binary);
    if (!desc_file) {
        throw std::runtime_error("Provider Read error: Unable to open descriptor: " + desc_path);
    }
    desc_file >> desc;
    if (!desc.isObject()) {
        throw std::runtime_error("Provider Read error: Invalid descriptor: " + desc_path);
    }

    this->name = std::string((desc["name"]).asString());
    this->version = std::string((desc["version"]).asString());
    this->description = std::string((desc["description"]).asString());
    this->modality = std::string((desc["modality"]).asString());
    this->sourceLanguage = std::string((desc["sourceLanguage"]).asString());
#ifdef BIQT_JAVA_SUPPORT
    if (this->sourceLanguage == "java") {
        this->className = std::string((desc["className"]).asString());
        this->classPath = this->getClassPath(modulePath + "/providers/" + lib);
    } else {
#endif
        if (!this->handle) {
            throw std::runtime_error("Provider Read error: Missing shared object: " + this->soPath);
        }
        this->eval = (evaluator)dlsym(this->handle, "provider_eval");
        if (!this->eval) {
            throw std::runtime_error("Provider API Error:"
                                 "Unable to locate the provider_eval function");
        }
        this->free_result = (result_deleter)dlsym(this->handle, "provider_free");
#ifdef BIQT_JAVA_SUPPORT
    }
#endif
}

#ifdef BIQT_JAVA_SUPPORT
namespace {
bool hasSuffix(const char *name, const char *suffix)
{
    size_t name_length = strlen(name);
    size_t suffix_length = strlen(suffix);
    return name_length >= suffix_length &&
           !strcmp(name + name_length - suffix_length, suffix);
}
}

std::string ProviderInfo::getClassPath(std::string providerPath)
{
    struct dirent *module;
    std::set<std::string> paths;
    std::string classPath = "";
    DIR *dir;
    std::string biqt_home = getenv("BIQT_HOME");

    if (!BIQT::fileExists(providerPath)) {
        return "";
    }
    /* find the BIQT jar files */
    dir = opendir(biqt_home.c_str());
    if (!dir) {
        return "";
    }
    while ((module = readdir(dir)) != nullptr) {
        if (hasSuffix(module->d_name, ".jar")) {
            paths.insert(biqt_home + DIRSEP + module->d_name);
        }
    }
    closedir(dir);
    /* find the jar files for the provider */
    dir = opendir(providerPath.c_str());
    if (!dir) {
        return classPath;
    }
    while ((module = readdir(dir)) != nullptr) {
        if (hasSuffix(module->d_name, ".jar")) {
            paths.insert(providerPath + DIRSEP + module->d_name);
        }
    }
    closedir(dir);
    for (const auto &path : paths) {
        if (classPath.length()) {
            classPath += PATHSEP;
        }
        classPath += path;
    }
    return classPath;
}
#endif

ProviderInfo::~ProviderInfo()
{
    this->eval = nullptr;
    if (this->handle) {
        dlclose(this->handle);
    }
}

char *ProviderInfo::evaluate(std::string filename) const
{
#ifdef BIQT_JAVA_SUPPORT
    if (this->sourceLanguage == "java") {
        char *returnvalue = java_provider_eval(filename.c_str(), this->name.c_str(),
                this->className.c_str(), this->classPath.c_str());
        if (!returnvalue) {
            std::cerr << "An error occurred indicating a problem with the"
                         " selected provider: " << this->name << ". Please "
                         "check with the provider vendor for updates."
                          << std::endl;
        }
        return returnvalue;
    }
    else
#endif
    if (this->eval) {
        return this->eval(filename.c_str());
    }
    else {
        std::cerr << "ERROR: Missing provider evaluator for " << this->name
                  << "." << std::endl;
        return nullptr;
    }
}

void ProviderInfo::freeResult(char *result) const
{
    if (!result) {
        return;
    }
    if (this->free_result) {
        this->free_result(result);
    }
    else {
        delete[] result;
    }
}

BIQT::BIQT()
{
    /* Check default installation paths */
    char *biqt_home = getenv("BIQT_HOME");

    /* if BIQT_HOME is already set, look for the providers directory */
    if ((biqt_home)) {
        this->modulePath = std::string(biqt_home);
        if (!this->fileExists(std::string(biqt_home) + "/providers")) {
            std::cerr << "ERROR: BIQT_HOME set to '" << biqt_home
                      << "', but no providers directory found!" << std::endl;
        }
    }
    else if (getenv("BIQT_ALLOW_LOCAL_PROVIDERS")) {
        std::cerr << "WARNING: BIQT_HOME is not set; using the current "
            "directory because BIQT_ALLOW_LOCAL_PROVIDERS is set." << std::endl;
        setenv("BIQT_HOME", ".", 1);
        this->modulePath = std::string(".");
    }
    else {
        std::cerr << "ERROR: BIQT_HOME is not set. Refusing to load providers "
            "from the current directory. Set BIQT_HOME or set "
            "BIQT_ALLOW_LOCAL_PROVIDERS=1 for development-only local loading."
                  << std::endl;
    }
    this->getProviders();
}

BIQT::~BIQT()
{
    for (const auto p : this->providers) {
        delete p;
    }
}

/**
 * Returns the BIQT version number
 *
 * @return The current version number.
 */
std::string BIQT::version() const { return __BIQT_VERSION__; }

/**
 * Determines whether a file or directory exists and is readable.
 *
 * @param filename The path to the desired file or directory.
 * @return true if it exists and is readable, false otherwise.
 */
bool BIQT::fileExists(const std::string &filename)
{
    struct stat info;
    return stat(filename.c_str(), &info) == 0;
}

std::set<std::string> BIQT::providerLibs()
{
    std::set<std::string> returnValue;
    if (this->modulePath.empty()) {
        return returnValue;
    }
    std::string provider_path = this->modulePath + "/providers";
    struct dirent *pro;

    DIR *dir = opendir(provider_path.c_str());
    if (dir) {
        while ((pro = readdir(dir)) != nullptr) {
            if (pro->d_name[0] != '.') {
                returnValue.insert(std::string(pro->d_name));
            }
        }
        closedir(dir);
    }
    return returnValue;
}

/**
 * Creates a vector of application providers and stores it in the object.
 *
 * @return The newly created vector.
 */
std::vector<ProviderInfo *> BIQT::getProviders()
{
    std::set<std::string> libs;
    std::string providerPath;
    Json::Value desc;

    if (!this->providers.empty()) {
        return this->providers;
    }

    libs = this->providerLibs();
    for (const auto &lib : libs) {
        try {
            ProviderInfo *p = new ProviderInfo(this->modulePath, lib);
            this->providers.push_back(p);
        }
        catch (std::runtime_error e) {
            std::cerr << e.what();
        }
    }
    return this->providers;
}

/**
 * Gets a single provider by name
 *
 * @param pname The name of the provider
 * @return The requested provider, or NULL if no such provider exists.
 */
const ProviderInfo *BIQT::getProvider(const std::string &pname)
{
    for (auto &provider : this->providers) {
        if (provider->name == pname)
            return provider;
    }
    return nullptr;
}

/**
 * Runs a particular provider with the given arguments.
 *
 * @param pName The name of the provider to run.
 *
 * @return The return status of the provider.
 */
Provider::EvaluationResult BIQT::runProvider(const std::string &pName,
                                             const std::string &filePath)
{
    Provider::EvaluationResult result;
    const ProviderInfo *p = getProvider(pName);
    if (p) {
        result = this->runProvider(p, filePath);
    }
    else {
        std::cerr << "Provider '" << pName << "' not found." << std::endl;
    }
    return result;
}

Provider::EvaluationResult BIQT::runProvider(const ProviderInfo *p,
                                             const std::string &filePath)
{
    Provider::EvaluationResult result;
    result.errorCode = 0;
    char* result_str = NULL;
    try {
        result_str = p->evaluate(filePath);
        result = Provider::deserializeResult(result_str);
        result.provider = p->name;
    }
    catch (...) {
        std::cerr << "Abnormal termination, unhandled error type from provider."
                  << std::endl;
        if (result.errorCode == 0)
            result.errorCode = -1;
    }
    if(result_str){
        p->freeResult(result_str);
    }

    if (result.errorCode != 0) {
        std::cerr << "There was an error evaluating " << filePath
                  << " for provider " << p->name << ". ";
        std::cerr << "Error code: " << result.errorCode;
        std::cerr << std::endl;
    }
    return result;
}

/**
 * Runs a provider based on modality.
 *
 * @param modality The modality of the provider to run.
 *
 * @return The return status of the provider.
 */
std::map<std::string, Provider::EvaluationResult>
BIQT::runModality(const std::string &modality, const std::string &filePath)
{
    int providerCount = 0;
    std::map<std::string, Provider::EvaluationResult> results;
    Provider::EvaluationResult result;
    for (const auto provider : getProviders()) {
        if (provider->modality == modality) {
            providerCount++;
            result = runProvider(provider, filePath);
            if (!result.errorCode) {
                results.insert(
                    std::pair<std::string, Provider::EvaluationResult>(
                        provider->name, result));
            }
        }
    }
    if (!providerCount) {
        std::cerr << "No available providers found with the modality '"
                  << modality << "'." << std::endl;
    }
    return results;
}
