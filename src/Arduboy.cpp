#include "Arduboy.h"

#include <stdio.h>
#include <string>

#include "StreamUtils.h"

Arduboy::Arduboy() : display(&mcu) {
	mcu.dataspace.setSPIByteCallB(display.spiCallB);
}
Arduboy::Arduboy(const Arduboy& src) : mcu(src.mcu), display(src.display),
debug(src.debug), targetFPS(src.targetFPS), buttonState(src.buttonState), emulationSpeed(src.emulationSpeed)
{
	display.mcu = &mcu;
}
Arduboy& Arduboy::operator=(const Arduboy& src) {
	mcu = src.mcu;
	display = src.display;
	display.mcu = &mcu;

	debug = src.debug;
	targetFPS = src.targetFPS;

	buttonState = src.buttonState;
	emulationSpeed = src.emulationSpeed;
	return *this;
}

void Arduboy::reset() {
	mcu.reset();
	display.reset();
}

void Arduboy::newFrame() {
	updateButtons();
	display.activate();
	mcu.execute((uint64_t)(cycsPerFrame() * emulationSpeed), debug);
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

void Arduboy::getState(std::ostream& output){
	mcu.getState(output);
	display.getState(output);

	StreamUtils::write(output, debug);
	StreamUtils::write(output, targetFPS);
	StreamUtils::write(output, buttonState);
	StreamUtils::write(output, emulationSpeed);
}
void Arduboy::setState(std::istream& input){
	mcu.setState(input);
	display.setState(input);

	StreamUtils::read(input, &debug);
	StreamUtils::read(input, &targetFPS);
	StreamUtils::read(input, &buttonState);
	StreamUtils::read(input, &emulationSpeed);
}
bool Arduboy::operator==(const Arduboy& other) const{
#define _CMP_(x) (x==other.x)
	return _CMP_(mcu) && _CMP_(display) && _CMP_(debug) && _CMP_(targetFPS) && _CMP_(buttonState);
#undef _CMP_
}

size_t Arduboy::sizeBytes() const {
	size_t sum = 0;

	sum += mcu.sizeBytes();
	sum += display.sizeBytes();

	sum += sizeof(debug);
	sum += sizeof(targetFPS);

	sum += sizeof(buttonState);

	sum += sizeof(emulationSpeed);

	return sum;
}