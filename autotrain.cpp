#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <opencv.hpp>
#include "../../../include/include/FileSystem.h"

void createInfoList(std::string infoname, std::vector<std::string> imgList) {
	std::cout << "creating info list.....\n";
	std::fstream fs(infoname, std::ios::out);
	for(std::vector<std::string>::iterator it = imgList.begin(); it != imgList.end(); it++) {
		std::string filename = *it;
		cv::Mat src = cv::imread(filename);
		int numObject = 1;
		int w = src.cols;
		int h = src.rows;
		fs << filename << " " << numObject << " 0 0 " << w << " " << h << std::endl;
	}
	fs.close();
}

int main(int argc, char *argv[])
{
	if(argc == 1) {
	std::cout << "Welcome to automatically adaboost learning system\n"
			  << "Usuage: auto_adaboost.exe\n"
			  << "[-info <collection_directory>]\n"
			  << "[-vec <output_vec_directory>]\n"
			  << "[-w <sample_width = 24>]\n [-h <sample_height = 24>]";
	}
	
	int w = 24;
	int h = 24;
	std::string vecDir;
	std::string colDir;

	for(int i = 1; i < argc; i++) {
		if(!strcmp(argv[i], "-info")) {
			colDir = argv[++i];
		}
		if(!strcmp(argv[i], "-vec")) {
			vecDir = argv[++i];
		}
		if(!strcmp(argv[i], "-w")) {
			w = atoi(argv[++i]);
		}
		if(!strcmp(argv[i], "-h")) {
			h = atoi(argv[++i]);
		}
	}

	FileSystem filesystem;
	std::vector<std::string>imgList = filesystem.getFileList(colDir, ALL, true);
	int num = imgList.size();
	std::string infoname = "./info.txt";
	std::string vecname = vecDir + "/test.vec";
	createInfoList(infoname, imgList);

	std::ostringstream createsamples;
	createsamples << "opencv_createsamples.exe -info " << infoname << " -vec "
				  << vecname << " -w " << w << " -h " << h << " -num " << num;
	
	std::cout << createsamples.str() << std::endl;

	system(createsamples.str().c_str());
	//system("opencv_traincascade");
	system("pause");
}
