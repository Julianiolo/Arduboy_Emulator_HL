#include "Arduboy.h"

Arduboy::Arduboy() : display(&mcu) {
	mcu.dataspace.setSPIByteCallB(display.spiCallB);
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
