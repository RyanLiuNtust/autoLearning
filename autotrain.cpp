/**
* @file autoTrain.cpp
* @version $0.1$
* @author Po-Hsien Liu <pohsienliu2012@gmail.com>
* @note Copyright (c) 2014/5/17., all rights reserved.
* @ Prerequisites
* @ Boost 1.55 (preferred) http://www.boost.org/
* @ OpenCV http://opencv.org/
* --------------------------------------------------
* @ Description: 
* The main purpose of code is to make training easier.
* It will automatically draw the leanring curve
* and find the optimal solution at the end

* e.g. 
*/

#include <Windows.h>
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <opencv.hpp>
#include "../../../include/include/FileSystem.h"

void createInfoList(std::string infoname, std::vector<std::string> imgList, bool isPos = true) {
	std::cout << "creating info list.....\n";
	std::fstream fs(infoname, std::ios::out);
	for(std::vector<std::string>::iterator it = imgList.begin(); it != imgList.end(); it++) {
		cv::Mat src = cv::imread(*it);
		int numObject = 1;
		int w = src.cols;
		int h = src.rows;
		if(isPos)
			fs << *it << " " << numObject << " 0 0 " << w << " " << h << std::endl;
		else 
			fs << *it << std::endl;
	}
	fs.close();
}

bool executeCommandLine(std::vector<std::string> cmdLine) {
	for(std::vector<std::string>::iterator it = cmdLine.begin(); it != cmdLine.end(); it++) {
		STARTUPINFO startupinfo;
		PROCESS_INFORMATION processinfo;

		ZeroMemory( &startupinfo, sizeof(startupinfo) );
		startupinfo.cb = sizeof(startupinfo);
		ZeroMemory( &processinfo, sizeof(processinfo) );
		if( !CreateProcess(NULL,   // No module name (use command line)
			const_cast<char *>(it->c_str()),        // Command line
			NULL,					// Process handle not inheritable
			NULL,					// Thread handle not inheritable
			FALSE,					// Set handle inheritance to FALSE
			CREATE_NEW_CONSOLE,		// No creation flags
			NULL,					// Use parent's environment block
			NULL,					// Use parent's starting directory 
			&startupinfo,           // Pointer to STARTUPINFO structure
			&processinfo )          // Pointer to PROCESS_INFORMATION structure
		  ) {
			std::cout <<  "CreateProcess failed (" << GetLastError()  << ").\n";
			system("pause");
			return 0;
		}
	}
}

cv::Mat rotate(cv::Mat src, double angle, double scale = 1) {
	double radius = angle * CV_PI/180;
	double alpha = cos(radius) * scale;
	double beta = sin(radius) * scale;
	int nCols = static_cast<int>(src.cols * fabs(alpha) + src.rows * fabs(beta));
	int nRows = static_cast<int>(src.cols * fabs(beta) + src.rows * fabs(alpha));

    cv::Point2f center(src.cols/2., src.rows/2.);
    cv::Mat m = cv::getRotationMatrix2D(center, angle, 1.0);
	m.ptr<double>(0)[2] +=  static_cast<int>((nCols - src.cols)/2);
	m.ptr<double>(1)[2] +=	static_cast<int>((nRows - src.rows)/2);
	
	cv::Mat dst;
    warpAffine(src, dst, m, cv::Size(nCols, nRows));
    return dst;
}

void synthesisData(std::vector<std::string>posImgs, std::string synDir, int synNum) {

}

int main(int argc, char *argv[])
{
	if(argc == 1) {
	std::cout << "Welcome to automatically adaboost learning system\n"
			  << "Usuage: auto_adaboost.exe\n"
			  << "[-pos <postive_collection_directory>]\n"
			  << "[-neg <negative_collection_directory>]\n"
			  << "[-vec <output_vec_directory>]\n"
			  << "[-w <sample_width = 24>]\n[-h <sample_height = 24>]\n"
			  << "----------synthesis paramters----------\n"
			  << "[-rot <synthesis_rotation>]\n"
			  << "[-syn <synthesis_number = 2>]\n"
			  << "[-synDir <syntehsis_data_collection_directory>\n]"
			  << "----------optimal paramters----------\n"
			  << "[-opt <optimize_result>]\n"
			  << "[-num <number_of_divided_training = 4>]";
			  
	}

	bool opt = false;
	bool rot = false;
	int w = 24;
	int h = 24;
	int synNum = 2;
	int num = 4;
	std::string vecDir;
	std::string posColDir;
	std::string negColDir;
	std::string synDir = "./synData";

	for(int i = 1; i < argc; i++) {
		if(!strcmp(argv[i], "-pos")) {
			posColDir = argv[++i];
		}
		if(!strcmp(argv[i], "-neg")) {
			negColDir = argv[++i];
		}
		if(!strcmp(argv[i], "-vec")) {
			vecDir = argv[++i];
		}
		if(!strcmp(argv[i], "-rot")) {
			rot = true;
		}
		if(!strcmp(argv[i], "-syn")) {
			synNum = atoi(argv[++i]);
		}
		if(!strcmp(argv[i], "synDir")) {
			synDir = argv[++i];
		}
		if(!strcmp(argv[i], "-opt")) {
			opt = true;
		}
		if(!strcmp(argv[i], "-num")) {
			num = atoi(argv[++i]);
		}
		if(!strcmp(argv[i], "-w")) {
			w = atoi(argv[++i]);
		}
		if(!strcmp(argv[i], "-h")) {
			h = atoi(argv[++i]);
		}
	}
	FileSystem filesystem;
	std::vector<std::string>posImgs = filesystem.getFileList(posColDir, ALL);
	int numPos = posImgs.size();

	FileSystem negFilesystem;
	std::vector<std::string>negImgs = negFilesystem.getFileList(negColDir, ALL);
	int numNeg = negImgs.size();
	
	if(numPos == 0) {
		std::cout << "Do not exist any training samples, pls check whether the path exist\n";
		system("pause");
		return 0;
	}
	else if(numPos <= 650) {
		std::cout << "The training samples is not enough to train the good detector\n"
				  << "Please try to get more training data to improve the performance\n";
		system("pause");
		return 0;
	}

	std::string posInfoname = "pos.txt";
	std::string vecname = vecDir + "/test.vec";
	createInfoList(posInfoname, posImgs);

	std::string negInfoname = "neg.txt";
	createInfoList(negInfoname, negImgs, false);

	std::ostringstream createsamplesCmd;
	createsamplesCmd << "opencv_createsamples.exe -info " << posInfoname << " -vec "
					 << vecname << " -w " << w << " -h " << h << " -num " << numPos;
	system(createsamplesCmd.str().c_str());

	std::vector<std::string> cmdLine(4);
	cmdLine[0] = "opencv_traincascade.exe -data cascade/1 -vec vec/test.vec -bg neg.txt -numPos 1000 -numNeg 1000 -w 30 -h 14";
	cmdLine[1] = "opencv_traincascade.exe -data cascade/2 -vec vec/test.vec -bg neg.txt -numPos 1500 -numNeg 1000 -w 30 -h 14";
	executeCommandLine(cmdLine);
	system("pause");
}