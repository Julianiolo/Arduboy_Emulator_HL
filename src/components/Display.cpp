#include "Display.h"
#include <iostream>

AB::Display::Display(A32u4::ATmega32u4* mcu) : mcu(mcu)
#if AB_USE_HEAP
,pixels(WIDTH*HEIGHT), pixelsRaw((WIDTH*HEIGHT)/8)
#endif
{
	activate();
}

void AB::Display::reset() {
	on = false;
	screenOverride = false;
	screenOverrideVal = false;
	invert = false;

	startLineReg = 0;
	clockDevisor = 1;
	oscFreq = 0b1000;

	addrPtr = 0;

	parameterStackPointer = 0;
	isRecivingParameters = false;
	currentCommandID = -1;
	currentCommandByte = -1;
}

void AB::Display::reciveSPIByte(uint8_t byte){
	if (isDataMode())
		reciveDataByte(byte);
	else
		reciveCommandByte(byte);
}

void AB::Display::reciveCommandByte(uint8_t byte) {
	if (!isRecivingParameters) {
		currentCommandByte = byte;
		currentCommandID = getCommandInd(byte);
		if (currentCommandID == 255) {
			std::cout << "cant find display command id for: " << (int)byte << std::endl;
			abort();
		}
		if (commands[currentCommandID].paramAmt == 0)
			handleCurrentCommand();
		else
			startRecivingParams();
	}
	else {
		parameterStack[parameterStackPointer++] = byte;
		if (parameterStackPointer == commands[currentCommandID].paramAmt) {
			stopRecivingParams();
			handleCurrentCommand();
		}
	}
}
void AB::Display::reciveDataByte(uint8_t byte) {
	pixelsRaw[addrPtr] = byte;
	addrPtr = (addrPtr + 1) % ((WIDTH * HEIGHT) / 8);
}

void AB::Display::handleCommand(uint8_t byte, uint8_t id) {
	if (id == (uint8_t)-1)
		id = getCommandInd(byte);

	switch (id) {
		case 0:
			break;
		case 1: {
			bool forceOn = byte & 0b1;
			screenOverride = forceOn;
			screenOverrideVal = true;
		} break;
		case 2:
			invert = byte & 0b1; break;
		case 3:
			on = byte & 0b1;     break;
		
		/* scroll commands */
		case 4:
			break;

		case 5:
			break;
		case 6:
			break;
		case 7:
			break;
		case 8:
			break;

		// addressing commands
		case 9:
			break;
		case 10:
			break;
		case 11:
			break;
		case 12:
			break;
		case 13:
			break;
		case 14:
			break;

		case 15:
			startLineReg = byte & 0b111111;
			break;
		
		case 16:
			break;
		
		case 17:
			break;
		case 18:
			break;
		
		case 19:
			break;
		case 20:
			break;
		

		case 21:
			clockDevisor = parameterStack[0] & 0b1111;
			oscFreq = (parameterStack[0] & 0b11110000) >> 4;
			break;
		case 22:
			break;
		case 23:
			break;
		case 24:
			break;

		case 25:
			break;

		default:
			std::cout << "Unhandled Display Command: " << (int)byte << " ID: " << (int)id << std::endl;
			abort();
	}
}
void AB::Display::handleCurrentCommand() {
	handleCommand(currentCommandByte,currentCommandID);
}

uint8_t AB::Display::getCommandInd(uint8_t firstByte) {
	for (uint8_t i = 0; i < commandsLen; i++) {
		auto& c = commands[i];
		if ((firstByte & c.idMask) == c.id) {
			return i;
		}
	}
	return 0xFF;
}

void AB::Display::startRecivingParams() {
	isRecivingParameters = true;
	parameterStackPointer = 0;
}
void AB::Display::stopRecivingParams() {
	isRecivingParameters = false;
}


bool AB::Display::isDataMode() {
	return (mcu->dataspace.getDataByte(A32u4::DataSpace::Consts::PORTD) & (1<<4)) != 0;
}

void AB::Display::activate() {
	activeDisplay = this;
}
void AB::Display::update() {
	if (!on) {
		for (size_t i = 0; i < WIDTH * HEIGHT; i++) {
			pixels[i] = false;
		}
	}
	else if (screenOverride) {
		for (size_t i = 0; i < WIDTH * HEIGHT; i++) {
			pixels[i] = screenOverrideVal;
		}
	}
	else {
		for (uint8_t y = 0; y < HEIGHT / 8; y++) {
			for (uint8_t x = 0; x < WIDTH; x++) {
				for (uint8_t i = 0; i < 8; i++) {
					bool val = (pixelsRaw[y * WIDTH + x] & (1<<i)) != 0;
					if (invert)
						val = !val;
					getPixel(x, y * 8 + i) = val;
				}
			}
		}
	}

#if 0
	for(int y = 0; y<HEIGHT;y++){
		for(int x = 0; x<WIDTH;x++){
			int xC = x+((y/8)*WIDTH);
			int yC = (y%8);
			std::cout << (((pixelsRaw[xC]&(1<<yC))>0)?'#':' ');
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
#endif
}

bool& AB::Display::getPixel(uint8_t x, uint8_t y) {
	return (bool&)pixels[y * WIDTH + x];
}

AB::Display* AB::Display::activeDisplay;
void AB::Display::spiCallB(uint8_t data) {
	activeDisplay->reciveSPIByte(data);
}

void AB::Display::getState(std::ostream& output){
	output.write((const char*)&pixels[0], pixels.size());
	output.write((const char*)&pixelsRaw[0], pixelsRaw.size());

	output << on;
	output << screenOverride;
	output << screenOverrideVal;
	output << invert;

	output << startLineReg;
	output << clockDevisor;
	output << oscFreq;

	output << addrPtr;

	output.write((const char*)&parameterStack[0], parameterStack.size());
	output << parameterStackPointer;
	output << isRecivingParameters;

	output << currentCommandID;
	output << currentCommandByte;
}
void AB::Display::setState(std::istream& input){
	input.read((char*)&pixels[0], pixels.size());
	input.read((char*)&pixelsRaw[0], pixelsRaw.size());

	input >> on;
	input >> screenOverride;
	input >> screenOverrideVal;
	input >> invert;

	input >> startLineReg;
	input >> clockDevisor;
	input >> oscFreq;

	input >> addrPtr;

	input.read((char*)&parameterStack[0], parameterStack.size());
	input >> parameterStackPointer;
	input >> isRecivingParameters;

	input >> currentCommandID;
	input >> currentCommandByte;
}