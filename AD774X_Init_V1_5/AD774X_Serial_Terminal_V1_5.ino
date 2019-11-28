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
    // read one character into end the buffer
    SxBuff[SxBuffLength - 1] = Serial.read();
    //----------------------------------------------------------------------
    //  ? are the termination characters 0x0d + 0x0a at the end of the buffer ?
    //----------------------------------------------------------------------
    if (SxBuff[SxBuffLength - 2] == 0x0D && SxBuff[SxBuffLength - 1] == 0x0A) {

      //----------------------------------------------------------------------
      // write registers to flash
      //----------------------------------------------------------------------
      if ((SxBuff[SxBuffLength - 4] == 'f' || SxBuff[SxBuffLength - 4] == 'F') && (SxBuff[SxBuffLength - 3] == 'w' || SxBuff[SxBuffLength - 3] == 'W')) {
        WriteRegistersToFlash();
        Serial.print(F("\r\nRegisters 07-14 and SamplePeriod interval are written to EEPROM memory as default"));
      }
      //----------------------------------------------------------------------
      // delete flash memory and sets PROGMEM as default
      //----------------------------------------------------------------------
      if ((SxBuff[SxBuffLength - 4] == 'f' || SxBuff[SxBuffLength - 4] == 'F') && (SxBuff[SxBuffLength - 3] == 'r' || SxBuff[SxBuffLength - 3] == 'R')) {
        DeleteEEPROM();
        WriteRegistersFromFlash();
        Serial.print(F("\r\nDeleted EEPROM memory and set PROGMEM as default for registers"));
        StartNewConversion();
      }
      //----------------------------------------------------------------------
      // Automatic capacity channel offset adjustment
      //----------------------------------------------------------------------
      if ((SxBuff[SxBuffLength - 4] == 'o' || SxBuff[SxBuffLength - 4] == 'O' ) && (SxBuff[SxBuffLength - 3] == 'o' || SxBuff[SxBuffLength - 3] == 'O')) {
        Serial.print(F("\r\n!!! Quiet please !!!"));
        Serial.print(F("\r\nAutomatic of capacity channel offset adjustment starting"));
        OffsetAutomaticStart();
      }
      //----------------------------------------------------------------------
      // offset compensation is deleted
      //----------------------------------------------------------------------
      if ((SxBuff[SxBuffLength - 4] == 'o' || SxBuff[SxBuffLength - 4] == 'O' ) && (SxBuff[SxBuffLength - 3] == 'r' || SxBuff[SxBuffLength - 3] == 'R')) {
        CapdacClear();
        Serial.print(F("\r\nOffset compensation is deleted"));
        Serial.print(F("\r\nThis setting can be permanently saved by the FW command !"));
        StartNewConversion();
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
        Serial.print(F("\r\nThe sampling period is written [ms]: "));
        Serial.print(SamplePeriod);
        Serial.print(F("\r\nThis setting can be permanently saved by the FW command !"));
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
        EnablePeriodicSampling = !EnablePeriodicSampling;
        if (EnablePeriodicSampling)Serial.print(F("\r\nPeriodic sampling started"));
        else Serial.print(F("\r\nPeriodic sampling stoped"));
      }
      //----------------------------------------------------------------------
      // reads the data registers once and displays them
      //----------------------------------------------------------------------
      if ((SxBuff[SxBuffLength - 4] == 't' || SxBuff[SxBuffLength - 4] == 'T') && (SxBuff[SxBuffLength - 3] == 't' || SxBuff[SxBuffLength - 3] == 'T')) {
        AD774X_Read_Registers(ADR_CAP_DATAH, RTxBuff, 6);
        SerialPrintData();
      }
      //----------------------------------------------------------------------
      // NOP - delay 250ms
      //----------------------------------------------------------------------
      if ((SxBuff[SxBuffLength - 4] == 'n' || SxBuff[SxBuffLength - 4] == 'N') && (SxBuff[SxBuffLength - 3] == 'n' || SxBuff[SxBuffLength - 3] == 'N')) {
        Serial.print(F("\r\nDelay 250 ms"));
        delay(250);
      }
      //----------------------------------------------------------------------
      // version of installed firmware
      //----------------------------------------------------------------------
      if ((SxBuff[SxBuffLength - 4] == 'v' || SxBuff[SxBuffLength - 4] == 'V') && (SxBuff[SxBuffLength - 3] == 'v' || SxBuff[SxBuffLength - 3] == 'V')) {
        Serial.print(F(VERSION));
      }
    }
  }
}
