//---------------------------------------------------------------------
// SW reset AD774X, registers of AD774X are set to factory default
// Input: none
// Output: I2C_State - returns an error of I2C, 0 = without error
//---------------------------------------------------------------------
void AD774X_Reset() {
  Wire.beginTransmission(AD774X_ADDRESS);
  Wire.write(0xBF);
  I2C_State = Wire.endTransmission();
  delay(1);
}
//---------------------------------------------------------------------
// Reads one register AD774X
// Input: RegAdres - registry address
// Output: content of the registry
// Output: I2C_State - returns an error of I2C, 0 = without error
//---------------------------------------------------------------------
uint8_t AD774X_Read_Single_Register(uint8_t RegAdres) {
  Wire.beginTransmission(AD774X_ADDRESS);
  Wire.write(RegAdres);
  I2C_State = Wire.endTransmission(false);
  Wire.requestFrom(AD774X_ADDRESS, OneByte);
  return Wire.read();
}
//---------------------------------------------------------------------
// Reads the specified number of registers to the TxBuff
// INPUT: RegAdres - first registry address
// INPUT: quantity - number of registers read
// Output: TxBuff - data in the TxBuff field are stored at the registry addresses !!!
// Output: I2C_State - returns an error of I2C, 0 = without error
//---------------------------------------------------------------------
void AD774X_Read_Registers(uint8_t RegAdres, uint8_t *TxBuff, uint8_t quantity) {
  Wire.beginTransmission(AD774X_ADDRESS);
  Wire.write(RegAdres);
  I2C_State = Wire.endTransmission(false);
  Wire.requestFrom(AD774X_ADDRESS, quantity);
  for (uint8_t i = 0; i < quantity; i++) {
    TxBuff[RegAdres + i] = Wire.read();
  }
}
//---------------------------------------------------------------------
// Writes one byte to one AD774X register
// Input: RegAdres - registry address
// Input: DataSingl - written data
// Output: I2C_State - returns an error of I2C, 0 = without error
//----------------------------------------------------------------------
uint8_t AD774X_Write_Single_Register(uint8_t RegAdres, uint8_t DataSingl) {
  Wire.beginTransmission(AD774X_ADDRESS);
  Wire.write(RegAdres);
  Wire.write(DataSingl);
  I2C_State = Wire.endTransmission();
  return I2C_State;
}
//---------------------------------------------------------------------
// Writes to the registry a defined number of bytes that are stored in the RxBuff field
// Input: RegAdres - first registry address
// Input: RxBuff - buffer where the transmitted data are stored,
//         data to RxBuff must always be saved to register addresses !!!
// Input: quantity - number of registers write
// Output: I2C_State - returns an error of I2C, 0 = without error
//---------------------------------------------------------------------
void AD774X_Write_Registers(uint8_t RegAdres, uint8_t *RxBuff, uint8_t quantity ) {
  Wire.beginTransmission(AD774X_ADDRESS);
  Wire.write(RegAdres);
  Wire.write(RxBuff + RegAdres, quantity);
  I2C_State = Wire.endTransmission();
}
