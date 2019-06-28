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
                    "Provider Read error: Unable to load the shared object");
        }
    }

    std::string desc_path =
        modulePath + "/providers/" + lib + "/descriptor.json";
    std::ifstream desc_file(desc_path.c_str(), std::ifstream::binary);
    desc_file >> desc;

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
        this->eval = (evaluator)dlsym(this->handle, "provider_eval");
        if (!this->eval) {
            throw std::runtime_error("Provider API Error:"
                                 "Unable to locate the provider_eval function");
        }
#ifdef BIQT_JAVA_SUPPORT
    }
#endif
}

#ifdef BIQT_JAVA_SUPPORT
std::string ProviderInfo::getClassPath(std::string providerPath)
{
    struct dirent *module;
    std::set<std::string> paths;
    std::string classPath = "";
    int extension;
    DIR *dir;
    std::string biqt_home = getenv("BIQT_HOME");

    if (!BIQT::fileExists(providerPath)) {
        return "";
    }
    /* find the BIQT jar files */
    dir = opendir(biqt_home.c_str());
    while ((module = readdir(dir)) != nullptr) {
        extension = strlen(module->d_name) - 4;
        if (!strcmp(module->d_name + extension, ".jar")) {
            paths.insert(biqt_home + DIRSEP + module->d_name);
        }
    }
    closedir(dir);
    /* find the jar files for the provider */
    dir = opendir(providerPath.c_str());
    while ((module = readdir(dir)) != nullptr) {
        extension = strlen(module->d_name) - 4;
        if (!strcmp(module->d_name + extension, ".jar")) {
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

const char *ProviderInfo::evaluate(std::string filename) const
{
#ifdef BIQT_JAVA_SUPPORT
    const char *returnvalue;
    if (this->sourceLanguage == "java") {
        returnvalue = java_provider_eval(filename.c_str(), this->name.c_str(),
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
    else {
        /* BIQT_HOME is not set or providers directory missing,
         * try the current directory */
        std::cerr << "WARNING: BIQT_HOME is not set; using the current "
            "directory instead. This may not be what you want!" << std::endl;
        setenv("BIQT_HOME", ".", 1);
        this->modulePath = std::string(".");
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
    if (stat(filename.c_str(), &info))
        return false;
#ifdef _WIN32
    return (bool)(info.st_mode);
#else
    return (bool)(info.st_mode & (S_IXOTH | S_IROTH));
#endif
}

std::set<std::string> BIQT::providerLibs()
{
    std::set<std::string> returnValue;
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
    Provider::EvaluationResult result = {0};
    try {
        // const char* cFilePath = filePath.c_str();
        std::string result_str = p->evaluate(filePath);
        result = Provider::deserializeResult(result_str.c_str());
        result.provider = p->name;
    }
    catch (...) {
        std::cerr << "Abnormal termination, unhandled error type from provider."
                  << std::endl;
        if (result.errorCode == 0)
            result.errorCode = -1;
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
