#include "Sound.h"

AB::Sound::Sound() {
    buffer.push_back({ false, false, 0 });
}

void AB::Sound::registerSoundPin(uint64_t totalCyls, bool plus, bool minus, bool oldPlus, bool oldMinus) {
    if(plus == oldPlus && minus == oldMinus)
        return;
    
    bool isLoud = plus != oldPlus && minus != oldMinus;
    
    buffer.push_back({plus, isLoud, totalCyls-bufferStart});
}

void AB::Sound::clearBuffer(uint64_t totalCycs){
    auto last = buffer.back();
    last.offset = 0;
    buffer.clear();
    buffer.push_back(last);
    bufferStart = totalCycs;
}

