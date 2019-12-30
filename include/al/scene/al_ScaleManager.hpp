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
   * @brief Constructor using scala files (use for custom tunings)
   *
   * @param sclPath	Path to the scala file
   * @param A		Frequency value of A in Hz (Generally 440)
   */
  Scale(std::string sclPath, unsigned int A = 440);

  /**
   * @brief Constructs a scale using equal temperament (the usual tuning system)
   *
   * @param A		Frequency value of A in Hz
   */
  Scale(unsigned int A = 440);

  /**
   * @brief Provides a frequency for the given MIDI note value
   *
   * @param midiNote	An integer corresponding to a note on a piano keyboard
			(See https://newt.phys.unsw.edu.au/jw/notes.html)
   */
  bool freqFromMIDI(int midiNote);

  /**
   * @brief Maps a note on a computer keyboard to a frequency value
   *
   * @param key		Character of the key being pressed
   */
  bool freqFromASCII(char* key);

  ~Scale();
}

}

#endif
