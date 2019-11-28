#define VERSION "\r\nVersion of installed firmware is V1.4 - 11/2019 by jankop"
#include <Wire.h>
#include <EEPROM.h>
const uint8_t AD774X_ADDRESS = 0x48;// AD774X I2C address
//*********************** Settings! ********************************
// AD7747 default registers definition for the differential input configuration
// +- 8.192pF  or for the single-ended input configuration 0 - 8.192pF.
// This setting is compatible with automatic offset using the OO command.
// Registers definitions are for AD7747 only! You must use yours own
// registers settings for AD7745 or AD7746 here!
const bool    AD7747         = true;       // Set your IC type, true = AD7747, false = AD7745/46
const uint8_t DATA_CAP_SETUP = B10100000;  // 7  0xA0 CAPEN+0+CAPDIF+00000 according to the datasheet AD7747
const uint8_t DATA_VT_SETUP  = B10000001;  // 8  0x81 VTEN+000000+VTCHOP for internal temperature
const uint8_t DATA_EXC_SETUP = B00001110;  // 9  0x0E 0000+EXDAC+EXCEN+EXCLVL1+0 according to the datasheet AD7747
const uint8_t DATA_CFG       = B10100000;  // 10 0xA0 VTFS1+0+CAPFS2+00000 = Idle mode, conversion time VT-62.1ms; CAP-124ms
const uint8_t DATA_CAPDACA   = B00000000;  // 11 0x00 CAPDACA OFF
const uint8_t DATA_CAPDACB   = B00000000;  // 12 0x00 CAPDACB OFF
const uint8_t DATA_CAP_OFFH  = B10000000;  // 13 0x80 OFFSET 0x8000 - the middle of the interval (the full range is is approximately +- 1 pF)
const uint8_t DATA_CAP_OFFL  = B00000000;  // 14 0x00     "                 "
//******************** Settings end  ********************************
// AD774X Register address Definition
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
//-------------------------------------------------------------------
const uint8_t MODES          = B11111000;  // 0xF8 is "AND" mask for preset convert mode
const uint8_t SINGLE         = B00000010;  // 0x02 is "OR" mask for start single convert mode
const uint8_t CONTIN         = B00000001;  // 0x01 is "OR" mask for start continual convert mode
const uint8_t CAPDAC_ON      = B10000000;  // 0x80 is "OR" mask for CAPDAC ON
const uint8_t CAPDAC_OFF     = B00000000;  // 0x00 is value for CAPDAC OFF
const uint8_t CAP_RDY        = B00000001;  // 0x01 is "AND" mask for CAP READY
const uint8_t REFERENCE      = B11101111;  // 0xEF is "AND" mask for unconditionally set internal reference
const uint8_t EEPROMStart    = 0;          // first address of EEPROM - EEPROM/PROGMEM flag (one byte of EEPROM)
const uint8_t EEPROMAddrSamplePeriod = 1;  // address for SamplePeriod variable (two bytes of EEPROM)
const uint8_t OneByte = 1;                 // auxiliary variables
const uint8_t HowManySteps = 5;            // number of samples per phase of automatic offset adjustment, the phases are four
uint8_t StepByStep = 0;                    // auxiliary variables to phase the offset adjustment process
const unsigned long AD774XTimeOut = 1000;  // response waiting time when offset is setting
unsigned long TimeTemp;                    // auxiliary variables for timing
const uint8_t SxBuffLength = 8;            // length of input buffer for parsing commands
uint8_t SxBuff[SxBuffLength + 1];          // input buffer for serial parser
uint8_t RTxBuff[20];                       // I/O buffer for AD774X registers
uint8_t I2C_State = 0;                     // status of I2C bus, 0 = without error
unsigned int SamplePeriod = 1000;          // sample period in [ms]
float C1 = 0, C2 = 0;                      // auxiliary variables for zero correction calculation
float Capacitance = 0.0, Temperature = 0.0;// real data
bool EnablePeriodicSampling = false;       // periodic sampling with output to serial port, is default disabled
bool EnableSerialTerminal = true;          // enable input from serial port
bool EnableOffsetAutomatic = false;        // enable automatic offset, better said automatic zero setting, is stopped as default
// the indexes in the DefaultRegisters field correspond to the addresses of each AD774X registry
const uint8_t DefaultRegisters[] PROGMEM = {0, 0, 0, 0, 0, 0, 0, DATA_CAP_SETUP, DATA_VT_SETUP, DATA_EXC_SETUP, DATA_CFG,
                                            DATA_CAPDACA, DATA_CAPDACB, DATA_CAP_OFFH, DATA_CAP_OFFL, 0, 0, 0, 0
                                           };
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
  // AD774X registers 07-14 are initialized from PROGMEM before using of the command FW - Flash Write
  // for the first time. After using of FW command, registers are initialized only from EEPROM.
  // PROGMEM reuse is possible after application of the FR command. The commands are entered via
  // the serial interface as described in section AD774X_Comment.
  //--------------------------------------------------------------------
  AD774X_Reset();
  if (I2C_State != 0) Serial.print(F("\r\nAD774X not responding !"));
  WriteRegistersFromFlash();
  StartNewConversion();
  Serial.print(F("\r\nI'm waiting for commands:"));
}
//----------------------------------------------------------------------
void loop() {
  if (EnableOffsetAutomatic)OffsetAutomaticBody();
  if (EnablePeriodicSampling)PeriodicSampling();
  if (EnableSerialTerminal)SerialTerminal();
}
