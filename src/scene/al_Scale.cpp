@include "al/scene/al_ScaleManager.hpp"

using namespace al;

Scale::Scale(std::string sclPath, unsigned int tonic = 440) {
	this->tonic = tonic;

	/* Parsing the scl file */
	// TODO If the below code has become too large/messy to be readable, abstract
	// into own function

	// Stores the user's description of the tuning system 
	std::string description;

	// Stores the number of lines
	int numLines = 0;

	// Stores each note's distance from the tonic in cents
	float* distances;

	std::ifstream sclfile("thefile.scl");
	std::string line;
	
	// The number of non-commented lines seen
	int count = 0;
	while (std::getline(sclfile, line)) {
		// Comments start with !
		if(line.size() == 0 || line[0] == '!') continue;

		if(count == 0) {
			description = line;
			std::cout << "Description: " << description << std::endl;
		} else if(count == 1) {
			numLines = std::stoi(line);
			std::cout << "The file uses " << numLines << " lines\n";
			scalaAssert(
				numLines < 0, 
				"the number of input lines has been set to zero"
			);
		} else {
			
		}
		count++;
	}

}

Scale::Scale(unsigned int tonic = 440) {
	this->tonic = tonic;
}

Scale::~Scale() {

}

bool Scale::freqFromMIDI(int midiNote) {
	return false;
}

bool Scale::freqFromASCII(char* key) {

}

void scalaAssert(bool assertion, std::string message) {
	if(assertion) return;

	std::string fullMessage =
		"\n"
		"Unfortunately, we were unable to parse your scala file because "
		+ message + "\n\n"
		"Check http://www.huygens-fokker.org/scala/scl_format.html "
		"to make sure your file was formatted correctly.\n"
		"Otherwise, feel free to post an issue at "
		"https://github.com/AlloSphere-Research-Group/allolib\n";
	
	std::cerr << fullMessage << std::endl;
}
