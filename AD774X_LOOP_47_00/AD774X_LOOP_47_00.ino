#include <Wire.h>
#include <EEPROM.h>
// AD774X I2C address
const uint8_t  AD774X_ADDRESS = 0x48;
// AD774X Register address Definition
//--------------------------------------------------
const uint8_t ADR_STATUS     =  0;  // Read Only
const uint8_t ADR_CAP_DATAH  =  1;  // Read Only
const uint8_t ADR_CAP_DATAM  =  2;  // Read Only
const uint8_t ADR_CAP_DATAL  =  3;  // Read Only
const uint8_t ADR_VT_DATAH   =  4;  // Read Only
const uint8_t ADR_VT_DATAM   =  5;  // Read Only
const uint8_t ADR_VT_DATAL   =  6;  // Read Only
const uint8_t ADR_CAP_SETUP  =  7;  // CAP SETUP REGISTER
const uint8_t ADR_VT_SETUP   =  8;  // VT SETUP REGISTER
const uint8_t ADR_EXC_SETUP  =  9;  // EXC SETUP REGISTER
const uint8_t ADR_CFG        = 10;  // CONFIGURATION REGISTER
const uint8_t ADR_CAPDACA    = 11;  // CAP DAC A REGISTER
const uint8_t ADR_CAPDACB    = 12;  // CAP DAC B REGISTER
const uint8_t ADR_CAP_OFFH   = 13;  // CAP OFFSET CALIBRATION REGISTER HIGH
const uint8_t ADR_CAP_OFFL   = 14;  // CAP OFFSET CALIBRATION REGISTER LOW
const uint8_t ADR_CAP_GAINH  = 15;  // factory calibration
const uint8_t ADR_CAP_GAINL  = 16;  // factory calibration
const uint8_t ADR_VOLT_GAINH = 17;  // factory calibration
const uint8_t ADR_VOLT_GAINL = 18;  // factory calibration
// AD7747 Register Data Definition
//*** Single Ended Input 0-16pF, configuration as the Figure 28. from datasheet ***
const uint8_t DATA_CAP_SETUP = B10100000;  // 7  0xA0 CAPEN+0+CAPDIF+00000 according to the datasheet AD7747
const uint8_t DATA_VT_SETUP  = B10000001;  // 8  0x81 VTEN+000000+VTCHOP for internal temperature
const uint8_t DATA_EXC_SETUP = B00001110;  // 9  0x0E 0000+EXDAC+EXCEN+0 according to the datasheet AD7747
const uint8_t DATA_CFG       = B10100000;  // 10 0xA0 VTFS1+0+CAPFS2+00000 = Idle mode, conversion time VT-62.1ms; CAP-124ms
const uint8_t DATA_CAPDACA   = B10011111;  // 11 0xA0 CAPDACA ON + 8pF , mid range of capacity = 0x20 (is cca -9pF, cca -0.274 pF by step)
const uint8_t DATA_CAPDACB   = B10000000;  // 12 0x80 CAPDACB ON + 0pF according to the datasheet AD7747 for Single-Ended
const uint8_t DATA_CAP_OFFH  = B10000000;  // 13 0x80 OFFSET 0x8000 - the middle of the interval (the full range is +- 1 pF)
const uint8_t DATA_CAP_OFFL  = B00000000;  // 14 0x00     "                 "
//--------------------------------------------------------
const uint8_t  SINGLE_SAMPLE = B00000010;  // 0x02 is "OR" mask for start single sample
uint8_t RTxBuff[20];                       // I/O buffer for AD774X registers
const uint8_t OneByte = 1;
const uint8_t SxBuffLength = 8;            // length of input buffer for parsing commands
uint8_t  SxBuff[SxBuffLength + 1];         // input buffer for serial parser
uint8_t  I2C_State = 0;                    // status of I2C bus, 0 = without error
unsigned int SamplePeriod = 1000;          // Sample period in [ms]
unsigned long NewTime = 0, OldTime = 0;    // for time counter
double Capacitance = 0.0;
double Temperature = 0.0;
bool StopSample = true;    // periodic sampling is default stopped !!! 
// the indexes in the DefaultRegisters field correspond to the addresses of each AD774X registry
const uint8_t DefaultRegisters[] PROGMEM = {0, 0, 0, 0, 0, 0, 0, DATA_CAP_SETUP, DATA_VT_SETUP, DATA_EXC_SETUP, DATA_CFG, DATA_CAPDACA, DATA_CAPDACB, DATA_CAP_OFFH, DATA_CAP_OFFL, 0, 0, 0, 0};
//----------------------------------------------------------------------
//declare Arduino reset function at address 0
//----------------------------------------------------------------------
void(* resetFunc)(void) = 0;
//----------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  Wire.begin();
  Serial.print(F("\r\nArduino Restart"));
  //--------------------------------------------------------------------
  // SW reset and init AD774X
  // AD774X registers 07-14 are initialized from PROGMEM before using of command FW - Flash Write
  // for the first time. After using of FW command, registers are initialized only from FLASH.
  //--------------------------------------------------------------------
  AD774X_Reset();
  if (EEPROM.read(0) != 0xAA) {
    for (uint8_t i = ADR_CAP_SETUP; i < ADR_CAP_GAINH; i++) {
      RTxBuff[i] = pgm_read_byte_near(DefaultRegisters + i);
    }
  }
  else {
    for (uint8_t i = ADR_CAP_SETUP; i < ADR_CAP_GAINH; i++) {
      RTxBuff[i] = EEPROM.read(i);
    }
  }
  AD774X_Write_Registers(ADR_CAP_SETUP, RTxBuff, 8);
  //--------------------------------------------------------------------
  // start of first conversion
  //--------------------------------------------------------------------
  AD774X_Write_Single_Register(ADR_CFG, AD774X_Read_Single_Register(ADR_CFG) | SINGLE_SAMPLE);
  // preparation for sampling period measurement
  OldTime = millis();
  Serial.print(F("\r\nI'm waiting for commands:"));
}
//**********************************************************************
void loop() {
  if (!StopSample)PeriodicSampling();
  SerialTerminal();
}
