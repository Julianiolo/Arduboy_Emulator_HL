#ifndef _ARDUBOY_H
#define _ARDUBOY_H

#include "ATmega32u4.h"
#include "components/Display.h"

class Arduboy {
public:
	typedef A32u4::ATmega32u4::LogCallB LogCallB;
	typedef A32u4::ATmega32u4::LogCallBSimple LogCallBSimple;
private:
	LogCallB logCallB = defaultLog;
	LogCallBSimple logCallBSimple = defaultLogSimple;
	static void defaultLog(A32u4::ATmega32u4::LogLevel logLevel, const char *msg, const char *fileName, size_t lineNum, const char *Module);
	static void defaultLogSimple(A32u4::ATmega32u4::LogLevel logLevel, const char *msg);
public:
	A32u4::ATmega32u4 mcu;
	AB::Display display;

	uint8_t execFlags = A32u4::ATmega32u4::ExecFlags_None;
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

	Arduboy();

	bool loadFromHexString(const char* str, const char* str_end = 0);
	bool loadFromHexFile(const char* fileName);

	void reset();

	void newFrame(float speed = 1);
	uint64_t cycsPerFrame() const;

	void pressButtons(uint8_t buttons);
	void releaseButtons(uint8_t buttons);

	void activateLog();
	void setLogCallB(LogCallB newLogCallB);
	void setLogCallBSimple(LogCallBSimple newLogCallBSimple);

	void updateButtons();
private:
	static Arduboy* activeAB;
	static void log(A32u4::ATmega32u4::LogLevel logLevel, const char* msg, const char* fileName , size_t lineNum, const char* Module);
	static void logSimple(A32u4::ATmega32u4::LogLevel logLevel, const char* msg);
};

#endif