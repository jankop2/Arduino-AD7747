# Arduino serial terminal and tester for  AD7745, AD7746 AD7747 with precise i2c communication and automatic offset adjustment
Tested with Arduino IDE 1.8.10 and Arduino AVR Boards Built-In Arduino version 1.8.1 
This text is valid for software version V1.4
# Serial Terminal Commands:
  - the baud rate is 115200
  - number of digits for a command with parameter must be preserved, excess zeros must not be deleted
  - sent command must be terminated with codes 0x0D + 0x0A ie. CR + LF or \r\n, most PC terminals doing this by default
    (Arduino Serial Monitor must be set to "Both NL&CR")
  - macros can be created if the PC terminal allows it,  but each command must be from the next command separated by codes 0x0D + 0x0A
  - registers 00 - 06 are read only, registers 15 - 18 are factory calibrated and they are not saved to flash memory
  - the sampling period value SamplePeriod (command pwdddd) is a program variable,
    the minimal sample period is determined by the convert time, set in the CONFIGURATION registry
  - signs in the following text: h = hexadeximal digit, d = decimal digit

1. **FW** writes current registers 07-14 to EEPROM memory as default after RESET / POR including CAP Offset register
2. **FR** clears the EEPROM memory area in use, registers 07-14 are initialized from PROGMEM after RESET / POR
3. **NN** no operation - delay 250 ms
4. **OO** automatic offset compensation
5. **OR** clear offset compensation (CAPDACs = off, OFFSET = 0x8000 the middle of the interval)
6. **PR** prints the sampling period of the converter, in the decimal form dddd, the unit is time in [ms]
7. **PWdddd** write setting the AD774X sampling period, dddd are four digits 0000-9999, the unit is time in [ms]
8. **RR** prints all registers in the form "R00 = hh R01 = hh… R18 = hh"
9. **RRdd** prints one register in the form "Rdd = hh"
10. **RWddhh** writes to the register dd hexadecimal value hh, dd are two decimal digits in the range 00 - 18
11. **SS** disables / enables periodic sampling of AD774X, default is OFF !!!
12. **TT** reads the data registers once and displays them. Makes meaning only when sampling is stop.
13. **VV** displays the version of this software
14. **XX** restart Arduino with SW reset AD774X including setting its default values

  
Example: 
 - _**rw10A2<CR+LF>**_                   write to configuration register number 10 value 0xA2
 - _**RW10a2<CR+LF>**_ is identical
 - _**pw0300<CR+LF>**_                   changes the sampling period to 300ms (only until RESET or POR)
 
Example makro:
 - _**SS<CR+LF>**_                       use it separately for stop sampling
 - _**RW10A2<CR+LF>NN<CR+LF>TT<CR+LF>**_ and then this as macro - write to configuration register number 10 value 0xA2(start single conversion), set delay 250ms and display recieved data
