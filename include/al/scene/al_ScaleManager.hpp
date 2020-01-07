// TODO Should be a separate file or part of PolySynth?
// TODO Conventions?

#ifndef AL_SCALE
#define AL_SCALE

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
   *			file and a given fundamental frequency
   *			For example, to use Just Intonation based on A440, 
   *			set the pitch standard to 440 Hz
   *
   * @param sclPath		Path to the scala file
   * @param tonic	Frequency value of tonic note in Hz 
   */
  Scale(std::string sclPath, unsigned int tonic = 440);

  /**
   * @brief		Constructs a scale using equal temperament (the usual tuning system)
   * @detailed		If you do not need precise control over your tunings,
   *			use this constructor instead. 
   *
   * @param tonic	Frequency value of A in Hz
   */
  Scale(unsigned int tonic = 440);

  /**
   * @brief		Provides a frequency for the given MIDI note value
   *
   * @param midiNote	An integer corresponding to a note on a piano keyboard
			(See https://newt.phys.unsw.edu.au/jw/notes.html)
   */
  bool freqFromMIDI(int midiNote);

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
    * Though MIDI notes will only generally range from 21 to 108, the LUT 
    * (lookup table) is of size 108 rather than 87 for readability
    */
   float tunings[108];

   /**
    * @brief	Wrapper for C++ assert, using a more detailed message.
    *
    * @param assertion	Some boolean expression to be evaluated
    */
   void scalaAssert(bool assertion, std::string message) {
   
   }

}

#endif
