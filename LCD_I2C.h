#ifndef I2C_LCD
#define I2C_LCD

#include <inttypes.h>

/**
 * Poor man's driver for I2C based 16x2 Liquid Crystal Displays.
 *
 * This implementation is meant for Hitachi HD44780 based LCDs that are connected via
 * I2C (using some http://www.ti.com/product/PCF8574 based adapter).
 *
 * It has been tested successfully with multiple of these modules (3v3 & 5V) that can be found
 * cheaply on AliExpress. The hard-coded pin mappings that it is using work for the modules that
 * I encountered so far but they might need to be adapted for other module versions.
 *
 * This library has not specifically been designed to be easily extensible/reusable for
 * different types of displays - nor does it support any of the LCD-features that I do not use
 * myself (e.g. scrolling, blinking, customized chars, etc). The no frills functionality is limited to
 * displaying characters within the fixed visible 2x16 area of the display.
 *
 * There are plenty of other more generic and also more functionally comprehensive LCD libraries available
 * and if you are happy with their respective licensing then you might want to check those out.
 *
 * Personally I prefer to NOT use anything that comes with a shitty GPL license as a base
 * for my work and I therefore decided to write my own MickyMouse LCD library instead. (I feel that it
 * would be stupid to let some trivial functionality library dictate the licenses that you can or
 * cannot use for your own work..)
 *
 * 	Copyright (C) 2018 Juergen Wothke
 *
 * Terms of Use: This software is licensed under a CC BY-NC-SA
 * (http://creativecommons.org/licenses/by-nc-sa/4.0/).
 */
class LCD_I2C {
public:
	/**
	* The LCD is initially turned "on" and back-light is "on" as well.
	*/
	LCD_I2C(uint8_t i2cAddr);

	/**
	* Clears the display and sets the cursor to position 0/0.
	*/
	void clear();

	/**
	* Controls the back-light.
	*/
	void backlightOn();
	void backlightOff();

	/**
	* Turns on/off the display.
	*/
	void displayOn();
	void displayOff();

	/**
	* Sets the cursor at the respective screen coordinate.
	*/
	void setCursor(uint8_t x, uint8_t y);

	/**
	 * Prints text to the LCD (at the current cursor position).
	 */
	void print(const char* msg);
	void printChar(uint8_t value);

private:
	void initHD44780();
	size_t initI2C();

	void writeI2C(uint8_t value);
	uint8_t mapPins(uint8_t value);
	void writeDataNibble(uint8_t value);
	void writeCmdNibble (uint8_t value);

	void sendCmd(uint8_t value);
	void sendCmdNibble(uint8_t value);

private:
	size_t	_isReady;
	uint8_t _width, _height;		// size of the LCD (in case of future extensions)

	uint8_t _i2cAddr;

	uint8_t _ctrlFlags;			// last used flags for CTRL command
	uint8_t _backlightStatus;

	uint8_t _enablePin;
	uint8_t _registerSelectPin;
	uint8_t _backlightPin;
	uint8_t _dataPin[4];
};

#endif
