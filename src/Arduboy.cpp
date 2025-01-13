#include "Arduboy.h"

#include <stdio.h>
#include <string>

#include "StreamUtils.h"
#include "DataUtils.h"

static avr_t* setup_avr() {
	avr_t* avr = avr_make_mcu_by_name("Atmega32u4");
	avr_init(avr);
	
	return avr;
}

static avr_irq_t* setup_avr_callbacks(avr_t* avr) {
	const char* names[] = {"A", "B"};
	avr_irq_t* irq = avr_alloc_irq(&avr->irq_pool, 0, 2, names);
	return irq;
}

Arduboy::Arduboy() : avr(setup_avr()), irqs(setup_avr_callbacks(avr)), display(avr) {
	mcu.setPinChangeCallB(genPinChangeFunc());
}
Arduboy::Arduboy(const Arduboy& src) : avr(avr), display(src.display),
debug(src.debug), targetFPS(src.targetFPS), buttonState(src.buttonState), emulationSpeed(src.emulationSpeed)
{
	mcu.setPinChangeCallB(genPinChangeFunc());
	display.avr = avr;
}
Arduboy& Arduboy::operator=(const Arduboy& src) {
	avr = src.avr; // TODO actually copy
	mcu.setPinChangeCallB(genPinChangeFunc());
	display = src.display;
	display.mcu = &mcu;

	debug = src.debug;
	targetFPS = src.targetFPS;

	buttonState = src.buttonState;
	emulationSpeed = src.emulationSpeed;
	return *this;
}

std::function<void(uint8_t pinReg, reg_t oldVal, reg_t val)> Arduboy::genPinChangeFunc(){
	return [&] (uint8_t pinReg, reg_t oldVal, reg_t val) {
		if(pinReg == A32u4::ATmega32u4::PinChange_PORTC) {
			bool plus = !!(val & (1 << 6));
			bool minus = !!(val & (1 << 7));
			bool oldPlus = !!(oldVal & (1 << 6));
			bool oldMinus = !!(oldVal & (1 << 7));
			sound.registerSoundPin(mcu.cpu.getTotalCycles(), plus, minus, oldPlus, oldMinus);
		}
	};
}


void Arduboy::reset() {
	mcu.reset();
	display.reset();
	sound.reset();
}

void Arduboy::newFrame() {
	updateButtons();
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

uint32_t Arduboy::hash() const noexcept {
	uint32_t h = 0;
	DU_HASHC(h, mcu);
	DU_HASHC(h, display);

	DU_HASHC(h,debug);
	DU_HASHC(h,targetFPS);

	DU_HASHC(h,buttonState);

	DU_HASHC(h,emulationSpeed);

	return h;
}
