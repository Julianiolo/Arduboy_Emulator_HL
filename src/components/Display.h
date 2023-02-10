#ifndef _ARDUBOY_DISPLAY
#define _ARDUBOY_DISPLAY

#include <stdint.h>
#include <array>
#include <vector>

#include "../config.h"

#include "ATmega32u4.h"

namespace AB {
	class Display { //mimics the SSD1306 https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf
	public:
		static constexpr uint8_t WIDTH = 128;
		static constexpr uint8_t HEIGHT = 64;
	private:
		A32u4::ATmega32u4* mcu;

#if !AB_USE_HEAP
		std::array<uint8_t,WIDTH*HEIGHT> pixels;
		std::array<uint8_t,(WIDTH*HEIGHT)/8> pixelsRaw;
#else
		std::vector<uint8_t> pixels;
		std::vector<uint8_t> pixelsRaw;
#endif
		bool on = false;
		bool screenOverride = false;
		bool screenOverrideVal = false;
		bool invert = false;

		uint8_t startLineReg = 0;
		uint8_t clockDevisor = 1;
		uint8_t oscFreq = 0b1000;

		uint16_t addrPtr = 0;

		struct Command {
			uint8_t id;
			uint8_t idMask;
			uint8_t paramAmt;
		};

		static constexpr std::array<Command,26> commands = {
			// fundamental commands
			Command{0b10000001,0xFF,1}, // set contrast control
			Command{0b10100100,0xFE,0}, // entire display on
			Command{0b10100110,0xFE,0}, // set normal/inverse display
			Command{0b10101110,0xFE,0}, // set display on/off
			// scrolling commands
			Command{0b00100110,0xFE,6}, // continuous horizontal scroll setup
			Command{0b00101000,0xFC,5}, // continuous vertical and horizontal scroll setup
			Command{0b00101110,0xFF,0}, // disable scroll
			Command{0b00101111,0xFF,0}, // activate scroll
			Command{0b10100011,0xFF,2}, // set vertical scroll area
			// addressing settings
			Command{0b00000000,0xF0,0}, // set lower column start address for page addressing mode
			Command{0b00010000,0xF0,0}, // set higher column start address for page addressing mode
			Command{0b00100000,0xF0,1}, // set memory addressing mode
			Command{0b00100001,0xFF,2}, // set column adress
			Command{0b00100010,0xFF,2}, // set page address
			Command{0b10110000,0xF8,0}, // set page start address for page addressing mode
			// hardware configuration
			Command{0b01000000,0xC0,0}, // set display start size
			Command{0b10100000,0xFE,0}, // set segment re-map
			Command{0b10101000,0xFF,1}, // set multiplex ratio
			Command{0b11000000,0xF7,0}, // set com output scan direction
			Command{0b11010011,0xFF,1}, // set display offset
			Command{0b11011010,0xFF,0}, // set com pins hardware configuration
			// timing & driving scheme settings
			Command{0b11010101,0xFF,1}, // set display clock divide ratio/oscillator frequency
			Command{0b11011001,0xFF,1}, // set pre-charge period
			Command{0b11011011,0xFF,1}, // set Vcomh Deselect Level
			Command{0b11100011,0xFF,0}, // NOP

			Command{0b10001101,0xFF,1}, // charge pump regulator
		};

		

		std::array<uint8_t,10> parameterStack;
		uint8_t parameterStackPointer = 0;
		uint8_t isRecivingParameters = false;

		uint8_t currentCommandID = -1;
		uint8_t currentCommandByte = -1;

		void reciveSPIByte(uint8_t byte);
		void reciveCommandByte(uint8_t byte);
		void reciveDataByte(uint8_t byte);
		bool isDataMode();

		void handleCommand(uint8_t byte, uint8_t id = -1);
		void handleCurrentCommand();
		
		uint8_t getCommandInd(uint8_t firstByte);
		void startRecivingParams();
		void stopRecivingParams();
	public:
		Display(A32u4::ATmega32u4* mcu);

		void reset();

		void activate();
		void update();

		bool& getPixel(uint8_t x, uint8_t y);

		static Display* activeDisplay;
		static void spiCallB(uint8_t data);

		void getState(std::ostream& output);
		void setState(std::istream& input);

		bool operator==(const Display& other) const;
	};
}

#endif