# TinyLCD

Poor man's driver for I2C based 16x2 Liquid Crystal Displays.

This implementation is meant for Hitachi HD44780 based LCDs that are connected via
I2C (using some http://www.ti.com/product/PCF8574 based adapter):

![alt text](https://github.com/wothke/TinyLCD/raw/master/docs/16x2LCD.jpg "I2C 16x2")


It has been tested successfully with multiple of these modules (3v3 and 5V) that can be found
cheaply on AliExpress (ca. 2$ per piece). The hard-coded pin mappings that it is using do 
work for the modules that I encountered so far but they might need to be adapted for other 
module versions.

This library has not specifically been designed to be easily extensible/reusable for
different types of displays - nor does it support any of the LCD-features that I do not use
myself (e.g. scrolling, blinking, customized chars, etc). The no frills functionality is limited to
displaying characters within the fixed visible 16x2 area of the display.

There are plenty of other more generic and also more functionally comprehensive LCD libraries available
and if you are happy with their respective licensing then you might want to check those out.

Personally I prefer to NOT use anything that comes with a shitty GPL license as a base
for my work. (I don't like GPL for the same reason that I don't hand out blanko checks.)
Since I didn't find anything acceptable, I decided to write my own MickyMouse LCD library 
instead. (I feel that it would be stupid to let some trivial functionality library - 
like this one - dictate the licenses that you can or cannot use for your own work..)

Copyright (C) 2018 Juergen Wothke

## License
Terms of Use: This software is licensed under a CC BY-NC-SA (http://creativecommons.org/licenses/by-nc-sa/4.0/)
	