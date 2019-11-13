
/********************** Serial Terminal Commands *********************
  - the baud rate is 115200
  - number of digits for a command with parameter must be preserved,
    excess zeros must not be deleted
  - sent command must be terminated with codes 0x0D + 0x0A ie.
    CR + LF or \r\n, most PC terminals doing this by default
  - macros can be created if the PC terminal allows it,
    but each command must be from the next command separated
    by codes 0x0D + 0x0A
  - registers 00 - 06 are read only
  - registers 15 - 18 are factory calibrated and they are not saved to flash memory
  - the sampling period value SamplePeriod (command pwdddd) is a program variable,
    the minimal sample period is determined by the convert time, set in the CONFIGURATION registry
  - signs in the following text: h = hexadeximal digit, d = decimal digit

  fw     FLASH WRITE      writes current registers 07-14 to EEPROM memory as default after RESET / POR including CAP Offset register
  nn     NOP              no operation - delay 250 ms
  oo     OFFSET           not implemented now
  pr     PERIODE READ     prints the sampling period of the converter, in the decimal form dddd, the unit is time in [ms]
  pwdddd PERIODE WRITE    write setting the AD774X sampling period, dddd are four digits 0000-9999, the unit is time in [ms]
  rr     REGISTERS READ   prints all registers in the form "R00 = hh R01 = hh… R18 = hh"
  rrdd   REGISTER READ    prints one register in the form "Rdd = hh"
  rwddhh REGISTER WRITE   writes to the register dd hexadecimal value hh, dd are two decimal digits in the range 00 - 18
  ss     SAMPLE STOP      disables / enables periodic sampling of AD774X, default is OFF !!!
  tt     TEST THEM        reads the data registers once and displays them. Makes meaning only when sampling is stop.
  xx     XINDL            restart Arduino with SW reset AD774X including setting its default values
  
  Example: 
  rw10A2<CR+LF>                   write to configuration register number 10 value 0xA2
                                  identical is RW10a2<CR+LF>
  pw0300<CR+LF>                   changes the sampling period to 300ms (only until
                                  RESET or POR)
  Example makro:
  SS<CR+LF>                       use it separately for stop sampling
  RW10A2<CR+LF>NN<CR+LF>TT<CR+LF> and then this as macro - write to configuration
                                  register number 10 value 0xA2(start single conversion),
                                  set delay 250ms and display recieved data
*/
//----------------------------------------------------------------------
// function for communication via serial port,
// it allows to write and read AD774X registers
//----------------------------------------------------------------------
void SerialTerminal(void) {
  // ? are the data in the serial buffer?
  if (Serial.available()) {
    // rotate the input serial parse buffer to the left by 1 character
    // and discard first character
    for (uint8_t i = 0; i < SxBuffLength; i++) {
      SxBuff[i] = SxBuff[i + 1];
    }
    // read one character into the buffer
    SxBuff[SxBuffLength - 1] = Serial.read();
    //----------------------------------------------------------------------
    //  ? are the termination characters 0x0d + 0x0a at the end of the buffer ?
    //----------------------------------------------------------------------
    if (SxBuff[SxBuffLength - 2] == 0x0D && SxBuff[SxBuffLength - 1] == 0x0A) {

      //----------------------------------------------------------------------
      // write registers to flash
      //----------------------------------------------------------------------
      if ((SxBuff[SxBuffLength - 4] == 'f' || SxBuff[SxBuffLength - 4] == 'F') && (SxBuff[SxBuffLength - 3] == 'w' || SxBuff[SxBuffLength - 3] == 'W')) {
        AD774X_Read_Registers(ADR_CAP_SETUP, RTxBuff, 8);
        for (int8_t i = ADR_CAP_SETUP; i < ADR_CAP_GAINH; i++) {
          EEPROM.update(i, RTxBuff[i]);
        }
        // 0xAA is flag for default setting AD774X from EEPROM
        EEPROM.update(0, 0xAA);
        Serial.print(F("\r\nRegisters 07-14 are written to flash memory as default"));
      }
      //----------------------------------------------------------------------
      // Automatic capacity channel offset adjustment
      //----------------------------------------------------------------------
      if ((SxBuff[SxBuffLength - 4] == 'o' || SxBuff[SxBuffLength - 4] == 'O' ) && (SxBuff[SxBuffLength - 3] == 'o' || SxBuff[SxBuffLength - 3] == 'O')) {
        Serial.print(F("\r\nNot implemented now Automatic capacity channel offset adjustment"));
      }
      //----------------------------------------------------------------------
      // read the sampling period
      //----------------------------------------------------------------------
      if ((SxBuff[SxBuffLength - 4] == 'p' || SxBuff[SxBuffLength - 4] == 'P') && (SxBuff[SxBuffLength - 3] == 'r' || SxBuff[SxBuffLength - 3] == 'R')) {
        Serial.print(F("\r\nSampling period [ms]: "));
        Serial.print(SamplePeriod);
      }
      //----------------------------------------------------------------------
      // write the sampling period in [ms]
      //----------------------------------------------------------------------
      if ((SxBuff[SxBuffLength - 8] == 'p' || SxBuff[SxBuffLength - 8] == 'P') && (SxBuff[SxBuffLength - 7] == 'w' || SxBuff[SxBuffLength - 7] == 'W')) {
        SamplePeriod = 0;
        for (uint8_t i = SxBuffLength - 6; i < (SxBuffLength - 2); i++) {
          SamplePeriod = 10 * SamplePeriod + map(SxBuff[i], 48, 57, 0, 9);
        }
        if (SamplePeriod > 9999) {
          SamplePeriod = 1000;
          Serial.print(F("\r\nThe sampling period is too long!"));
        }
        Serial.print(F("\r\nThe sampling period is written [ms]:"));
        Serial.print(SamplePeriod);
      }
      //----------------------------------------------------------------------
      // listing of all registers
      //----------------------------------------------------------------------
      if ((SxBuff[SxBuffLength - 4] == 'r' || SxBuff[SxBuffLength - 4] == 'R') && (SxBuff[SxBuffLength - 3] == 'r' || SxBuff[SxBuffLength - 3] == 'R')) {
        AD774X_Read_Registers(ADR_STATUS, RTxBuff, 19);
        Serial.println(F("\r\nList all registers 00 - 18:"));
        for (uint8_t i = ADR_STATUS; i < 19; i++) {
          Serial.print(F("R"));
          if (i < 10)Serial.print(F("0"));
          Serial.print(i);
          Serial.print(F("="));
          if (RTxBuff[i] < 0x10)Serial.print(F("0"));
          Serial.print(RTxBuff[i], HEX);
          Serial.print(F(" "));
        }
      }
      //----------------------------------------------------------------------
      // read one register AD774X
      //----------------------------------------------------------------------
      if ((SxBuff[SxBuffLength - 6] == 'r' || SxBuff[SxBuffLength - 6] == 'R') && (SxBuff[SxBuffLength - 5] == 'r' || SxBuff[SxBuffLength - 5] == 'R')) {
        uint8_t RegDecadic =  10 * map(SxBuff[SxBuffLength - 4], 48, 57, 0, 9) + map(SxBuff[SxBuffLength - 3], 48, 57, 0, 9);
        if (RegDecadic > 18) {
          Serial.print(F("\r\nThe registry address is out of range!"));
          return;
        }
        AD774X_Read_Registers(RegDecadic, RTxBuff, 1);
        Serial.print(F("\r\nRegistry listing R"));
        if (RegDecadic < 10)Serial.print("0");
        Serial.print(RegDecadic);
        Serial.print(F("="));
        if (RTxBuff[RegDecadic] < 0x10)Serial.print(F("0"));
        Serial.print(RTxBuff[RegDecadic], HEX);
      }
      //----------------------------------------------------------------------
      // write one register
      //----------------------------------------------------------------------
      if ((SxBuff[SxBuffLength - 8] == 'r' || SxBuff[SxBuffLength - 8] == 'R' ) && (SxBuff[SxBuffLength - 7] == 'w' || SxBuff[SxBuffLength - 7] == 'W')) {
        // convert two decimal characters to a decimal number for addressing the registry
        uint8_t RegDecadic = 10 * map(SxBuff[SxBuffLength - 6], 48, 57, 0, 9) + map(SxBuff[SxBuffLength - 5], 48, 57, 0, 9);
        if (RegDecadic > 18) {
          Serial.print(F("\r\nThe registry address is out of range!"));
          return;
        }
        if (RegDecadic < 7) {
          Serial.print(F("\r\nThe register is read only!"));
          return;
        }
        // converting two hexadecimal characters per byte and store it in the output buffer
        const char temp[3] = {(const char)SxBuff[SxBuffLength - 4], (const char) SxBuff[SxBuffLength - 3]};
        RTxBuff[RegDecadic] = (uint8_t)strtol(&temp[0], NULL, 16);
        AD774X_Write_Registers(RegDecadic, RTxBuff, 1);
        AD774X_Read_Registers(RegDecadic, RTxBuff, 1);
        Serial.print(F("\r\nRegister is saved R"));
        if (RegDecadic < 10)Serial.print(F("0"));
        Serial.print(RegDecadic);
        Serial.print(F("="));
        if (RTxBuff[RegDecadic] < 0x10)Serial.print(F("0"));
        Serial.print(RTxBuff[RegDecadic], HEX);
      }
      //----------------------------------------------------------------------
      //  restart Aruino
      //----------------------------------------------------------------------
      if ((SxBuff[SxBuffLength - 4] == 'x' || SxBuff[SxBuffLength - 4] == 'X') && (SxBuff[SxBuffLength - 3] == 'x' || SxBuff[SxBuffLength - 3] == 'X')) {
        resetFunc();
      }
      //----------------------------------------------------------------------
      // sampling OFF / ON
      //----------------------------------------------------------------------
      if ((SxBuff[SxBuffLength - 4] == 's' || SxBuff[SxBuffLength - 4] == 'S') && (SxBuff[SxBuffLength - 3] == 's' || SxBuff[SxBuffLength - 3] == 'S')) {
        StopSample = !StopSample;
        if (StopSample)Serial.print(F("\r\nPeriodic sampling stoped"));
        else Serial.print(F("\r\nPeriodic sampling started"));
      }
      //----------------------------------------------------------------------
      // reads the data registers once and displays them
      //----------------------------------------------------------------------
      if ((SxBuff[SxBuffLength - 4] == 't' || SxBuff[SxBuffLength - 4] == 'T') && (SxBuff[SxBuffLength - 3] == 't' || SxBuff[SxBuffLength - 3] == 'T')) {
        AD774X_Read_Registers(ADR_CAP_DATAH, RTxBuff, 6);
        ConvertData();
      }
      //----------------------------------------------------------------------
      // NOP - delay 250ms
      //----------------------------------------------------------------------
      if ((SxBuff[SxBuffLength - 4] == 'n' || SxBuff[SxBuffLength - 4] == 'N') && (SxBuff[SxBuffLength - 3] == 'n' || SxBuff[SxBuffLength - 3] == 'N')) {
        Serial.print(F("\r\nDelay 250 ms"));
        delay(250);
      }
    }
  }
}
//************************
// CAPDACA  CAPDACB   pF
//    80      80    9.338
//    A0      80    0.292
//    BF      80    0.000
//************************
//  CAPDAC  -0,274pF/step
//  OFFSET   + - 1pF/all
