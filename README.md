# Arduino serial terminal and tester for  AD7745, AD7746 AD7747 with precise i2c communication
Tested with Arduino IDE 1.8.10 and Arduino AVR Boards Built-In Arduino version 1.8.1                                                        
# Serial Terminal Commands:
  - the baud rate is 115200
  - number of digits for a command with parameter must be preserved, excess zeros must not be deleted
  - sent command must be terminated with codes 0x0D + 0x0A ie. CR + LF or \r\n, most PC terminals doing this by default
  - macros can be created if the PC terminal allows it,  but each command must be from the next command separated by codes 0x0D + 0x0A
  - registers 00 - 06 are read only
  - registers 15 - 18 are factory calibrated and they are not saved to flash memory
  - the sampling period value SamplePeriod (command pwdddd) is a program variable,
    the minimal sample period is determined by the convert time, set in the CONFIGURATION registry
  - signs in the following text: h = hexadeximal digit, d = decimal digit

1.    **FW**  writes current registers 07-14 to EEPROM memory as default after RESET / POR including CAP Offset register
2.  **NN** no operation - delay 250 ms
3.  **OO** not implemented now
4.   **PR**  prints the sampling period of the converter, in the decimal form dddd, the unit is time in [ms]
5.   **PWdddd** write setting the AD774X sampling period, dddd are four digits 0000-9999, the unit is time in [ms]
6.   **RR** prints all registers in the form "R00 = hh R01 = hh… R18 = hh R19 = hh"
7.   **RRdd** prints one register in the form "Rdd = hh"
8.   **RWddhh** writes to the register dd hexadecimal value hh, dd are two decimal digits in the range 00 - 18
9.   **SS** disables / enables periodic sampling of AD774X, default is OFF !!!
10.   **TT** reads the data registers once and displays them. Makes meaning only when sampling is stop.
11.   **XX** restart Arduino with SW reset AD774X including setting its default values

  
Example: 
 - _**rw10A2<CR+LF>**_                   write to configuration register number 10 value 0xA2
 - _**RW10a2<CR+LF>**_ is identical
 - _**pw0300<CR+LF>**_                   changes the sampling period to 300ms (only until RESET or POR)
 
Example makro:
 - _**SS<CR+LF>**_                       use it separately for stop sampling
 - _**RW10A2<CR+LF>NN<CR+LF>TT<CR+LF>**_ and then this as macro - write to configuration register number 10 value 0xA2(start single conversion), set delay 250ms and display recieved data
