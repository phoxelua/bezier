#pragma once

#include "algebra3.h"
#include "BezPatch.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>


void loadScene(std::string filename, vector<BezPatch>& patches, double u, double v) {

	//store variables and set stuff at the end
	int numPatches = 0, curveCount = 0, patchCount = 0;
	BezCurve curves[4]= {};



	std::ifstream inpfile(filename.c_str());
	if(!inpfile.is_open()) {
		std::cout << "Unable to open file" << std::endl;
	} else {
		std::cout << "Filename: " << filename << endl;
		std::string line;

		while(inpfile.good()) {
			std::vector<std::string> splitline;
			std::string buf;

			std::getline(inpfile,line);
			std::stringstream ss(line);

			while (ss >> buf) {
				splitline.push_back(buf);
			}
			//Ignore blank lines
			if(splitline.size() == 0) {
				continue;
			}

			//Ignore comments
			if(splitline[0][0] == '#') {
				continue;
			}

			if(splitline.size() == 1){
				numPatches = atoi(splitline[0].c_str());
				curveCount = 0, patchCount = 0;
				std::cout << "Number of Patches: " << numPatches << endl;
				patches.resize(numPatches);
			}


			if(splitline.size() == 12){
				curves[curveCount] = BezCurve(
					vec3(atof(splitline[0].c_str()), atof(splitline[1].c_str()), atof(splitline[2].c_str())),
					vec3(atof(splitline[3].c_str()), atof(splitline[4].c_str()), atof(splitline[5].c_str())),
					vec3(atof(splitline[6].c_str()), atof(splitline[7].c_str()), atof(splitline[8].c_str())),
					vec3(atof(splitline[9].c_str()),atof(splitline[10].c_str()), atof(splitline[11].c_str())),						u
				);
				curveCount++;
			}

			if (curveCount == 4){
				//cout << patchCount << endl;
				patches[patchCount] = BezPatch(curves[0], curves[1], curves[2],curves[3], u, v);
				curveCount = 0;
				patchCount++;
			}



			//Valid commands:
			//size width height
			//  must be first command of file, controls image size
			/*else if(!splitline[0].compare("size")) {
				width = atoi(splitline[1].c_str());
				height = atoi(splitline[2].c_str());
			}*/
			
			//unknown command
			//else {
			//	std::cerr << "Unknown command: " << splitline[0] << std::endl;
			//}
		}

		inpfile.close();
	}
}