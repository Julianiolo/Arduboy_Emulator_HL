#ifndef _ARDUBOY_DISPLAY
#define _ARDUBOY_DISPLAY

#include <stdint.h>
#include <array>
#include <vector>

#include "../config.h"

#include "ATmega32u4.h"

class Arduboy;

namespace AB {
	class Display { //mimics the SSD1306 https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf
	public:
		static constexpr uint8_t WIDTH = 128;
		static constexpr uint8_t HEIGHT = 64;
	private:
		friend Arduboy;
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
			Command{0b10000001,0xFF,1}, //  0: set contrast control
			Command{0b10100100,0xFE,0}, //  1: entire display on
			Command{0b10100110,0xFE,0}, //  2: set normal/inverse display
			Command{0b10101110,0xFE,0}, //  3: set display on/off
			// scrolling commands
			Command{0b00100110,0xFE,6}, //  4: continuous horizontal scroll setup
			Command{0b00101000,0xFC,5}, //  5: continuous vertical and horizontal scroll setup
			Command{0b00101110,0xFF,0}, //  6: disable scroll
			Command{0b00101111,0xFF,0}, //  7: activate scroll
			Command{0b10100011,0xFF,2}, //  8: set vertical scroll area
			// addressing settings
			Command{0b00000000,0xF0,0}, //  9: set lower column start address for page addressing mode
			Command{0b00010000,0xF0,0}, // 10: set higher column start address for page addressing mode
			Command{0b00100000,0xFF,1}, // 11: set memory addressing mode
			Command{0b00100001,0xFF,2}, // 12: set column adress
			Command{0b00100010,0xFF,2}, // 13: set page address
			Command{0b10110000,0xF8,0}, // 14: set page start address for page addressing mode
			// hardware configuration
			Command{0b01000000,0xC0,0}, // 15: set display start size
			Command{0b10100000,0xFE,0}, // 16: set segment re-map
			Command{0b10101000,0xFF,1}, // 17: set multiplex ratio
			Command{0b11000000,0xF7,0}, // 18: set com output scan direction
			Command{0b11010011,0xFF,1}, // 19: set display offset
			Command{0b11011010,0xFF,0}, // 20: set com pins hardware configuration
			// timing & driving scheme settings
			Command{0b11010101,0xFF,1}, // 21: set display clock divide ratio/oscillator frequency
			Command{0b11011001,0xFF,1}, // 22: set pre-charge period
			Command{0b11011011,0xFF,1}, // 23: set Vcomh Deselect Level
			Command{0b11100011,0xFF,0}, // 24: NOP

			Command{0b10001101,0xFF,1}, // 25:  charge pump regulator
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

		void setCallB();
		void update();

		bool& getPixel(uint8_t x, uint8_t y) const;

		void getState(std::ostream& output);
		void setState(std::istream& input);

		bool operator==(const Display& other) const;
		size_t sizeBytes() const;
		uint32_t hash() const noexcept;
	};
}

namespace DataUtils {
	inline size_t approxSizeOf(const AB::Display& v) {
		return v.sizeBytes();
	}
}

template<>
struct std::hash<AB::Display>{
	inline std::size_t operator()(const AB::Display& v) const noexcept{
		return (size_t)v.hash();
	}
};

#endif