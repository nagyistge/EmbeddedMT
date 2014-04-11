#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <dirent.h>

#include "cm/global.hpp"
#include "imProc/imageProcBase.hpp"
#include "descriptor/sift.hpp"
#include "descriptor/surf.hpp"
#include "match/bfMatcher.hpp"
#include "match/flannBasedMatcher.hpp"
#include "draw/draw.hpp"
#include "displacement/displacementBase.hpp"

#include "application/findTheBall.hpp"

std::string getMethodPname(uint8_t matchAlgorithm);
GBL::CmRetCode_t getExecutors(uint32_t matchAlgorithm, const Descriptor::DescriptorInterface** descriptor,
		const Match::MatcherInterface** matcher);
GBL::CmRetCode_t writeResults(const char* outputFile, std::vector<GBL::Displacement_t> data);
GBL::CmRetCode_t setFiles (const char* const dir, GBL::ImageSequence_t& imageSequence);
bool isImage(struct dirent* dirp);

int main(int argc, char** argv) {
	uint32_t matchAlgorithm = 0;
	const char* outputFile = "displacements.result";
	const char* imageSequenceFolder = nullptr;

	const ImageProc::ImageProc* imProc = new ImageProc::ImageProcBase();
	const Draw::DrawInterface* drawer = new Draw::Draw();
	const Displacement::DisplacementInterface* displacement = new Displacement::DisplacementBase();

	const Descriptor::DescriptorInterface* descriptor = nullptr;
	const Match::MatcherInterface* matcher = nullptr;

	if (argc > 1) {
		imageSequenceFolder = argv[1];
	} else {
		std::cerr << "Image sequence folder not defined" << std::endl;
		return -1;
	}
	if (argc > 2) {
		matchAlgorithm = (uint32_t) std::strtoul(argv[2], NULL, 10);
	}
	if(argc > 3) {
		outputFile = argv[3];
	}
	std::cout << "Using " << getMethodPname(matchAlgorithm) << std::endl;

	if(getExecutors(matchAlgorithm, &descriptor, &matcher) != GBL::RESULT_SUCCESS) {
		std::cerr << "Could not get valid descriptor and matcher. Define a valid algorithm to be used." << std::endl;
		return -1;
	}

	GBL::ImageSequence_t imageSequence;
	if(setFiles(imageSequenceFolder, imageSequence) != GBL::RESULT_SUCCESS) {
		std::cerr << "Could not retrieve content of " << imageSequenceFolder << std::endl;
	}
	// Sort files
	std::sort(imageSequence.images.begin(), imageSequence.images.end());

	// Print files
	for(uint32_t i = 0; i < imageSequence.images.size(); i++) {
		std::cout << imageSequence.images[i] << std::endl;
	}

	clock_t tStart = clock();
	std::vector<GBL::Displacement_t> displacements = findTheBall(imageSequence, imProc, *drawer, *descriptor, *matcher, *displacement);
	float_t totalTimeElapsed = (float_t) (clock() - tStart)/ CLOCKS_PER_SEC;
	std::cout << "Total time taken: " << totalTimeElapsed << " s" << std::endl;

	if(writeResults(outputFile, displacements) != GBL::RESULT_SUCCESS) {
		std::cerr << "Could not write results to file " << outputFile << std::endl;
	}
	return 0;
}

GBL::CmRetCode_t writeResults(const char* outputFile, std::vector<GBL::Displacement_t> data) {
	std::ofstream file;
	file.open(outputFile);
	if(! file.is_open()) {
		return GBL::RESULT_FAILURE;
	}
	for(uint32_t i = 0; i < data.size(); i++) {
		file << data[i].x << "\t" << data[i].y << std::endl;
	}
	file.close();
	return GBL::RESULT_SUCCESS;
}

std::string getMethodPname(uint8_t matchAlgorithm) {
	std::string algoName = "Unknown";
	switch (matchAlgorithm) {
	case 0:
		algoName = "SURF+brute force";
		break;
	case 1:
		algoName = "SURF+FLANN";
		break;
	case 2:
		algoName = "SIFT+brute force";
		break;
	case 3:
		algoName = "SIFT+FLANN";
		break;
	}
	return algoName;
}

GBL::CmRetCode_t getExecutors(uint32_t matchAlgorithm, const Descriptor::DescriptorInterface** descriptor,
		const Match::MatcherInterface** matcher) {
	switch (matchAlgorithm) {
	case 0:
		*descriptor = new Descriptor::Surf();
		*matcher = new Match::BfMatcher();
		break;
	case 1:
		*descriptor = new Descriptor::Surf();
		*matcher = new Match::FlannBasedMatcher();
		break;
	case 2:
		*descriptor = new Descriptor::Sift();
		*matcher = new Match::BfMatcher();
		break;
	case 3:
		*descriptor = new Descriptor::Sift();
		*matcher = new Match::FlannBasedMatcher();
		break;
	default:
		std::cout << "Unknown matching algorithm" << std::endl;
		return GBL::RESULT_FAILURE;
	}
	return GBL::RESULT_SUCCESS;
}

GBL::CmRetCode_t setFiles (const char* const dir, GBL::ImageSequence_t& imageSequence) {
	snprintf(imageSequence.backgroundImage, (size_t) GBL::maxFilenameLength, "%s/background.jpg", dir);
	std::cout << imageSequence.backgroundImage << std::endl;

    DIR *dp;
    struct dirent *dirp;
    dp  = opendir(dir);
    if(dp == nullptr) {
        std::cout << "Error opening " << dir << std::endl;
        return GBL::RESULT_FAILURE;
    }

    dirp = readdir(dp);
    while (dirp != nullptr) {
    	if(isImage(dirp)) {
    		char imagePath[GBL::maxFilenameLength];
    		snprintf(imagePath, (size_t) GBL::maxFilenameLength, "%s/%s", dir, dirp->d_name);
    		imageSequence.images.push_back(imagePath);
    	}
    	dirp = readdir(dp);
    }
    closedir(dp);
    return GBL::RESULT_SUCCESS;
}

bool isImage(struct dirent* dirp) {
	// Check if file
	if(dirp->d_type != DT_REG) {
		return false;
	}

	size_t len = strlen(dirp->d_name);
	// Check for length -> point to "." and ".."
	if(len <= 2) {
		return false;
	}

	// We use filenames in stead of MIME-type for the moment
	if(std::strncmp(dirp->d_name, "background.jpg", sizeof("background.jpg")) == 0) {	// Check if it is the background image
		return false;
	}

	if(std::strncmp(&dirp->d_name[len-4], ".jpg", sizeof(".jpg")) == 0) {
		return true;
	}
	return false;
}
