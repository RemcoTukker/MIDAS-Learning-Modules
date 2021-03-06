/**
 * @file MeanAndVarianceModuleMain.cpp
 * @brief Entry function for MeanAndVarianceModule
 *
 * This file is created at "Almende". It is open-source software and part of "". 
 * This software is published under the  license ().
 *
 * Copyright © 2014 Remco Tukker <>
 *
 * @author                   Remco Tukker
 * @date                     Apr 28, 2014
 * @organisation             Almende
 * @project                  
 */
 
#include <MeanAndVarianceModuleExt.h>

#include <stdlib.h>
#include <iostream>

using namespace rur;
using namespace std;

/**
 * Every module is a separate binary and hence has its own main method. It is recommended
 * to have a version of your code running without any middleware wrappers, so preferably
 * have this file and the MeanAndVarianceModule header and code in a separate "aim" directory.
 */

int main(int argc, char *argv[])  {
	MeanAndVarianceModuleExt *m = new MeanAndVarianceModuleExt();

	if (argc < 2) {
		std::cout << "Use an identifier as argument for this instance" << endl;
		return EXIT_FAILURE;
	}
	std::string identifier = argv[1];
	m->Init(identifier);

	do {
		m->Tick();
	} while (!m->Stop());

	delete m;

	return EXIT_SUCCESS;
}
