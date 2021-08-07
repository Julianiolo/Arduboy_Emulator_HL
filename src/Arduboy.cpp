#include "Arduboy.h"

Arduboy::Arduboy() : display(&mcu) {
	mcu.dataspace.setSPIByteCallB(display.spiCallB);
	mcu.setLogCallB(getLog);
	mcu.setLogCallBSimple(getLogSimple);
	activate();
}

bool Arduboy::load(const char* fileName) {
	return mcu.flash.loadFromHexFile(fileName);
}

void Arduboy::reset() {
	
	mcu.reset();
	display.reset();
}

void Arduboy::newFrame() {
	updateButtons();
	mcu.execute(A32u4::CPU::ClockFreq/targetFPS, execFlags);
	display.update();
}

void Arduboy::pressButtons(uint8_t buttons) {

}
void Arduboy::releaseButtons(uint8_t buttons) {

}

void Arduboy::updateButtons() {
	mcu.dataspace.setBitsTo(A32u4::DataSpace::Consts::PINF, 0b11110000, (~buttonState) & (Button_Up | Button_Right | Button_Left | Button_Down));
	mcu.dataspace.setBitTo(A32u4::DataSpace::Consts::PINE, 6, !(buttonState & Button_A));
	mcu.dataspace.setBitTo(A32u4::DataSpace::Consts::PINB, 4, !(buttonState & Button_B));
}

Arduboy* Arduboy::activeAB = nullptr;
void Arduboy::activate(){
	activeAB = this;
}
void Arduboy::getLog(const char* msg, A32u4::ATmega32u4::LogLevel logLevel, const char* fileName , size_t lineNum, const char* Module){
	if(activeAB != nullptr){
		if(activeAB->logCallB != nullptr){
			activeAB->logCallB(msg,logLevel,fileName,lineNum,Module);
		}
	}
}
void Arduboy::getLogSimple(const char* msg, A32u4::ATmega32u4::LogLevel logLevel){
	if(activeAB != nullptr){
		if(activeAB->logCallBSimple != nullptr){
			activeAB->logCallBSimple(msg,logLevel);
		}
	}
}

void Arduboy::setLogCallB(LogCallB newLogCallB){
	logCallB = newLogCallB;
}
void Arduboy::setLogCallBSimple(LogCallBSimple newLogCallBSimple){
	logCallBSimple = newLogCallBSimple;
}