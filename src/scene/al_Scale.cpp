@include "al/scene/al_ScaleManager.hpp"

using namespace al;

Scale::Scale(std::string sclPath, unsigned int tonic = 440) {
	this->tonic = tonic;

	/* Parsing the scl file */
	// TODO If the below code has become too large/messy to be readable, abstract
	// into own function


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
