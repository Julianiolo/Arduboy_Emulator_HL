#include "Arduboy.h"

#include <stdio.h>
#include <string>
#include <iostream>

#include "StreamUtils.h"
#include "DataUtils.h"

#include "simavr/sim/avr_ioport.h"
#include "extras/Disassembler.h"

static avr_t* setup_avr() {
	avr_t* avr = avr_make_mcu_by_name("atmega32u4");
	avr_init(avr);
	
	return avr;
}

static avr_irq_t* setup_avr_callbacks(avr_t* avr) {
	const char* names[] = {"btn_pinf", "btn_pine", "btn_pinb"};
	avr_irq_t* irqs = avr_alloc_irq(&avr->irq_pool, 0, Arduboy::IRQ__COUNT, names);

	avr_connect_irq(&irqs[Arduboy::IRQ_PORTF], avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('F'), 0));
	avr_connect_irq(&irqs[Arduboy::IRQ_PORTE], avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('E'), 0));
	avr_connect_irq(&irqs[Arduboy::IRQ_PORTB], avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 0));
	avr_connect_irq(&irqs[Arduboy::IRQ_PORTD], avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('D'), 0));
	avr_connect_irq(&irqs[Arduboy::IRQ_SPI], avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 0));
	return irqs;
}

Arduboy::Arduboy() : avr(setup_avr()), irqs(setup_avr_callbacks(avr)), display(this) {
}
Arduboy::Arduboy(const Arduboy& src) : avr(src.avr), irqs(setup_avr_callbacks(avr)), display(src.display),
debug(src.debug), targetFPS(src.targetFPS), buttonState(src.buttonState), emulationSpeed(src.emulationSpeed)
{
	std::terminate();
	display.ab = this;
}
Arduboy::~Arduboy() {
	avr_free_irq(irqs, IRQ__COUNT);
	irqs = NULL;
	avr_terminate(avr);
	avr = NULL;
}
Arduboy& Arduboy::operator=(const Arduboy& src) {
	avr = src.avr; // TODO actually copy
	avr_free_irq(irqs, IRQ__COUNT);
	irqs = NULL;
	irqs = setup_avr_callbacks(avr);
	display = src.display;
	display.ab = this;

	debug = src.debug;
	targetFPS = src.targetFPS;

	buttonState = src.buttonState;
	emulationSpeed = src.emulationSpeed;
	return *this;
}

void Arduboy::reset() {
	avr_reset(avr);
	display.reset();
	sound.reset();
}

void Arduboy::runForCycs(uint64_t num_cycs) {
	updateButtons();

	uint64_t end_cycs = avr->cycle + num_cycs;
	while(avr->cycle < end_cycs) {
		avr_run(avr);

		uint16_t pc = avr->pc/2;
		uint64_t cycs = avr->cycle;
		uint16_t word1 = ((uint16_t)avr->flash[pc*2+1] << 8) | avr->flash[pc*2+0];
		uint16_t word2 = ((uint16_t)avr->flash[pc*2+3] << 8) | avr->flash[pc*2+2];

		auto disasm = A32u4::Disassembler::disassemble(word1, word2, pc);
		printf("@%6x:  %s\n", cycs, disasm.c_str());
	}
	
	display.update();
}
void Arduboy::newFrame() {
	runForCycs((uint64_t)(cycsPerFrame() * emulationSpeed));
}

uint64_t Arduboy::cycsPerFrame() const{
	return avr->frequency/targetFPS;
}

void Arduboy::pressButtons(uint8_t buttons) {
	buttonState |= buttons;
}
void Arduboy::releaseButtons(uint8_t buttons) {
	buttonState &= ~(buttons);
}

void Arduboy::updateButtons() {
	avr_raise_irq(&irqs[Arduboy::IRQ_PORTF], (~buttonState) & (Button_Up | Button_Right | Button_Left | Button_Down)); // mask: 0b11110000
	avr_raise_irq(&irqs[Arduboy::IRQ_PORTE], ~(buttonState & Button_A)); // bit: 6
	avr_raise_irq(&irqs[Arduboy::IRQ_PORTB], ~(buttonState & Button_B)); // bit: 4
}

void Arduboy::getState(std::ostream& output){
	//mcu.getState(output); // TODO
	display.getState(output);

	StreamUtils::write(output, debug);
	StreamUtils::write(output, targetFPS);
	StreamUtils::write(output, buttonState);
	StreamUtils::write(output, emulationSpeed);
}
void Arduboy::setState(std::istream& input){
	// mcu.setState(input); // TODO
	display.setState(input);

	StreamUtils::read(input, &debug);
	StreamUtils::read(input, &targetFPS);
	StreamUtils::read(input, &buttonState);
	StreamUtils::read(input, &emulationSpeed);
}
bool Arduboy::operator==(const Arduboy& other) const{
	// TODO
#define _CMP_(x) (x==other.x)
	return true && _CMP_(display) && _CMP_(debug) && _CMP_(targetFPS) && _CMP_(buttonState);
#undef _CMP_
}

size_t Arduboy::sizeBytes() const {
	size_t sum = 0;

	// sum += mcu.sizeBytes();  // TODO
	sum += display.sizeBytes();

	sum += sizeof(debug);
	sum += sizeof(targetFPS);

	sum += sizeof(buttonState);

	sum += sizeof(emulationSpeed);

	return sum;
}

uint32_t Arduboy::hash() const noexcept {
	uint32_t h = 0;
	//DU_HASHC(h, mcu);  // TODO
	DU_HASHC(h, display);

	DU_HASHC(h,debug);
	DU_HASHC(h,targetFPS);

	DU_HASHC(h,buttonState);

	DU_HASHC(h,emulationSpeed);

	return h;
}
