// #######################################################################
// NOTICE
//
// This software (or technical data) was produced for the U.S. Government
// under contract, and is subject to the Rights in Data-General Clause
// 52.227-14, Alt. IV (DEC 2007).
//
// Copyright 2019 The MITRE Corporation. All Rights Reserved.
// #######################################################################

#ifndef NEWPROVIDER_H
#define NEWPROVIDER_H

#include <ProviderInterface.h>
#include <fstream>
#include <json/json.h>
#include <json/value.h>
// TODO: Include any additional header files

class NewProvider : public Provider {

  public:
    NewProvider();
	~NewProvider() override;
    Provider::EvaluationResult evaluate(const std::string &file) override;
};

#endif
