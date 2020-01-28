#include "al/scene/al_ScaleManager.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <cassert>
#include <cmath>

using namespace al;

Scale::Scale(std::string sclPath, unsigned int tonic, unsigned int midi) {
	this->tonic = tonic;
	this->tonic_midi = tonic_midi;

	/* Parsing the scl file */
	// TODO If the below code has become too large/messy to be readable, abstract
	// into own function

	// Stores the user's description of the tuning system 
	// (first line of the file)
	std::string description;

	// Stores the number of lines
	int numNotes = 0;

	// Stores each note's distance from the tonic in cents
	double* cents = NULL;

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
			scalaAssert(numNotes > 0, "the number notes in the scale is being read as zero");

			// Size of our tunings array depends on the number of
			// notes in the scale
			cents = new double[numNotes];

		} else {
			// The first two lines contain metadata
			int index = count - 2;
			cents[index] = parseLine(line);
			std::cout << parseLine(line) << std::endl;
		}
		count++;
	}

	// Using the newly-gathered cents data, generate tunings LUT
	this->computeTuning(cents, count);

	// Free heap
	delete[] cents;
}

Scale::Scale(unsigned int tonic, unsigned int midi) {
	this->tonic = tonic;
	this->tonic_midi = tonic_midi;

	// Computes tunings using hardcoded equally-tempered scale
	double cents[] = {100.0, 200.0, 300.0, 400.0, 500.0, 600.0, 700.0, 800.0, 900.0, 100.00, 1100.0, 1200.0};
	this->computeTuning(cents, 12);
}

Scale::~Scale() {

}

double Scale::freqFromMIDI(unsigned int midiNote) {
	scalaAssert(21 <= midiNote && midiNote <= 108, "the given MIDI note is out of range");
	return midi_lookup[midiNote];
}

bool Scale::freqFromASCII(char* key) {
	// TODO Implement
  return false;
}

void Scale::computeTuning(double* cents, int len) {
	// First, we must know the tonic. From there we work our way up and
	// down. 

	// Frequency value of given tonic
	int cur_tonic = this->tonic;

	// Number of notes above the root (used while iterating)
	int scale_degree = 0;
	
	// Going up from tonic (108 is the highest MIDI number)
	for(int i = this->tonic_midi; i < 108; i++) {
		// Calculate frequency of i'th scale degree
		double hz = cur_tonic * std::pow(2, cents[scale_degree] / 1200.0);
		this->midi_lookup[i] = hz;

		scale_degree++;

		// When we reach the top end of our scale, raise the tonic to a
		// new octave
		if(scale_degree >= len) {
			scale_degree = 0;
			
			cur_tonic *= std::pow(2, cents[len - 1] / 1200.0);
		}
	}

	// Reset variables
	// Start the tonic down an octave
	cur_tonic = this->tonic / std::pow(2, cents[len - 1] / 1200.0);

	// Start at the note below the octave, then go down to root
	scale_degree = len - 1;		

	// Going down from tonic (to 21)
	for(int i = this->tonic_midi; i > 21; i--) {
		// Calculate frequency
		double hz = cur_tonic * std::pow(2, cents[scale_degree] / 1200.0);
		this->midi_lookup[i] = hz;

		scale_degree--;

		if(scale_degree < 0) {
			cur_tonic *= 1 / std::pow(2, cents[len - 1] / 1200.0);
			scale_degree = len - 1;
		}
	}

  // TODO Delete after
  for(int i = 0; i < 100; i++) {
    std::cout << this->midi_lookup[i] << std::endl;
  }

}

void Scale::scalaAssert(bool assertion, std::string message) {
	if(assertion) return;

	std::string fullMessage =
		"\n"
		"Unfortunately, we are unable to continue because "
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

	unsigned int pos = line.find('/');
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
