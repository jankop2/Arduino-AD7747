//--------------------------------------------------------------------------------------------------
// automatic offset adjustment,
// it uses CAPDAC and OFFSET registers
//--------------------------------------------------------------------------------------------------
void OffsetAutomaticStart(void) {
  // stops unnecessary processes
  EnablePeriodicSampling = false;
  EnableSerialTerminal = false;
  // CAPDACs ON
  RTxBuff[ADR_CAPDACA] = CAPDAC_ON;
  RTxBuff[ADR_CAPDACB] = CAPDAC_ON;
  // the offset registers to the center
  RTxBuff[ADR_CAP_OFFH] = 0x80;
  RTxBuff[ADR_CAP_OFFL] = 0x00;
  // sets this registers
  AD774X_Write_Registers(ADR_CAPDACA, RTxBuff, 4);
  C1 = 0;
  C2 = 0;
  StepByStep = 0;
  // sets VT_SETUP register for internal reference
  AD774X_Write_Single_Register(ADR_VT_SETUP, AD774X_Read_Single_Register(ADR_VT_SETUP) & REFERENCE);
  // sets continual mode for conversion to speed up the compensation process
  AD774X_Write_Single_Register(ADR_CFG, (AD774X_Read_Single_Register(ADR_CFG) & MODES) | CONTIN);
  // enable offset compensation process
  EnableOffsetAutomatic = true;
  TimeTemp = millis();
}
void OffsetAutomaticBody(void) {
  // ? is an AD774X respond ? - timeout detection
  if (millis() - AD774XTimeOut > TimeTemp ) {
    Serial.print(F("\r\nAD774X not responding !"));
    CapdacClear();
    OffsetAutomaticEnd();
    return;
  }
  // waits for flag RDYCAP
  if ((AD774X_Read_Single_Register(ADR_STATUS) & CAP_RDY) == 0) {
    TimeTemp = millis();
    // auxiliary variable for CAPDACs range testing
    uint8_t TestScale = 16;   
    // reads valid data
    AD774X_Read_Registers(ADR_CAP_DATAH, RTxBuff, 3);
    if (StepByStep < HowManySteps) {
      // converts valid CAP data, the sum of the deviation patterns from zero
      C1 += ConvertCapData();
      StepByStep++;
      if (StepByStep >= HowManySteps) {
        //to accurately determine the capacity range of the CAPDAC register
        if (C1 >= 0)AD774X_Write_Single_Register(ADR_CAPDACA, CAPDAC_ON | TestScale);
        else   AD774X_Write_Single_Register(ADR_CAPDACB, CAPDAC_ON | TestScale);
        Serial.print(F("\r\nPhase 1. terminated"));
      }
      return;
    }
    if (StepByStep < 2 * HowManySteps) {
      // Convert valid CAP data, sum of samples
      C2 += ConvertCapData();
      StepByStep++;
      if (StepByStep >= 2 * HowManySteps) {
        // rough CAPDACs correction settings
        if (C1 >= 0)AD774X_Write_Single_Register(ADR_CAPDACA, (uint8_t)((float)TestScale * C1 / (C1 - C2)) | CAPDAC_ON);
        else AD774X_Write_Single_Register(ADR_CAPDACB, (uint8_t)((float)TestScale * C1 / (C1 - C2)) | CAPDAC_ON);
        C1 = 0;
        C2 = 0;
        Serial.print(F("\r\nPhase 2. terminated"));
      }
      return;
    }
    if (StepByStep < 3 * HowManySteps) {
      // Convert valid CAP data, sum of samples
      C1 += ConvertCapData();
      StepByStep++;
      if (StepByStep >= 3 * HowManySteps) {
        // to accurately determine the capacity range of the OFFSET register
        if (C1 >= 0) {
          AD774X_Write_Single_Register(ADR_CAP_OFFH, 0xff);
          AD774X_Write_Single_Register(ADR_CAP_OFFL, 0xff);
        }
        else
        {
          AD774X_Write_Single_Register(ADR_CAP_OFFH, 0x00);
          AD774X_Write_Single_Register(ADR_CAP_OFFL, 0x00);
        }
        Serial.print(F("\r\nPhase 3. terminated"));
      }
      return;
    }
    if (StepByStep < 4 * HowManySteps) {
      // Convert valid CAP data, sum of samples
      C2 += ConvertCapData();
      StepByStep++;
      if (StepByStep >= 4 * HowManySteps) {
        // calculation CAP_OFFSET correction settings
        float FineOffset = (32768.0 * C1 / (C1 - C2));
        // an error - offset could not be compensated
        if (abs(FineOffset) > 32767) {
          Serial.print(F("\r\nPhase 4. terminated"));
          Serial.print(F("\r\nError - Offset could not be compensated !"));
          CapdacClear();
          OffsetAutomaticEnd();
          return;
        }
        //fine CAP_OFFSET correction settings
        if (C1 >= 0) FineOffset = 32768.0 + FineOffset;
        else FineOffset = 32768.0 - FineOffset;
        AD774X_Write_Single_Register(ADR_CAP_OFFH, (uint8_t)((uint16_t)FineOffset >> 8));
        AD774X_Write_Single_Register(ADR_CAP_OFFL, (uint8_t)((uint16_t)FineOffset & 0x00FF));
        Serial.print(F("\r\nPhase 4. terminated"));
        Serial.print(F("\r\nOffset setting complete !"));
        Serial.print(F("\r\nThis setting can be permanently saved by the FW command !"));
        OffsetAutomaticEnd();
      }
      return;
    }
  }
}
void CapdacClear(void) {
  // CAPDACs OFF
  RTxBuff[ADR_CAPDACA] = CAPDAC_OFF;
  RTxBuff[ADR_CAPDACB] = CAPDAC_OFF;
  // the offset registers to the center
  RTxBuff[ADR_CAP_OFFH] = 0x80;
  RTxBuff[ADR_CAP_OFFL] = 0x00;
  // sets this registers
  AD774X_Write_Registers(ADR_CAPDACA, RTxBuff, 4);
}
void OffsetAutomaticEnd(void) {
  // renewal of periodic processes
  EnableOffsetAutomatic = false;
  EnablePeriodicSampling = true;
  EnableSerialTerminal = true;
  // starts first sample after offset setting
  StartNewConversion();
}
