#ifndef _ARDUBOY_H
#define _ARDUBOY_H

#include <iostream>

#include "ATmega32u4.h"
#include "components/Display.h"
#include "components/Sound.h"

class Arduboy {
public:
	A32u4::ATmega32u4 mcu;
	AB::Display display;
	AB::Sound sound;

	bool debug = true;
	uint16_t targetFPS = 60;

	uint8_t buttonState = 0;
	enum {
		Button_Up_Bit    = 7,
		Button_Right_Bit = 6,
		Button_Left_Bit  = 5,
		Button_Down_Bit  = 4,
		Button_A_Bit     = 3,
		Button_B_Bit     = 2
	};
	enum {
		Button_None  = 0,
		Button_Up    = 1 << Button_Up_Bit,
		Button_Right = 1 << Button_Right_Bit,
		Button_Left  = 1 << Button_Left_Bit,
		Button_Down  = 1 << Button_Down_Bit,
		Button_A     = 1 << Button_A_Bit,
		Button_B     = 1 << Button_B_Bit
	};

	float emulationSpeed = 1;

private:
	std::function<void(uint8_t pinReg, reg_t oldVal, reg_t val)> genPinChangeFunc();
public:

	Arduboy();
	Arduboy(const Arduboy& src);
	Arduboy& operator=(const Arduboy& src);

	void reset();

	void newFrame();
	uint64_t cycsPerFrame() const;

	void pressButtons(uint8_t buttons);
	void releaseButtons(uint8_t buttons);

	void updateButtons();

	void getState(std::ostream& output);
	void setState(std::istream& input);

	bool operator==(const Arduboy& other) const;
	size_t sizeBytes() const;
	uint32_t hash() const noexcept;
};

namespace DataUtils {
	inline size_t approxSizeOf(const Arduboy& v) {
		return v.sizeBytes();
	}
}

template<>
struct std::hash<Arduboy>{
	inline std::size_t operator()(const Arduboy& v) const noexcept{
		return (size_t)v.hash();
	}
};

#endif