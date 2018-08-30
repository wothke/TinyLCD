#include <Arduino.h>
#include <Wire.h>

#include "LCD_I2C.h"

// note: the implementation completely ignores the "busy flag" of the LCD and new commands
// may be sent before previous ones have been handled. If ever this leads to any problems
// "delay()" calls might be added between commands.

// see "Instructions" on page 24 of https://www.sparkfun.com/datasheets/LCD/HD44780.pdf
// note: the most significant "set-bit" selects command (optional flag-bits then follow)

#define CMD_CLEAR			0x01		/* Clear display */
#define CMD_HOME          	0x02		/* Return home */
#define CMD_EM				0x04		/* Entry mode set */
#define CMD_CTRL			0x08		/* Display on/off control */
#define CMD_SHIFT			0x10		/* Cursor or display shift */
#define CMD_FUNC			0x20		/* Function set */
#define CMD_SET_CGRAM		0x40		/* Set CGRAM address */
#define CMD_SET_DDRAM		0x80		/* Set DDRAM address */

// below constants come in pairs that refer to the same flag-bit (set bit/cleared bit)
// CAUTION: the "cleared bit" variants serve for documentation purposes only and if a
// previously set bit needs to be actually cleared then the "set bit" variant must be used!

	// flags specific to "Entry mode set":
#define EM_DO_SHIFT_DISPLAY		0x01
#define EM_DONT_SHIFT_DISPLAY	0x00	// dummy for documentation
#define EM_INCREMENT			0x02
#define EM_DECREMENT			0x00

	// flags specific to "Display on/off control":
#define CTRL_BLINK_ON			0x01
#define CTRL_BLINK_OFF			0x00	// dummy for documentation
#define CTRL_CURSOR_ON			0x02
#define CTRL_CURSOR_OFF			0x00	// dummy for documentation
#define CTRL_DISPLAY_ON			0x04
#define CTRL_DISPLAY_OFF		0x00	// dummy for documentation

	// flags specific to "Cursor or display shift":
#define SHIFT_RIGHT           0x04
#define SHIFT_LEFT            0x00
#define SHIFT_DISPLAY         0x08
#define SHIFT_CURSOR          0x00

// flags specific to "Function set":
#define FUNC_5x10		0x04
#define FUNC_5x8		0x00
#define FUNC_2LINES		0x08
#define FUNC_1LINE		0x00
#define FUNC_8BIT		0x10
#define FUNC_4BIT		0x00


#define MODE_4BIT		(CMD_FUNC >> 4)
#define MODE_8BIT		((CMD_FUNC | FUNC_8BIT) >> 4)



// make sure slow command gets enough time (1.52ms would be enough for "Return home" but "clear" must be more expensive)
#define CLEAR_WAIT_MS      2


LCD_I2C::LCD_I2C(uint8_t i2cAddr) {
	_isReady= false;

	_i2cAddr= i2cAddr;

	_width= 16;
	_height = 2;

	// the below mapping works fine for the cheap models from AliExpress
	// (but it might need to be adapted for other LCD models)

	_registerSelectPin= (1 << 0);
	_enablePin= 		(1 << 2);
	_backlightPin= 		(1 << 3);
	_dataPin[0]= 		(1 << 4);
	_dataPin[1]= 		(1 << 5);
	_dataPin[2]= 		(1 << 6);
	_dataPin[3]= 		(1 << 7);

	initHD44780();
}

void LCD_I2C::initHD44780() {
	// for start sequence: see https://www.sparkfun.com/datasheets/LCD/HD44780.pdf - see page 23:
	// initial state
	//	1. Display clear
	//	2. Function set:
	//	DL = 1; 8-bit interface data
	//	N = 0; 1-line display
	//	F = 0; 5 × 8 dot character font
	//	3. Display on/off control:
	//	D = 0; Display off
	//	C = 0; Cursor off
	//	B = 0; Blinking off
	//	4. Entry mode set:
	//	I/D = 1; Increment by 1
	//	S = 0; No shift

	if (initI2C()) {
		_backlightStatus= 0;	// default: off

		// "When the power is turned on, 8-bit operation is automatically selected and the first write is
		// performed as an 8-bit operation. .." (page 39)
		//see example from page 46:

		delay (150); 	// who knows at what voltage the (esp8266, etc) device might already power on - and how far that
						// might be from 2.7V - or how the 3v3 to 5v conversion might interfere for 3v3 version..
						// so lets just wait some.. 150ms seems to work well enough

		sendCmdNibble(MODE_8BIT);
		delayMicroseconds(4200);	// wait more than 4.1 ms

		sendCmdNibble(MODE_8BIT);
		delayMicroseconds(110);		// wait more than 100 us

		sendCmdNibble(MODE_8BIT);
		delayMicroseconds(110);		// this wait may not be necessary

		sendCmdNibble(MODE_4BIT);	// change to 4-bit mode


		// perform actual settings (from here on use regular 8-bit sendCmd):
		sendCmd(CMD_FUNC | FUNC_4BIT | FUNC_2LINES | FUNC_5x8);	// The number of display lines and character
																// font cannot be changed after this point

		// display off
		_ctrlFlags = CTRL_CURSOR_OFF | CTRL_BLINK_OFF;
		displayOff();

		// display clear
		clear();

		// entry mode set
		sendCmd(CMD_EM | EM_INCREMENT | EM_DONT_SHIFT_DISPLAY);	// use fixed text direction

		// standard initialization ends here (see page 46)


		// turn display on
		backlightOn();
		displayOn();

	} else {
		// error - should be signaled
	}
}

void LCD_I2C::clear() {
	sendCmd(CMD_CLEAR);
	delay(CLEAR_WAIT_MS);
}

void LCD_I2C::setCursor(uint8_t x, uint8_t y) {
	// basic sanity checks
	if ( x >= _width ) { x = _width-1; }
	if ( y >= _height ) { y = _height-1; }

	sendCmd(CMD_SET_DDRAM | (x + y * 0x40));	// note: displays with more than 2 lines would need to be handled differently
}

void LCD_I2C::backlightOn() {
	// _backlightStatus is sent with every command/data..
	_backlightStatus= _backlightPin;

	sendCmd(CMD_CTRL | _ctrlFlags);	// any command would do to propagate the updated back-light
}

void LCD_I2C::backlightOff() {
	_backlightStatus= 0;

	sendCmd(CMD_CTRL | _ctrlFlags);	// any command would do to propagate the updated back-light
}

void LCD_I2C::displayOn() {
	_ctrlFlags |= CTRL_DISPLAY_ON;
	sendCmd(CMD_CTRL | _ctrlFlags);
}

void LCD_I2C::displayOff() {
	_ctrlFlags &= ~CTRL_DISPLAY_ON;
	sendCmd(CMD_CTRL | _ctrlFlags);
}

// ------------------------ internal utilities ---------------------------------

void LCD_I2C::writeDataNibble(uint8_t value) {
	// in present mapping configuration the high-nibble contains the 4-data bits
	// whereas all the flags happen in the low-nibble

	uint8_t mapped= mapPins(value)
			| _registerSelectPin	// flag "data" (as opposed to "commands")
			| _backlightStatus;		// flag back-light

	// write by sending pulse to enable line
	writeI2C(mapped | _enablePin);
	writeI2C(mapped & ~_enablePin);
}

void LCD_I2C::writeCmdNibble(uint8_t value) {
	uint8_t mapped= mapPins(value) | _backlightStatus;

	// write by sending pulse to enable line
	writeI2C(mapped | _enablePin);
	writeI2C(mapped & ~_enablePin);
}

void LCD_I2C::sendCmdNibble(uint8_t value) {
	writeCmdNibble((value & 0xF));
}

void LCD_I2C::sendCmd(uint8_t value) {
	writeCmdNibble(value >> 4);
	writeCmdNibble(value & 0xF);
}

uint8_t LCD_I2C::mapPins(uint8_t value) {
	uint8_t mapped = 0;
	for (uint8_t i = 0; i < 4; i++) {
		if (value & 0x1) {
			mapped |= _dataPin[i];
		}
		value >>= 1;
	}
	return mapped;
}

void LCD_I2C::printChar(uint8_t value) {
	writeDataNibble(value >> 4);
	writeDataNibble(value & 0x0F);
}

void LCD_I2C::print(const char* msg) {
	for (uint8_t i= 0; i<strlen(msg); i++) {
		printChar((uint8_t)msg[i]);
	}
}

size_t LCD_I2C::initI2C() {
	Wire.begin();

	// unfortunately old RTC only supports 100kHz so I cannot do this on old HW
#ifdef ARDUINO_ESP8266_WEMOS_D1MINI
	// for some reason EEPROM does not seem to work with 100kHz on D1Mini
	Wire.setClock(400000L);			// 400kHz
#endif

	_isReady= Wire.requestFrom(_i2cAddr, (uint8_t)1);	// request 1 byte
	Wire.read();	// discard requested byte

	if (_isReady) {
		writeI2C(0);
	}

	return _isReady;
}

void LCD_I2C::writeI2C(uint8_t value) {
	if (_isReady) {
		Wire.beginTransmission(_i2cAddr);
		Wire.write(value);

		// the delays seem to help to avoid display corruption (experienced on ATmega128)
		delayMicroseconds(2);
		Wire.endTransmission();	// todo: add error handling? (0= success)
		delayMicroseconds(2);
	}
}
