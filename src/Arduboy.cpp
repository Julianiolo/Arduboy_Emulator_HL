#include "Arduboy.h"

#include <stdio.h>
#include <string>

Arduboy::Arduboy() : display(&mcu) {
	activateLog();
	mcu.dataspace.setSPIByteCallB(display.spiCallB);
	mcu.setLogCallB(log);
	mcu.setLogCallBSimple(logSimple);
}

bool Arduboy::loadFromHexString(const char* str, const char* str_end) {
	return mcu.flash.loadFromHexString(str, str_end);
}
bool Arduboy::loadFromHexFile(const char* fileName) {
	return mcu.flash.loadFromHexFile(fileName);
}

void Arduboy::reset() {
	mcu.reset();
	display.reset();
}

void Arduboy::newFrame() {
	updateButtons();
	display.activate();
	mcu.execute((uint64_t)(cycsPerFrame() * emulationSpeed), execFlags);
	display.update();
}

uint64_t Arduboy::cycsPerFrame() const{
	return A32u4::CPU::ClockFreq/targetFPS;
}

void Arduboy::pressButtons(uint8_t buttons) {
	buttonState |= buttons;
}
void Arduboy::releaseButtons(uint8_t buttons) {
	buttonState &= ~(buttons);
}

void Arduboy::updateButtons() {
	mcu.dataspace.setBitsTo(A32u4::DataSpace::Consts::PINF, 0b11110000, (~buttonState) & (Button_Up | Button_Right | Button_Left | Button_Down));
	mcu.dataspace.setBitTo(A32u4::DataSpace::Consts::PINE, 6, !(buttonState & Button_A));
	mcu.dataspace.setBitTo(A32u4::DataSpace::Consts::PINB, 4, !(buttonState & Button_B));
}

Arduboy* Arduboy::activeAB = nullptr;
void Arduboy::activateLog(){
	activeAB = this;
	mcu.activateLog();
}
void Arduboy::log(A32u4::ATmega32u4::LogLevel logLevel, const char* msg, const char* fileName , size_t lineNum, const char* Module){
	MCU_ASSERT(activeAB != nullptr);
	MCU_ASSERT(activeAB->logCallB != nullptr);
	activeAB->logCallB(logLevel, msg,fileName,lineNum,Module);
}
void Arduboy::logSimple(A32u4::ATmega32u4::LogLevel logLevel, const char* msg){
	MCU_ASSERT(activeAB != nullptr);
	MCU_ASSERT(activeAB->logCallBSimple != nullptr);
	activeAB->logCallBSimple(logLevel, msg);
}

void Arduboy::setLogCallB(LogCallB newLogCallB){
	logCallB = newLogCallB;
}
void Arduboy::setLogCallBSimple(LogCallBSimple newLogCallBSimple){
	logCallBSimple = newLogCallBSimple;
}

void Arduboy::defaultLog(A32u4::ATmega32u4::LogLevel logLevel, const char *msg, const char *fileName, size_t lineNum, const char *Module){
	printf("[%s]%s: %s @%s:%" MCU_PRIuSIZE "\n", 
		A32u4::ATmega32u4::logLevelStrs[logLevel],
		Module != 0 ? (std::string("[")+Module+"]").c_str() : "",
		msg,
		fileName, lineNum
	);
}
void Arduboy::defaultLogSimple(A32u4::ATmega32u4::LogLevel logLevel, const char *msg){
	printf("[%s]: %s\n", 
		A32u4::ATmega32u4::logLevelStrs[logLevel],
		msg
	);
}

void Arduboy::getState(std::ostream& output){
	mcu.getState(output);
	display.getState(output);

	output << execFlags;
	output << targetFPS;
	output << buttonState;
}
void Arduboy::setState(std::istream& input){
	mcu.setState(input);
	display.setState(input);

	input >> execFlags;
	input >> targetFPS;
	input >> buttonState;
}
