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
#include <json/json.h>

#include "BIQT.h"

#ifndef _MSC_VER /* Check for microsoft compiler */
#include <getopt.h>
#include <libgen.h>
#else
#include "windows/getopt.h"
#include "windows/libgen.h"
#define setenv(name, value, opt) _putenv_s(name, value)
#endif

void usage()
{
    std::cout << "SYNOPSIS\n"
                 "  biqt [OPTIONS | COMMAND file]\n\n"
                 "OPTIONS\n"
                 "  --help\n"
                 "    Prints this usage information.\n\n"
                 "  -P MODALITY | --providers[=MODALITY]\n"
                 "    Lists the Provider plugins known to this BIQT "
                 "installation. If `MODALITY` is provided, only Providers "
                 "matching that type are returned.\n\n"
                 "  -V|--version\n"
                 "    Prints the version information associated with BIQT and "
                 "the CLI. For simplicity, assume that the CLI and the BIQT "
                 "framework are always released together as a single "
                 "package.\n\n"
                 "COMMANDS\n"
                 "  -m MODALITY|--modality=MODALITY\n"
                 "    Evaluates the given file using all of the providers "
                 "associated with the given modality.\n\n"
                 "  -p PROVIDER|--provider=PROVIDER\n"
                 "    Evaluates the given file using a specific provider.\n\n"
                 "INPUT BEHAVIORS\n"
                 "  -l|--file-list\n"
                 "    Indicates that the file path contains a "
                 "newline-separated list of input file paths (relative the "
                 "working directory). If this is not provided, it is assumed "
                 "that file should be parsed as-is.\n"
                 "OUTPUT BEHAVIORS\n"
                 "  -f (json|text)|--output-format=(json|text)\n"
                 "    Controls how output is returned to the user. By default, "
                 "text is used.\n"
              << std::endl;
}

void write_text(std::ostream &outputStream, const std::string &imageName,
                const Provider::EvaluationResult &result)
{
    // Write header
    std::string delim = ",";
    outputStream << "Provider" << delim << "Image" << delim << "Detection"
                 << delim << "AttributeType" << delim << "Key" << delim
                 << "Value" << std::endl;

    // Loop through every detection
    int d = 1;
    for (const auto &qualityResult : result.qualityResult) {
        // Metric map
        for (const auto &metric : qualityResult.metrics) {
            outputStream << result.provider << delim << imageName << delim << d
                         << delim << "Metric" << delim << metric.first << delim
                         << metric.second << std::endl;
        }
        // Feature map
        for (const auto &feature : qualityResult.features) {
            outputStream << result.provider << delim << imageName << delim << d
                         << delim << "Feature" << delim << feature.first
                         << delim << feature.second << std::endl;
        }
        d = d + 1;
    }
}

void write_text2(std::ostream &outputStream, const std::string &imageName,
                 const Provider::EvaluationResult &result)
{
    std::string delim = ",";
    // Loop through every detection
    int d = 1;
    for (const auto &qualityResult : result.qualityResult) {
        // Metric map
        for (const auto &metric : qualityResult.metrics) {
            outputStream << result.provider << delim << imageName << delim << d
                         << delim << "Metric" << delim << metric.first << delim
                         << metric.second << std::endl;
        }
        // Feature map
        for (const auto &feature : qualityResult.features) {
            outputStream << result.provider << delim << imageName << delim << d
                         << delim << "Feature" << delim << feature.first
                         << delim << feature.second << std::endl;
        }
        d = d + 1;
    }
}

void to_text(const std::string &imageName,
             const Provider::EvaluationResult &result,
             const std::string &outputPath)
{
    // Create the file if it does not exist
    if (outputPath != "-") {
        std::ofstream outputStream(outputPath, std::ofstream::app);
        write_text(outputStream, imageName, result);
        outputStream.close();
    }
    else {
        write_text(std::cout, imageName, result);
    }
}

void to_text2(const std::string &imageName,
              const std::map<std::string, Provider::EvaluationResult> &results,
              const std::string &outputPath)
{
    std::string delim = ",";
    // Create the file if it does not exist
    if (outputPath != "-") {
        std::ofstream outputStream(outputPath, std::ofstream::app);
        outputStream << "Provider" << delim << "Image" << delim << "Detection"
                     << delim << "AttributeType" << delim << "Key" << delim
                     << "Value" << std::endl;
        for (const auto &kv : results) {
            write_text2(outputStream, imageName, kv.second);
        }
        outputStream.close();
    }
    else {
        std::cout << "Provider" << delim << "Image" << delim << "Detection"
                  << delim << "AttributeType" << delim << "Key" << delim
                  << "Value" << std::endl;
        for (const auto &kv : results) {
            write_text2(std::cout, imageName, kv.second);
        }
    }
}

int to_json(const std::string &imageName,
            const Provider::EvaluationResult &result,
            const std::string &outputPath)
{
    Json::Value jsonResult;
    Json::Value vec(Json::arrayValue);

    // Read from module_jsonResult to Json object jsonResult
    for (const auto &qualityResult : result.qualityResult) {
        Json::Value dResult;

        for (const auto &metric : qualityResult.metrics) {
            dResult["metrics"][metric.first] = metric.second;
        }

        for (const auto &feature : qualityResult.features) {
            dResult["features"][feature.first] = feature.second;
        }

        vec.append(std::move(dResult));
    }

    jsonResult[imageName][result.provider] = std::move(vec);

    // Write Json object to file
    if (outputPath != "-") {
        std::ofstream outputStream(outputPath, std::ofstream::app);
        outputStream << jsonResult << std::endl;
        outputStream.close();
    }
    else {
        std::cout << jsonResult << std::endl;
    }
    return 0;
}

int to_json2(const std::string &imageName,
             const std::map<std::string, Provider::EvaluationResult> &results,
             const std::string &outputPath)
{
    Json::Value jsonResult;
    for (const auto &kv : results) {
        Provider::EvaluationResult result = kv.second;
        Json::Value vec(Json::arrayValue);

        // Read from module_jsonResult to Json object jsonResult
        for (const auto &qualityResult : result.qualityResult) {
            Json::Value dResult;

            for (const auto &metric : qualityResult.metrics) {
                dResult["metrics"][metric.first] = metric.second;
            }

            for (const auto &feature : qualityResult.features) {
                dResult["features"][feature.first] = feature.second;
            }

            vec.append(std::move(dResult));
        }

        jsonResult[imageName][result.provider] = std::move(vec);
    }

    // Write Json object to file
    if (outputPath != "-") {
        std::ofstream outputStream(outputPath, std::ofstream::app);
        outputStream << jsonResult << std::endl;
        outputStream.close();
    }
    else {
        std::cout << jsonResult << std::endl;
    }
    return 0;
}

int run_provider(BIQT &app, bool modality, const std::string &inputFile,
                 const std::string &outputFile, const std::string &mod_arg,
                 const std::string &output_type)
{
    if (modality) {
        // Iterate through map
        std::map<std::string, Provider::EvaluationResult> results =
            app.runModality(mod_arg, inputFile);

        for (const auto &kv : results) {
            std::string provider = kv.first;
            Provider::EvaluationResult result = kv.second;

            // Evaluate individual result
            if (result.errorCode) {
                return result.errorCode;
            }
        }

        if (results.size()) {
            if (output_type == "json")
                to_json2(inputFile, results, outputFile);
            else
                to_text2(inputFile, results, outputFile);
        }
    }

    else {
        Provider::EvaluationResult result = app.runProvider(mod_arg, inputFile);
        if (result.errorCode) {
            return result.errorCode;
        }
        if (output_type == "json")
            to_json(inputFile, result, outputFile);
        else
            to_text(inputFile, result, outputFile);
    }

    return 0;
}

int main(int argc, char **argv)
{
    std::string output_type = "text";
    std::string outputFile = "-";
    std::string inputFile;
    std::string mod_arg;
    bool found_matching_providers = false;
    bool modality_flag = false;
    bool provider_flag = false;
    bool file_list_flag = false;

    BIQT app;

    while (true) {
        static struct option long_options[] = {
            {"help", no_argument, 0, 'h'},
            {"version", no_argument, 0, 'V'},
            {"providers", optional_argument, 0, 'P'},
            {"modality", required_argument, 0, 'm'},
            {"provider", required_argument, 0, 'p'},
            {"output-format", required_argument, 0, 'f'},
            {"file-list", no_argument, 0, 'l'},
            {0, 0, 0, 0}};

        int option_index = 0;
        int c = getopt_long(argc, argv, "f:hlVP::m:p:", long_options,
                            &option_index);

        if (argc == 1) {
            std::cout << "Please provide input arguments. Run 'biqt --help' to "
                         "see correct usage."
                      << std::endl;
            exit(0);
        }

        if (c == -1)
            break;

        switch (c) {
        case 0: {
            break;
        }
        case 'h': {
            usage();
            exit(0);
        }
        case 'V': {
            std::cout << "BIQT v" << __BIQT_VERSION__ << std::endl; // TODO:
            exit(0);
        }
        case 'f': {
            output_type = optarg;
            break;
        }
        case 'o': {
            outputFile = optarg;
            break;
        }
        case 'l': {
            file_list_flag = true;
            break;
        }
        case 'P': {
            // Correct for optarg if space used
            // https://linux.die.net/man/1/getopt
            if (optarg == NULL && argv[optind] != NULL) {
              optarg = argv[optind];
            }

            // Error for no installed providers
            if (app.getProviders().empty()) {
              std::cerr << "No provider libraries were found." << std::endl;
              exit(0);
            }

            // Error for no installed providers matching specified modality
            for (const auto &p : app.getProviders()) {
                if (!optarg || p->modality == optarg) {
                  found_matching_providers = true;
                }
            }
            if (!found_matching_providers) {
              std::cerr << "No provider libraries matching the provided modality were found." << std::endl;
              exit(0);
            }

            // Normal behavior, print results
            if (!app.getProviders().empty()) {
                std::cout << std::endl
                          << "Provider\t\tVersion\tModality\tDescription"
                          << std::endl;
                for (const auto &p : app.getProviders()) {
                    if (!optarg || p->modality == optarg) {
                        std::cout << p->name << "\t\t" << p->version << "\t"
                                  << p->modality << "\t" << p->description << "\t"
                                  << std::endl;
                    }
                }
                std::cout << std::endl;
            }
            // No providers found!
            else {
                std::cerr << "No providers found in '" << app.modulePath << "'!"
                          << std::endl;
            }
            exit(0);
        }
        case 'm': {
            if (optind >= argc) {
                std::cout << "Please input a valid modality, followed by your "
                             "desired input image/directory."
                          << std::endl;
                break;
            }
            mod_arg = optarg;
            modality_flag = true;
            break;
        }
        case 'p': {
            if (optind >= argc) {
                std::cout << "Please input a valid provider, followed by your "
                             "desired input image/directory."
                          << std::endl;
                break;
            }
            mod_arg = optarg;
            provider_flag = true;
            break;
        }
        }
    }

    inputFile = argv[argc - 1];

    if (!modality_flag && !provider_flag) {
        std::cout << "Invalid arguments detected. Run 'biqt --help' to see "
                     "correct usage."
                  << std::endl;
        return -1;
    }

    if (file_list_flag) {
        std::ifstream fileList(inputFile);
        std::string imageFile;
        while (getline(fileList, imageFile)) {
            run_provider(app, modality_flag, imageFile, outputFile, mod_arg,
                         output_type);
        }
    }
    else {
        run_provider(app, modality_flag, inputFile, outputFile, mod_arg,
                     output_type);
    }
    return 0;
}
