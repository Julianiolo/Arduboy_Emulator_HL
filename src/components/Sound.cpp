#include "Sound.h"

void AB::Sound::registerSoundPin(uint64_t totalcyls, bool plus, bool minus, bool oldPlus, bool oldMinus) {
    if(plus == oldPlus && minus == oldMinus)
        return;
    
    bool isLoud = plus != oldPlus && minus != oldMinus;
    
}
