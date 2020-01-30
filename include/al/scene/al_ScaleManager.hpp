// TODO Conventions?
// FIXME Paths of input files are currently resolved relative to the compiled binary

#ifndef AL_SCALE
#define AL_SCALE

#include <string>

namespace al {

/**
 * @brief The Scale class
 *
 * Provides MIDI-to-frequency conversion using tuning systems defined in a scala
 * file (see http://www.huygens-fokker.org/scala/scl_format.html).
 */
class Scale {
 public:

  /**
   * @brief		Constructor using scala files (use for custom tunings)
   * @detailed		Creates a custom-tuned scale based on the given .scl
   *			        file and a given fundamental frequency
   *			        For example, to use Just Intonation based on A440, 
   *			        set the pitch standard to 440 Hz
   *
   * @param sclfile   References the scala file (uses an already-initialized
   *                  ifstream object to ensure file exists). For now, use paths
   *                  relative to the executable.
   * @param tonic	Frequency value of tonic note in Hz 
   */
  Scale(std::ifstream & sclfile, unsigned int tonic = 440, unsigned int midi = 69);

  /**
   * @brief		Constructs a scale using equal temperament (the usual tuning system)
   * @detailed		If you do not need precise control over your tunings,
   *			use this constructor instead. 
   *
   * @param tonic	Frequency value of A in Hz
   * @param midi	MIDI note of the given tonic
   */
  Scale(unsigned int tonic = 440, unsigned int midi = 69);

  /**
   * @brief		Provides a frequency for the given MIDI note value
   *
   * @param midiNote	An integer corresponding to a note on a piano keyboard
			(See https://newt.phys.unsw.edu.au/jw/notes.html)
   */
  double freqFromMIDI(unsigned int midiNote);

  /**
   * @brief		Maps a note on a computer keyboard to a frequency value
   *
   * @param key		Character of the key being pressed
   */
  bool freqFromASCII(char* key);

  ~Scale();

  private:
   /**
    * Value of A in Hz (generally 440)
    */
   unsigned int tonic;

   /**
    * MIDI note number of tonic
    */
   unsigned int tonic_midi;

   /**
    * Though MIDI notes will only generally range from 21 to 108, the LUT 
    * (lookup table) is of size 108 rather than 87 for readability.
    */
   double midi_lookup[108] = {0};

   /**
    * @brief	Wrapper for C++ assert, using a more detailed message.
    *
    * @param assertion	Some boolean expression to be evaluated
    */
   void scalaAssert(bool assertion, std::string message);

   /*
    * @brief	Used internally
    * @detailed	Computes a lookup table mapping MIDI notes to frequency values
    */
   void computeTuning(double* cents, int len);

   /**
    * @brief		Used internally while parsing the scala files
    * @detailed		Converts a line of text within a scala file to its 
    *			representation in cents
    */
   double parseLine(std::string line);


};

}

#endif
