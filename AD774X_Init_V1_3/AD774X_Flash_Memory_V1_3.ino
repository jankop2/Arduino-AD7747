//----------------------------------------------------------------------
void WriteRegistersFromFlash(void) {
  // 0xAA is flag for default setting AD774X from EEPROM
  if (EEPROM.read(EEPROMStart) != 0xAA) {
    // read registers from PROGMEM
    for (uint8_t i = ADR_CAP_SETUP; i < ADR_CAP_GAINH; i++) {
      RTxBuff[i] = pgm_read_byte_near(DefaultRegisters + i);
    }
  }
  else {
    // read registers from EEPROM
    for (uint8_t i = ADR_CAP_SETUP; i < ADR_CAP_GAINH; i++) {
      RTxBuff[i] = EEPROM.read(i);
    }
    EEPROM.get (EEPROMSamplePeriod, SamplePeriod);
  }
  AD774X_Write_Registers(ADR_CAP_SETUP, RTxBuff, 8);
}
//----------------------------------------------------------------------
void WriteRegistersToFlash(void) {
  AD774X_Read_Registers(ADR_CAP_SETUP, RTxBuff, 8);
  for (int8_t i = ADR_CAP_SETUP; i < ADR_CAP_GAINH; i++) {
    EEPROM.update(i, RTxBuff[i]);
  }
  EEPROM.put (EEPROMSamplePeriod, SamplePeriod);
  // 0xAA is flag for default setting AD774X from EEPROM
  EEPROM.update(0, 0xAA);
}
void DeleteEEPROM(void) {
  for (uint8_t i = EEPROMStart; i < 19; i++)EEPROM.update(i, 0xFF);
}
