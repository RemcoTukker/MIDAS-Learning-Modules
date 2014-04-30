#
# @file binding.gyp
# @brief This file provides the configuration and building options for node-gyp
#
# This file is created at "Almende". It is open-source software and part of "". 
# This software is published under the  license ().
#
# Copyright © 2014 Remco Tukker <>
#
# @author                    Remco Tukker
# @date                      Apr 28, 2014
# @organisation              Almende
# @project                   
#
{
	"targets": [
		{
			"target_name": "MeanAndVarianceModule",
			
			"include_dirs": [
				"../../inc",
				"../../aim-core/inc"
				
			],
			
			"dependencies":[
			],
			
			"cflags": [
			],
			
			"libraries": [
			],
			
			"ldflags": [
				"-pthread",
			],
			
			"sources":[
				"../../aim-core/src/MeanAndVarianceModule.cpp",
				"MeanAndVarianceModuleNode.cc",
				"../../src/MeanAndVarianceModuleExt.cpp"
			],
		}
	]
}
