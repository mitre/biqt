# #######################################################################
# NOTICE
#
# This software (or technical data) was produced for the U.S. Government
# under contract, and is subject to the Rights in Data-General Clause
# 52.227-14, Alt. IV (DEC 2007).
#
# Copyright 2019 The MITRE Corporation. All Rights Reserved.
# ####################################################################### 

import os
import re
import argparse
import sys

if not os.environ['BIQT_HOME']:
  sys.exit("Error: The BIQT_HOME environment variable is not set. This script requires BIQT_HOME to be assigned to the installation directory of BIQT.")

def instantiate_template(pname, template_name, out_file_name):
	with open(os.environ['BIQT_HOME'] + '/scripts/templates/' + template_name, 'r') as template_file, open(pname + '/' + out_file_name, 'w') as provider_file:
		template_data = template_file.read()
		provider_data = template_data.replace('NewProvider', pname)
		provider_data = provider_data.replace('NEWPROVIDER_H', pname.upper() + '_H')
		provider_file.write(provider_data)

def main():
	# Parse command line arguments
	parser = argparse.ArgumentParser()
	parser.add_argument('pname', type=str, help='Name of new provider algorithm.')
	args = parser.parse_args()

	# Is pname ok? Can only contain alphanumeric characters (A-Z, a-z, 0-9) or underscore (_) or dash(-)
	if not (re.match(r'^[A-Za-z0-9_-]+$', args.pname)):
		sys.exit("ERROR: The provider name you entered was not valid. The provider name should only contain alphanumeric characters, dashes, and underscores.")

	# Setup the provider directory structure
	prodir = args.pname + "/"
	for d in [prodir, prodir+'src', prodir+'config']:
		if not os.path.isdir(d):
			os.mkdir(d)

	# Create provider interface files
	if not os.path.exists(prodir + args.pname + '.h'):
		instantiate_template(args.pname, 'Provider.h', args.pname + '.h')
	if not os.path.exists(prodir + args.pname + '.cpp'):
		instantiate_template(args.pname, 'Provider.cpp', args.pname + '.cpp')

	# Create descriptor file
	if not os.path.exists(prodir + 'descriptor.json'):
		instantiate_template(args.pname, 'descriptor.json', 'descriptor.json')

	# Create CMakeLists.txt
	if not os.path.exists(prodir + 'CMakeLists.txt'):
		instantiate_template(args.pname, 'CMakeLists.txt', 'CMakeLists.txt')

if __name__ == '__main__':
    main()
