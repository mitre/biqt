// #######################################################################
// NOTICE
//
// This software (or technical data) was produced for the U.S. Government
// under contract, and is subject to the Rights in Data-General Clause
// 52.227-14, Alt. IV (DEC 2007).
//
// Copyright 2019 The MITRE Corporation. All Rights Reserved.
// #######################################################################

#include <NewProvider.h>
#include <fstream>
#include <map>

NewProvider::NewProvider()
{
    // Initialize metadata
    std::string biqt_home = getenv("BIQT_HOME");
    std::ifstream desc_file(biqt_home +
                                "/providers/NewProvider/descriptor.json",
                            std::ifstream::binary);
    desc_file >> DescriptorObject;

    // TODO: Instantiate any additional objects needed for your algorithm
}

NewProvider::~NewProvider()
{
    // TODO: If needed, populate the destructor
}

Provider::EvaluationResult NewProvider::evaluate(const std::string &file)
{
    // Initialize some variables
    Provider::EvaluationResult evalResult;
    Provider::QualityResult qualityResult;

    // TODO: Read the input image 'file' (string indicating file path to image)
    
	// TODO: Populate metrics and features maps.	
	qualityResult.metrics["quality"] = 555.55;
	qualityResult.features["left_eye_x"] = 1;
	
	evalResult.qualityResult.push_back(std::move(qualityResult));
    return evalResult;
}

DLL_EXPORT const char *provider_eval(const char *cFilePath)
{
    NewProvider p;
    std::string filePath(cFilePath);
    Provider::EvaluationResult result = p.evaluate(filePath);
    return Provider::serializeResult(result);
}
