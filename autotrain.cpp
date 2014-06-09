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
#include <lexical_cast.hpp>
#include "FileSystem.h"

struct ProcessWindowsInfo
{
    DWORD ProcessID;
    std::vector<HWND> Windows;

    ProcessWindowsInfo( DWORD const AProcessID )
        : ProcessID( AProcessID )
    {
    }
};

BOOL __stdcall EnumProcessWindowsProc( HWND hwnd, LPARAM lParam )
{
    ProcessWindowsInfo *Info = reinterpret_cast<ProcessWindowsInfo*>( lParam );
    DWORD WindowProcessID;

    GetWindowThreadProcessId( hwnd, &WindowProcessID );

    if( WindowProcessID == Info->ProcessID )
        Info->Windows.push_back( hwnd );

    return true;
}

void createInfoList(std::string infoname, std::vector<std::string> imgList, bool isPos = true) {
	std::cout << "creating info text.....\n";
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
		PROCESS_INFORMATION pi;
		ZeroMemory( &startupinfo, sizeof(startupinfo) );
		startupinfo.cb = sizeof(startupinfo);
		ZeroMemory( &pi, sizeof(pi) );
		
		if( !CreateProcess(NULL,   // No module name (use command line)
			const_cast<char *>(it->c_str()),        // Command line
			NULL,					// Process handle not inheritable
			NULL,					// Thread handle not inheritable
			FALSE,					// Set handle inheritance to FALSE
			CREATE_NEW_CONSOLE,		// No creation flags
			NULL,					// Use parent's environment block
			NULL,					// Use parent's starting directory 
			&startupinfo,           // Pointer to STARTUPINFO structure
			&pi )					// Pointer to PROCESS_INFORMATION structure
		  ) {
			std::cout <<  "CreateProcess failed (" << GetLastError()  << ").\n";
			system("pause");
			return 0;
		}
		//WaitForInputIdle(pi.hProcess, INFINITE);
		//ProcessWindowsInfo Info( GetProcessId( pi.hProcess ) );
		
		//EnumWindows( (WNDENUMPROC)EnumProcessWindowsProc,
		//			 reinterpret_cast<LPARAM>( &Info ) );
		//std::cout << "num of process = " << Info.Windows.size() << std::endl;
		//SetWindowText(Info.Windows
	}
}

static cv::Scalar randomColor( cv::RNG& rng )
{
	int icolor = (unsigned) rng;
	return cv::Scalar( icolor&255, (icolor>>8)&255, (icolor>>16)&255 );
}

cv::Mat rotate(cv::Mat src, double angle, double scale = 1, bool isWidthMean = false) {
	double radius = angle * CV_PI/180;
	double alpha = cos(radius) * scale;
	double beta = sin(radius) * scale;
	int nCols = static_cast<int>(src.cols * fabs(alpha) + src.rows * fabs(beta));
	int nRows = static_cast<int>(src.cols * fabs(beta) + src.rows * fabs(alpha));
    cv::Point2f center(src.cols/2., src.rows/2.);
    cv::Mat m = cv::getRotationMatrix2D(center, angle, 1.0);
	m.ptr<double>(0)[2] +=  static_cast<int>((nCols - src.cols)/2);
	m.ptr<double>(1)[2] +=	static_cast<int>((nRows - src.rows)/2);
	
	cv::Scalar bg(0, 0, 0);
	if(isWidthMean) {
		 bg = cv::mean(src);
	}
	cv::Mat dst;
    warpAffine(src, dst, m, cv::Size(nCols, nRows), 1, 0, bg);
    return dst;
}

void synthesisData(std::vector<std::string>posImgs, std::string synDir, int &syn_w, int &syn_h, double syn_angle = 0, double scale = 1) {
	FileSystem filesystem;
	filesystem.createFolder(synDir);
	int index = 0;
	
	for(std::vector<std::string>::iterator it = posImgs.begin(); it != posImgs.end(); it++) {
		std::string filename = synDir + boost::lexical_cast<std::string>(index++) + ".bmp";
		cv::Mat src = cv::imread(*it);
		cv::Mat rot = rotate(src, syn_angle, scale, true);
		syn_w = rot.cols;
		syn_h = rot.rows;
		cv::imwrite(filename, rot);
	}
}

void train(std::string vecname, int numPos, int numNeg, int numDivTrain, int numStages, int w, int h) {
	std::vector<std::string> cmdLine(numDivTrain);
	int numIntervalPos = (numPos-150)/numDivTrain;
	FileSystem filesystem;
	
	for(int i = 0; i < numDivTrain; i++) {
		std::string cascadeFolder = "cascade/" + boost::lexical_cast<std::string>(w) + "_" +
									 boost::lexical_cast<std::string>(h) + "_" +
									 boost::lexical_cast<std::string>((i+1)*numIntervalPos);
		filesystem.createFolder(cascadeFolder);
		cmdLine[i] = "opencv_traincascade.exe -bg neg.txt -minHitRate 0.9965 -maxFalseAlarmRate 0.44 -precalcValBufSize 450 -precalcIdxBufSize 200 -vec " + vecname + 
					 " -data " + cascadeFolder +
					 " -numPos " + boost::lexical_cast<std::string> ((i+1)*numIntervalPos) +
					 " -numNeg " + boost::lexical_cast<std::string> (numNeg) +
					 " -numStages " + boost::lexical_cast<std::string> (numStages) +
					 " -w " + boost::lexical_cast<std::string>(w) +
					 " -h " + boost::lexical_cast<std::string>(h);
		std::cout << cmdLine[i] << std::endl;
	}
	executeCommandLine(cmdLine);
}

int main(int argc, char *argv[])
{
	if(argc < 3) {
	std::cout << "Welcome to automatically adaboost learning system !!\n"
			  << "Usuage: auto_adaboost.exe\n"
			  << "[-pos <postive_collection_directory>]\n"
			  << "[-neg <negative_collection_directory>]\n"
			  << "[-vec <vec_file_name>]\n"
			  << "[-numStage <number_of_stages=15>]\n"
			  << "[-w <sample_width = 24>]\n[-h <sample_height = 24>]\n"
			  << "----------synthesis paramters----------\n"
			  << "[-rot <synthesis_rotation_angle>]\n"
			  << "[-syn <is_synthesis_number = false>]\n"
			  << "[-synDir <syntehsis_data_collection_directory>\n]"
			  << "----------optimal paramters----------\n"
			  << "[-opt <optimize_result>]\n"
			  << "[-num <number_of_divided_training = 4>]\n";
			  
	}
	std::string vecFilename;
	std::string posColDir;
	std::string negColDir;
	std::string synDir = ".\\syn\\";
	
	double rot = 0;
	int w = 24;
	int h = 24;
	int numDivTrain = 4;
	int numStages = 13;
	bool isSyn = false;
	bool isOpt = false;

	for(int i = 1; i < argc; i++) {
		if(!strcmp(argv[i], "-pos")) {
			posColDir = argv[++i];
		}
		if(!strcmp(argv[i], "-neg")) {
			negColDir = argv[++i];
		}
		if(!strcmp(argv[i], "-vec")) {
			vecFilename = argv[++i];
		}
		if(!strcmp(argv[i], "-rot")) {
			rot = atof(argv[++i]);
		}
		if(!strcmp(argv[i], "-syn")) {
			isSyn = true;
		}
		if(!strcmp(argv[i], "synDir")) {
			synDir = argv[++i];
		}
		if(!strcmp(argv[i], "-opt")) {
			isOpt = true;
		}
		if(!strcmp(argv[i], "-num")) {
			numDivTrain = atoi(argv[++i]);
		}
		if(!strcmp(argv[i], "-w")) {
			w = atoi(argv[++i]);
		}
		if(!strcmp(argv[i], "-h")) {
			h = atoi(argv[++i]);
		}
		if(!strcmp(argv[i], "-numStages")) {
			numStages = atoi(argv[++i]);
		}
	}
	
	FileSystem posFilesystem;
	std::vector<std::string>posImgs = posFilesystem.getFileList(posColDir, ALL);
	int numPos = posImgs.size();

	FileSystem negFilesystem;
	std::vector<std::string>negImgs = negFilesystem.getFileList(negColDir, ALL);
	int numNeg = negImgs.size();

	std::cout << "Number of positive images:" << numPos << std::endl
			  << "Number of negative images:" << numNeg << std::endl;

	if(numPos == 0) {
		std::cout << "Do not exist any training samples in current path, pls check whether the path exist\n";
		system("pause");
		return 0;
	}
	else if(numPos <= 650) {
		std::cout << "The training samples is not enough to train the good detector\n"
				  << "Recommend that the training set is prepared more than 650 samples\n"
				  << "Please try to get more training data to improve the performance\n";
		system("pause");
		return 0;
	}
	std::string posInfoname = "pos.txt";
	if(isSyn) {
		std::string synDirWithRot = synDir + boost::lexical_cast<std::string> (rot) + "\\";
		std::cout << "creating the synthesis images in " + synDirWithRot << std::endl;
		synthesisData(posImgs, synDirWithRot, w, h, rot);

		FileSystem synFilesystem;
		std::vector<std::string>synImgs = synFilesystem.getFileList(synDirWithRot, ALL);
		createInfoList(posInfoname, synImgs);
	}
	else {
		createInfoList(posInfoname, posImgs);
	}
	std::string negInfoname = "neg.txt";
	createInfoList(negInfoname, negImgs, false);

	std::string vecname = vecFilename + boost::lexical_cast<std::string>(rot) + "_" +
										boost::lexical_cast<std::string>(numPos) + ".vec";
	std::ostringstream createsamplesCmd;
	createsamplesCmd << "opencv_createsamples.exe -info " << posInfoname << " -vec "
					 << vecname << " -w " << w << " -h " << h << " -num " << numPos;
	
	std::cout << "------------- creating vec file -------------\n";
	std::cout << createsamplesCmd.str() << std::endl;
	system(createsamplesCmd.str().c_str());
	std::cout << "------------- training -------------\n";
	train(vecname, numPos, numNeg, numDivTrain, numStages, w, h);
	
	system("pause");
}