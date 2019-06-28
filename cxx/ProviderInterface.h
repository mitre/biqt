// #######################################################################
// NOTICE
//
// This software (or technical data) was produced for the U.S. Government
// under contract, and is subject to the Rights in Data-General Clause
// 52.227-14, Alt. IV (DEC 2007).
//
// Copyright 2019 The MITRE Corporation. All Rights Reserved.
// #######################################################################

#ifndef PROVIDERINTERFACE_H
#define PROVIDERINTERFACE_H

#include <cstring>
#include <fstream>
#include <iostream>
#include <json/json.h>
#include <json/value.h>
#include <json/writer.h>
#include <map>
#include <vector>

#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

/**
 * A class for implementing a provider.
 */
class Provider {

  public:
    struct QualityResult {
        std::map<std::string, double>
            metrics; /* A map containing the quality metrics */
        std::map<std::string, double> features; /* A map containing detection
                                                   features (EyePosition,
                                                   FaceHeight, etc.) */
    };

    struct EvaluationResult {
        int errorCode;        /* Non-zero values indicate errors */
        std::string provider; /* The provider name */
        std::string
            message; /* Used to bubble up messages from the provider to the
                        application */
        std::vector<QualityResult>
            qualityResult; /* A vector of QualityResults, in the case of
                              multiple detections. */
    };

    virtual ~Provider() = default;

    // Provider metadata
    /**
     * Returns the name of this provider.
     *
     * @return the name of the provider.
     */
    virtual std::string name() { return DescriptorObject["name"].asString(); }

    /**
     * Returns the version number of this provider.
     *
     * @return the provider version number.
     */
    virtual std::string version()
    {
        return DescriptorObject["version"].asString();
    }

    /**
     * Returns a description of this provider.
     *
     * @return the provider description.
     */
    virtual std::string description()
    {
        return DescriptorObject["description"].asString();
    }

    /**
     * Returns the modality of this provider.
     *
     * @return the provider modality
     */
    virtual std::string modality()
    {
        return DescriptorObject["modality"].asString();
    }

    /**
     * Returns a vector of provider attributes.
     *
     * @return the provider attributes.
     */
    virtual std::vector<std::string> attributes()
    {
        Json::Value attrs = DescriptorObject["attributes"];
        std::vector<std::string> attr_list;
        for (const auto &attr : attrs) {
            attr_list.push_back(attr["name"].asString());
        }
        return attr_list;
    }

    /**
     *
     * @param attribute The description of a particular attribute of the
     * provider
     *
     * @return the attribute description.
     */
    virtual std::string describeAttribute(const std::string &attribute)
    {
        Json::Value attrs = DescriptorObject["attributes"];

        for (const auto &attr : attrs) {
            if (attr["name"].asString() == attribute) {
                return attr["description"].asString();
            }
        }

        return "This module does not support the specified attribute.";
    }

    /* Provider functions */

    /**
     * Runs the provider to evaluate the given file.
     *
     * @param file The input file to evaluate.
     *
     * @return The result of the evaluation.
     */
    virtual EvaluationResult evaluate(const std::string &file) = 0;

    /**
     * Deserializes a JSON char array to populate an EvaluationResult struct
     *
     * @param result_str The JSON string
     *
     * @return The deserialized EvaluationResult
     */
    static EvaluationResult deserializeResult(const char *result_str)
    {
        EvaluationResult result;
        // String to JSON
        Json::Value result_json;
        Json::Reader reader;
        bool parsingSuccessful = reader.parse(result_str, result_json);
        if (!parsingSuccessful) {
            std::cout << "Failed to parse"
                      << reader.getFormattedErrorMessages();
        }

        // JSON to Evaluation::Result
        result.errorCode = result_json["errorCode"].asInt();
        result.provider = result_json["provider"].asString();
        result.message = result_json["message"].asString();

        // Vector of QualityResults json
        for (const auto &qualityResult : result_json["qualityResult"]) {
            Provider::QualityResult quality_result;
            for (const auto &metric :
                 qualityResult["metrics"].getMemberNames()) {
                quality_result.metrics[metric] =
                    qualityResult["metrics"][metric].asFloat();
            }
            for (const auto &feature :
                 qualityResult["features"].getMemberNames()) {
                quality_result.features[feature] =
                    qualityResult["features"][feature].asFloat();
            }

            result.qualityResult.push_back(std::move(quality_result));
        }
        return result;
    }

    /**
     * Serializes an EvaluationResult struct into a JSON char array. delete[]
     * should be called on the return value to avoid memory leaks
     *
     * @param result_str The EvaluationResult struct
     *
     * @return The serialized JSON string
     */
    static char *serializeResult(const Provider::EvaluationResult &result)
    {
        // Write result to JSON format
        Json::Value result_json;
        result_json["errorCode"] = result.errorCode;
        result_json["provider"] = result.provider;
        result_json["message"] = result.message;

        // Vector of QualityResults
        Json::Value vec(Json::arrayValue);
        for (const auto &qualityResult : result.qualityResult) {
            Json::Value quality_result;
            Json::Value metrics_map;
            Json::Value features_map;

            for (const auto &metric : qualityResult.metrics) {
                metrics_map[metric.first] = metric.second;
            }

            for (const auto &feature : qualityResult.features) {
                features_map[feature.first] = feature.second;
            }

            quality_result["metrics"] = std::move(metrics_map);
            quality_result["features"] = std::move(features_map);
            vec.append(std::move(quality_result));
        }
        result_json["qualityResult"] = std::move(vec);

        // Return as string
        std::string result_str = result_json.toStyledString();
        char *result_cstr = new char[result_str.length() + 1];
        strncpy(result_cstr, result_str.c_str(), result_str.length() + 1);
        return result_cstr;
    }

  protected:
    // Descriptor object
    Json::Value DescriptorObject;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A single function to begin provider analysis.
 *
 * @param filePath The path to the input file.
 *
 * @return The return status of the provider.
 */
DLL_EXPORT const char *provider_eval(const char *filePath);

#ifdef __cplusplus
}
#endif

#endif
