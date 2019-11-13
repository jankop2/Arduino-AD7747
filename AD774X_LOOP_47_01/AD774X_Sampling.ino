//----------------------------------------------------------------------
//conversion of the obtained data to real values
//----------------------------------------------------------------------
void ConvertData(void) {
  long CapacitanceRaw = ((long)RTxBuff[ADR_CAP_DATAH] << 16) + ((long)RTxBuff[ADR_CAP_DATAM] << 8) + (long)RTxBuff[ADR_CAP_DATAL] - 0x800000;
  Capacitance = (double)CapacitanceRaw / 1024.0 / 1000.0;
  unsigned long TemperatureRaw = ((unsigned long)RTxBuff[ADR_VT_DATAH] << 16) + ((unsigned long)RTxBuff[ADR_VT_DATAM] << 8) + (unsigned long)RTxBuff[ADR_VT_DATAL];
  Temperature = (double) TemperatureRaw / 2048.0 - 4096.0;
  Serial.print(F("\r\nCapacitance: "));
  Serial.print(Capacitance, 6);
  Serial.print(F(" pF"));
  Serial.print(F("     Temperature: "));
  Serial.print(Temperature, 1);
  Serial.print(F(" deg.C"));
}
//----------------------------------------------------------------------
// reading six data registers each sampling period,
// specified by the SamplePeriod variable and start a new conversion
//----------------------------------------------------------------------
void PeriodicSampling(void) {
  NewTime = millis();
  if ((NewTime - OldTime) > SamplePeriod) {
    //Serial.println(NewTime - OldTime);// My debug only
    OldTime = NewTime;
    // reading valid data
    AD774X_Read_Registers(ADR_CAP_DATAH, RTxBuff, 6);
    // start a new conversion
    AD774X_Write_Single_Register(ADR_CFG, AD774X_Read_Single_Register(ADR_CFG) | SINGLE_SAMPLE);
    // conversion of acquired data
    ConvertData();
  }
}
