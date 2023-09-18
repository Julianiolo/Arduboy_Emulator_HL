#include "Display.h"
#include <iostream>

#include "StreamUtils.h"
#include "DataUtils.h"

#define LU_MODULE "Display"

AB::Display::Display(A32u4::ATmega32u4* mcu) : mcu(mcu)
#if AB_USE_HEAP
,pixels(WIDTH*HEIGHT), pixelsRaw((WIDTH*HEIGHT)/8)
#endif
{
	setCallB();
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
			LU_LOGF(LogUtils::LogLevel_Error, "cant find display command id for: %" PRIu8, byte);
			return;
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
			LU_LOGF(LogUtils::LogLevel_Error, "Unhandled Display Command: %" PRIu8 " ID: %" PRIu8, byte, id);
			return;
	}
}
void AB::Display::handleCurrentCommand() {
	handleCommand(currentCommandByte,currentCommandID);
}

uint8_t AB::Display::getCommandInd(uint8_t firstByte) {
#if 0
	for (uint8_t i = 0; i < commands.size(); i++) {
		auto& c = commands[i];
		if ((firstByte & c.idMask) == c.id) {
			return i;
		}
	}
#else
	constexpr size_t problemCmdInd = 15; // only command which only has first 2 bits constant
	constexpr auto problemCmd = commands[problemCmdInd];
	DU_STATIC_ASSERT(problemCmd.idMask == 0xC0);

	DU_IF_UNLIKELY((firstByte & problemCmd.idMask) == problemCmd.id) {
		return problemCmdInd;
	}

	switch (firstByte & 0xf0) {
		case 0b0000 << 4: return 9;
		case 0b0001 << 4: return 10;
		case 0b0010 << 4:
			switch (firstByte & 0xf) {
				case 0b0000: return 11;
				case 0b0001: return 12;
				case 0b0010: return 13;
				case 0b0110:
				case 0b0111:
					return 4;
				case 0b1000:
				case 0b1001:
				case 0b1010:
				case 0b1011:
					return 5;
				case 0b1110:
					return 6;
				case 0b1111:
					return 7;
				default: return 0xff;
			}
		case 0b1000 << 4:
			switch (firstByte & 0xf) {
				case 0b0001: return 0;
				case 0b1101: return 25;
				default: return 0xff;
			}
		case 0b1010 << 4:
			switch (firstByte & 0b1110) {
				case 0b000 << 1: return 16;
				case 0b001 << 1: return 8;
				case 0b010 << 1: return 1;
				case 0b011 << 1: return 2;
				case 0b111 << 1: return 3;
				case 0b100 << 1: return 17;
				default: return 0xff;
			}
		case 0b1100 << 4: return 18;
		case 0b1101 << 4:
			switch (firstByte & 0xf) {
				case 0b0011: return 19;
				case 0b1010: return 20;
				case 0b0101: return 21;
				case 0b1001: return 22;
				case 0b1011: return 23;
				default: return 0xff;
			}
		case 0b1110 << 4: return 24;
		default: return 0xff;
	}
#endif
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

void AB::Display::setCallB() {
	mcu->dataspace.setSPIByteCallB([=](uint8_t data){ reciveSPIByte(data);});
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

void AB::Display::getState(std::ostream& output){
	output.write((const char*)&pixels[0], pixels.size());
	output.write((const char*)&pixelsRaw[0], pixelsRaw.size());

	StreamUtils::write(output, on);
	StreamUtils::write(output, screenOverride);
	StreamUtils::write(output, screenOverrideVal);
	StreamUtils::write(output, invert);

	StreamUtils::write(output, startLineReg);
	StreamUtils::write(output, clockDevisor);
	StreamUtils::write(output, oscFreq);

	StreamUtils::write(output, addrPtr);

	output.write((const char*)&parameterStack[0], parameterStack.size());
	StreamUtils::write(output, parameterStackPointer);
	StreamUtils::write(output, isRecivingParameters);

	StreamUtils::write(output, currentCommandID);
	StreamUtils::write(output, currentCommandByte);
}
void AB::Display::setState(std::istream& input){
	input.read((char*)&pixels[0], pixels.size());
	input.read((char*)&pixelsRaw[0], pixelsRaw.size());

	StreamUtils::read(input, &on);
	StreamUtils::read(input, &screenOverride);
	StreamUtils::read(input, &screenOverrideVal);
	StreamUtils::read(input, &invert);

	StreamUtils::read(input, &startLineReg);
	StreamUtils::read(input, &clockDevisor);
	StreamUtils::read(input, &oscFreq);

	StreamUtils::read(input, &addrPtr);

	input.read((char*)&parameterStack[0], parameterStack.size());
	StreamUtils::read(input, &parameterStackPointer);
	StreamUtils::read(input, &isRecivingParameters);

	StreamUtils::read(input, &currentCommandID);
	StreamUtils::read(input, &currentCommandByte);
}

bool AB::Display::operator==(const Display& other) const{
#define _CMP_(x) (x==other.x)
	return _CMP_(pixels) && _CMP_(pixelsRaw) &&
		_CMP_(on) && _CMP_(screenOverride) && _CMP_(screenOverrideVal) && _CMP_(invert) &&
		_CMP_(startLineReg) && _CMP_(clockDevisor) && _CMP_(oscFreq) &&
		_CMP_(addrPtr) && 
		_CMP_(parameterStack) && _CMP_(parameterStackPointer) && _CMP_(isRecivingParameters) && 
		_CMP_(currentCommandID) && _CMP_(currentCommandByte);
#undef _CMP_
}

size_t AB::Display::sizeBytes() const {
	size_t sum = 0;

	sum += sizeof(mcu);
	
	sum += WIDTH * HEIGHT;
	sum += (WIDTH * HEIGHT) / 8;
#if AB_USE_HEAP
	sum += sizeof(pixels);
	sum += sizeof(pixelsRaw);
#endif

	sum += sizeof(on);
	sum += sizeof(screenOverride);
	sum += sizeof(screenOverrideVal);
	sum += sizeof(invert);

	sum += sizeof(startLineReg);
	sum += sizeof(clockDevisor);
	sum += sizeof(oscFreq);

	sum += sizeof(addrPtr);

	sum += sizeof(parameterStack);
	sum += sizeof(parameterStackPointer);
	sum += sizeof(isRecivingParameters);

	sum += sizeof(currentCommandID);
	sum += sizeof(currentCommandByte);

	return sum;
}

uint32_t AB::Display::hash() const noexcept {
	uint32_t h = 0;

	DU_HASHCC(h, pixels);
	DU_HASHCC(h, pixelsRaw);

	DU_HASHC(h, on);
	DU_HASHC(h, screenOverride);
	DU_HASHC(h, screenOverrideVal);
	DU_HASHC(h, invert);

	DU_HASHC(h, startLineReg);
	DU_HASHC(h, clockDevisor);
	DU_HASHC(h, oscFreq);

	DU_HASHC(h, addrPtr);

	DU_HASHCC(h, parameterStack);
	DU_HASHC(h, parameterStackPointer);
	DU_HASHC(h, isRecivingParameters);

	DU_HASHC(h, currentCommandID);
	DU_HASHC(h, currentCommandByte);

	return h;
}

