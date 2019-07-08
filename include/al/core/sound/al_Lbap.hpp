#ifndef INCLUDE_AL_PANNING_LBAP
#define INCLUDE_AL_PANNING_LBAP

/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its
		contributors may be used to endorse or promote products derived from
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.


	File description:
	Layer Based Amplitude Panning

	File author(s):
	Andres Cabrera 2018 mantaraya36@gmail.com
*/

#include <map>

#include "al/core/math/al_Vec.hpp"
#include "al/core/spatial/al_DistAtten.hpp"
#include "al/core/spatial/al_Pose.hpp"
#include "al/core/sound/al_Speaker.hpp"
#include "al/core/sound/al_Vbap.hpp"

#define RAD_2_DEG_SCALE 57.29577951308232  // 360/(2*pi)

namespace al{

class LdapRing {
public:
    LdapRing(SpeakerLayout &sl) {
        vbap = std::make_shared<Vbap>(sl);
        elevation = 0;
        for (auto speaker: sl.speakers()) {
            elevation += speaker.elevation;
        }
        elevation /= sl.numSpeakers();
        vbap->compile();
    }

    std::shared_ptr<Vbap> vbap;
    float elevation;
};

/// Layer-based amplitude panner
///
/// @ingroup allocore
class Lbap : public Spatializer{
public:

    typedef enum {
        KEEP_SAME_ELEVATION = 0x1, // Don't discard triplets that have the same elevation
    } VbapOptions;

	/// @param[in] sl	A speaker layout
	Lbap(const SpeakerLayout &sl)
        : Spatializer(sl)
    {
    }

    virtual ~Lbap() override {
        if (buffer) {
            free(buffer);
        }
    }

    virtual void compile() override {
        std::map<int, SpeakerLayout> speakerRingMap;
        for (auto &speaker: mSpeakers) {
            if (speakerRingMap.find(speaker.group) == speakerRingMap.end()) {
                speakerRingMap[speaker.group] = SpeakerLayout();
            }
            speakerRingMap[speaker.group].addSpeaker(speaker);
        }
        for (auto speakerRing: speakerRingMap) {
            mRings.push_back(LdapRing(speakerRing.second));
        }
        // Sort by elevation
        std::sort(mRings.begin(), mRings.end(),
                  [](const LdapRing & a, const LdapRing & b) -> bool
        {
            return a.elevation > b.elevation;
        });
    }

    virtual void prepare(AudioIOData &io) override {
        if (buffer) {
            free(buffer);
        }
        buffer = (float *) malloc(2 * io.framesPerBuffer() * sizeof(float)); // Allocate 2 buffers
        bufferSize = io.framesPerBuffer();
    }


	virtual void renderSample(AudioIOData& io, const Pose& reldir, const float& sample, const int& frameIndex) override
    {

    }

	virtual void renderBuffer(AudioIOData& io, const Pose& listeningPose, const float *samples, const int& numFrames) override
    {
        Vec3d vec = listeningPose.vec();

        //Rotate vector according to listener-rotation
        Quatd srcRot = listeningPose.quat();
        vec = srcRot.rotate(vec);
        vec = Vec4d(-vec.z, -vec.x, vec.y);

        float elev = RAD_2_DEG_SCALE * atan(vec.z/sqrt(vec.x * vec.x + vec.y * vec.y));

        auto it = mRings.begin();
        while (it != mRings.end() && it->elevation > elev) {
            it++;
        }
        if (it == mRings.begin()) { // Top ring
            it->vbap->renderBuffer(io, listeningPose, samples, numFrames);
        } else if (it == mRings.end()) { // Bottom ring
            mRings.back().vbap->renderBuffer(io, listeningPose, samples, numFrames);
        } else { // Between inner rings
            auto topRingIt = it - 1; // top ring is previous ring
            float fraction = (elev - it->elevation)/(topRingIt->elevation - it->elevation); // elevation angle between layers
            float gainTop = sin(M_PI_2 * fraction);
            float gainBottom = cos(M_PI_2 *fraction);
            for (int i = 0; i < bufferSize; i++) {
                buffer[i] = samples [i] * gainTop;
                buffer[i + bufferSize] = samples [i] * gainBottom;
            }

            topRingIt->vbap->renderBuffer(io, listeningPose, buffer, bufferSize);
            it->vbap->renderBuffer(io, listeningPose, buffer + bufferSize, bufferSize);
        }
    }

    virtual void print(std::ostream &stream = std::cout) override {
        for (auto ring: mRings) {
            stream << " ---- Ring at elevation:" << ring.elevation << std::endl;
            ring.vbap->print(stream);
        }
    }

private:
	std::vector<LdapRing> mRings;
    float *buffer {nullptr}; // Two consecutive buffers (non-interleaved)
    int bufferSize {0};


};

} // al::
#endif