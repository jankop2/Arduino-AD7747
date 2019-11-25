
/********************** The automatic offset adjustment ************
   The automatic offset adjustment, it is command "oo", works only for the ranges
   0 - 8.192pF (0 - 4.096pF for AD7745/46) as CDC single-ended input configuration
   and +- 8.192pF (+- 4.096pF for AD7745/46) as CDC differential input configuration.
   For other ranges it is necessary to set the offset manually, because a universal
   algorithm would be unnecessarily complex.
   In the case of  AD7746, only the currently active channel is balanced.
   Be sure to make the basic settings in the AD774X_Init section.
   The message "AD774X not responding !" indicates that the AD774X probably not
   connected properly.
   
********************** Serial Terminal Commands *********************
  - the baud rate is 115200
  - number of digits for a command with parameter must be preserved, excess zeros must not be deleted
  - sent command must be terminated with codes 0x0D + 0x0A ie. CR + LF or \r\n,
    most PC terminals doing this by default (Arduino Serial Monitor must be set to "Both NL&CR")
  - macros can be created if the PC terminal allows it, but each command must be from the next command
    separated by codes 0x0D + 0x0A
  - registers 00 - 06 are read only and registers 15 - 18 are factory calibrated, they are not saved to flash memory
  - the sampling period variable SamplePeriod (command pwdddd) is a program variable. Minimal possible interval is determined
    by conversion time, set in CONFIGURATION registers. The SamplingPeriod variable is also stored in flash memory by the FW command.
  
          Signs in the following text: h = hexadeximal digit, d = decimal digit

  fw     FLASH WRITE      writes current registers 07-14 and variable SamplePeriod to EEPROM memory as default after RESET / POR
  fr     FLASH RESET      clears the EEPROM memory area in use, registers 07-14 are initialized from PROGMEM after RESET / POR
  nn     NOP              no operation - delay 250 ms
  oo     OFFSET           automatic offset compensation
  or     OFFSET RESET     clear offset compensation (CAPDACs = off, OFFSET = 0x8000 the middle of the interval)
  pr     PERIODE READ     prints the sampling period of the converter, in the decimal form dddd, the unit is time in [ms]
  pwdddd PERIODE WRITE    write setting the AD774X sampling period, dddd are four digits 0000-9999, the unit is time in [ms]
  rr     REGISTERS READ   prints all registers in the form "R00 = hh R01 = hh… R18 = hh"
  rrdd   REGISTER READ    prints one register in the form "Rdd = hh"
  rwddhh REGISTER WRITE   writes to the register dd hexadecimal value hh, dd are two decimal digits in the range 00 - 18
  ss     SAMPLE STOP      disables / enables periodic sampling of AD774X and printing of data, default is OFF !!!
  tt     TEST THEM        reads the data registers once and displays them. Makes meaning only when sampling is stop.
  vv     VERSION          displays the version of this software
  xx     XINDL            restart Arduino with SW reset AD774X including setting its default values

  Example:
  rw10A2<CR+LF>                   write to configuration register number 10 value 0xA2
                                  identical is RW10a2<CR+LF>
  pw0300<CR+LF>                   changes the sampling period to 300ms

  Example makro:
  SS<CR+LF>                       use it separately for stop sampling
  RW10A2<CR+LF>NN<CR+LF>TT<CR+LF> and then this as macro - write to configuration
                                  register number 10 value 0xA2(start single conversion),
                                  set delay 250ms for conversion and display recieved data
*/
