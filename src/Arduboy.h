#ifndef _ARDUBOY
#define _ARDUBOY

#include "ATmega32u4.h"
#include "components/Display.h"

class Arduboy {
public:
	A32u4::ATmega32u4 mcu;
	AB::Display display;

	uint8_t execFlags = A32u4::ATmega32u4::ExecFlags_None;
	uint16_t targetFPS = 60;

	uint8_t buttonState = 0;
	enum {
		Button_Up_Bit =    7,
		Button_Right_Bit = 6,
		Button_Left_Bit =  5,
		Button_Down_Bit =  4,
		Button_A_Bit =     3,
		Button_B_Bit =     2
	};
	enum {
		Button_Up =    1 << Button_Up_Bit,
		Button_Right = 1 << Button_Right_Bit,
		Button_Left =  1 << Button_Left_Bit,
		Button_Down =  1 << Button_Down_Bit,
		Button_A =     1 << Button_A_Bit,
		Button_B =     1 << Button_B_Bit
	};

	Arduboy();

	bool load(const char* fileName);

	void newFrame();

	void pressButtons(uint8_t buttons);
	void releaseButtons(uint8_t buttons);
private:
	void updateButtons();
};

#endif