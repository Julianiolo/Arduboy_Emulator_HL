#include "Arduboy.h"

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
	mcu.execute(cycsPerFrame(), execFlags);
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