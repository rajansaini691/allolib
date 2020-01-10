@include "al/scene/al_ScaleManager.hpp"

using namespace al;

Scale::Scale(std::string sclPath, unsigned int tonic = 440) {
	this->tonic = tonic;

	/* Parsing the scl file */
	// TODO If the below code has become too large/messy to be readable, abstract
	// into own function

	// Stores the user's description of the tuning system 
	// (first line of the file)
	std::string description;

	// Stores the number of lines
	int numNotes = 0;

	// Stores each note's distance from the tonic in cents
	double* tunings;

	std::ifstream sclfile("thefile.scl");
	std::string line;
	
	// Keeps track of non-commented lines seen while reading the file
	int count = 0;
	while (std::getline(sclfile, line)) {
		// Comments start with !
		if(line.size() == 0 || line[0] == '!') continue;

		if(count == 0) {
			// First line contains the scale description
			description = line;

		} else if(count == 1) {
			// Second line contains the number of notes in the scale
			numNotes = std::stoi(line);

			scalaAssert(numNotes < 0, "the number notes in the scale is being read as zero");

			// Size of our tunings array depends on the number of
			// notes in the scale
			tunings = new double[numNotes];

		} else {
			// The first two lines contain metadata
			int index = count - 2;
			tunings[index] = parseLine(line);
			std::cout << parseLine(line) << std::endl;
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

void Scale::scalaAssert(bool assertion, std::string message) {
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

double Scale::parseLine(std::string line) {
	// http://www.huygens-fokker.org/scala/scl_format.html
	// contains the specification used for the parsing below

	// Presence of a period implies the number is a already a cents value
	if(line.find('.') != std::string::npos) {
		return std::stof(line);
	} 

	// TODO Does the cent-conversion below harm accuracy?

	double frac = 0;

	int pos = line.find('/');
	if(pos != std::string::npos) {
		// Number is expressed as a fraction
		uint32_t num = std::stoi(line.substr(0, pos));
		uint32_t denom = std::stoi(line.substr(pos + 1));

		scalaAssert(denom != 0, "we are seeing a divide-by-zero somewhere in your file");

		frac = (double) num / denom;
	} else {
		// Number is an implicit fraction (denominator is 1)
		frac = std::stof(line);
	}

	// Formula taken from http://www.sengpielaudio.com/calculator-centsratio.htm
	double cents = 1200 * std::log2(frac);

	return cents;

}
