#ifndef __AB_SOUND_H__
#define __AB_SOUND_H__

#include <cstdint>

namespace AB {
    class Sound {
    public:
        void registerSoundPin(uint64_t totalcyls,bool plus, bool minus, bool oldPlus, bool oldMinus);
    };
}

#endif