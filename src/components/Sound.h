#ifndef __AB_SOUND_H__
#define __AB_SOUND_H__

#include <cstdint>
#include <vector>

namespace AB {
    class Sound {
    public:
        uint64_t bufferStart = 0;
        struct Sample {
            bool on;
            bool isLoud;
            uint64_t offset;
        };
        std::vector<Sample> buffer;

        Sound();

        void reset();

        void registerSoundPin(uint64_t totalCyls,bool plus, bool minus, bool oldPlus, bool oldMinus);
        void clearBuffer(uint64_t totalCycs);
    };
}

#endif