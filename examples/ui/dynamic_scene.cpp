// Example of how to use DynamicScene
//
// A DynamicScene manages the insertion and removal of PositionedVoice nodes in
// its rendering graph. A DynamicScene has three rendering contexts: The
// update() or simulation context, where state and internal changes should be
// computed, and the onProcess() contexts for audio and graphics.
//
// Author/Date
//

#include "Gamma/Envelope.h"    // AD<>
#include "Gamma/Oscillator.h"  // Sine<>
using namespace gam;

#include "al/core/app/al_App.hpp"
#include "al/core/graphics/al_Shapes.hpp"
#include "al/util/scene/al_DynamicScene.hpp"
using namespace al;

// XXX use struct rather than class in user-facing code; this removes the need
// for keywords such as public, private, protected, friend, etc. introducing
// these keywords (and the concepts they imply) now is unnecessary and
// confusing. the user (generally a novice creative coder) can always learn
// about this stuff later, when they need it. it is unlikely that the design of
// this kind of user code will benefit from the use of class(as opposed to
// stuct).
//
struct SimpleVoice : PositionedVoice {
  // XXX put all member variables at the front of the class; the most important
  // aspect of objects is their data. how many bytes? what types? this
  // information is shown in two places: 1) the declaration of member variables
  // and 2) in the inheritance list where each parent is its own block of data
  // that precedes the data of this class in memory. putting member variable
  // declarations immediately after the inheritance list allows use to see all
  // the class data in one place, but it mirrors how the data is laid out in
  // memory. the reasons for putting member declarations at the bottom of the
  // class do not apply to user-facing code.
  //
  Parameter mFreq{"Freq"};
  Sine<> mOsc;
  AD<> mAmpEnv{2.0f, 2.0f};
  Mesh mMesh;

  SimpleVoice() {
    addTorus(mMesh);
    mMesh.primitive(Mesh::LINE_STRIP);

    // register each parameter so setParamFields() works later..
    registerParameterAsField(mFreq);  // XXX was *this << mFreq
    // XXX don't use the stream operator for anything other than stream
    // operations; << and >> are confusing and ugly. for streams << and >>
    // supposedly have visual mnemonic value, but for callback registration i
    // don't see any value. you just have to remember to use << or >>?

    // changes to mFreq will automatically set the frequency of mOsc
    mFreq.registerChangeCallback([this](float value) { mOsc.freq(value); });
  }

  // YYY who calls this? how often? for what purpose? the answers should be in a
  // comment before the method
  //
  virtual void update(double dt) override {
    mFreq = mFreq * 0.995;
    pose().vec().y = mAmpEnv.value() * 3;
    pose().vec().x = mFreq / 440.0;
  }

  // YYY who calls this? how often? for what purpose? the answers should be in a
  // comment before the method
  //
  virtual void onProcess(AudioIOData &io) override {
    while (io()) {
      // += not = ??
      io.out(0) += mOsc() * mAmpEnv() * 0.05;
    }
    if (mAmpEnv.done()) {
      free();  // tells DynamicScene it may release this voice
    }
  }

  // YYY who calls this? how often? for what purpose? the answers should be in a
  // comment before the method
  //
  virtual void onProcess(Graphics &g) override {
    HSV c;
    c.h = mAmpEnv.value();
    g.color(Color(c));
    g.draw(mMesh);
  }

  // YYY who calls this? how often? for what purpose? the answers should be in a
  // comment before the method
  //
  virtual void onTriggerOn() override {
    pose().vec() = {mFreq / 440.0, 0.0, -10.0};
    mAmpEnv.reset();
  }
};

struct MyApp : App {
  DynamicScene scene{8};  // 8 is the number of threads to use

  virtual void onCreate() override {
    scene.showWorldMarker(false);
    scene.registerSynthClass<SimpleVoice>();
    // Preallocate 300 voices
    scene.allocatePolyphony("SimpleVoice", 300);
    scene.prepare(audioIO());
  }

  virtual void onAnimate(double dt) override {
    static double timeAccum = 0.1;
    timeAccum += dt;
    if (timeAccum > 0.1) {
      // Trigger one new voice every 0.05 seconds
      // First get a free voice of type SimpleVoice
      auto *freeVoice = scene.getVoice<SimpleVoice>();
      // Then set its parameters (this voice only has one parameter Freq)
      auto params = std::vector<float>{880.0f};
      freeVoice->setParamFields(params);
      // Set a position for it
      // Trigger it (this inserts it into the chain)
      scene.triggerOn(freeVoice);
      timeAccum -= 0.1;
    }

    scene.update(dt);  // Update all nodes in the scene
  }

  virtual void onSound(AudioIOData &io) override { scene.render(io); }

  virtual void onDraw(Graphics &g) override {
    g.clear();
    g.pushMatrix();
    scene.render(g);
    g.popMatrix();
  }
};

int main() {
  MyApp app;
  app.initAudio(44100., 512, 2, 2);
  // tell Gamma the sample rate
  gam::Domain::master().spu(app.audioIO().framesPerSecond());
  app.start();
}
