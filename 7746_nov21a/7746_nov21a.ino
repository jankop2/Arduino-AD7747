#include <Wire.h>
 
#define SLAVEWRT 0x48 // address of device during write cycle
#define SLAVERD  0x48 // address of device during read cycle

//AD7746 definitions
#define I2C_ADDRESS  0x48 //0x90 shift one to the right
 
#define REGISTER_STATUS 0x00
#define REGISTER_CAP_DATA 0x01
#define REGISTER_VT_DATA 0x04
#define REGISTER_CAP_SETUP 0x07
#define REGISTER_VT_SETUP 0x08
#define REGISTER_EXC_SETUP 0x09
#define REGISTER_CONFIGURATION 0x0A
#define REGISTER_CAP_DAC_A 0x0B
#define REGISTER_CAP_OFFSET 0x0D
#define REGISTER_CAP_GAIN 0x0F
#define REGISTER_VOLTAGE_GAIN 0x11
 
#define RESET_ADDRESS 0xBF
 
#define CAP_ZERO 0x800000L
 
int mySensor = 0; // where capacitive sensor data is kept

unsigned long readLong(unsigned char r) {
  union {
    char data[4];
    unsigned long value;
  }
  byteMappedLong;
 
  byteMappedLong.value = 0L;
 
  Wire.beginTransmission(I2C_ADDRESS); // begin read cycle
  Wire.write(0); //pointer to first data register
  Wire.endTransmission(); // end cycle
  //the data pointer is reset anyway - so read from 0 on
 
  Wire.requestFrom(I2C_ADDRESS,r+4); // reads 2 bytes plus all bytes before the register
 
    while (!Wire.available()==r+4) {
      ; //wait
    }
  for (int i=r+3; i>=0; i--) {
    uint8_t c = Wire.read();
    if (i < 4) {
      byteMappedLong.data[i]= c;
    }
  }
 
  return byteMappedLong.value;
 
}

long readValue() {
 long ret = 0;
 uint8_t data[3];
 
 char status = 0;
 //wait until a conversion is done
 while (!(status & (_BV(0) | _BV(2)))) {
 status= readRegister(REGISTER_STATUS);
 }
 
 unsigned long value =  readLong(REGISTER_CAP_DATA);
 
 value >>=8;
 //we have read one byte to much, now we have to get rid of it
 ret =  value;
 
 return ret;
}

void calibrate() {
  calibration = 0;
 
  Serial.println("Calibrating CapDAC A");
 
  long value = readValue();
 
  while (value > VALUE_UPPER_BOUND && calibration < 128) {
    calibration++;
    writeRegister(REGISTER_CAP_DAC_A, _BV(7) | calibration);
    value = readValue();
  }
  Serial.println("done");
}

void setup() {
  Serial.println("Initializing");
  Wire.begin(); // sets up i2c for operation
  Serial.begin(115200); // set up baud rate for serial
 
  Wire.beginTransmission(I2C_ADDRESS); // start i2c cycle
  Wire.write(RESET_ADDRESS); // reset the device
  Wire.endTransmission(); // ends i2c cycle
 
  //wait a tad for reboot
  delay(1);
   
  writeRegister(REGISTER_EXC_SETUP, _BV(3) | _BV(1) | _BV(0)); // EXC source A
 
  writeRegister(REGISTER_CAP_SETUP,_BV(7)); // cap setup reg - cap enabled
 
  Serial.println("Getting offset");
  offset = ((unsigned long)readInteger(REGISTER_CAP_OFFSET)) << 8;
  Serial.print("Factory offset: ");
  Serial.println(offset);
 
  writeRegister(0x0A, _BV(7) | _BV(6) | _BV(5) | _BV(4) | _BV(3) | _BV(2) | _BV(0));  // set configuration to calib. mode, slow sample
 
  //wait for calibration
  delay(10);
 
  //();
  Serial.print("Calibrated offset: ");
  offset = ((unsigned long)readInteger(REGISTER_CAP_OFFSET)) &lt;&lt; 8;
  Serial.println(offset);

}

void loop() {
  long value = readValue();
  Serial.print(offset);
  Serial.print("/");
  Serial.print((int)calibration);
  Serial.print("/");
  Serial.println(value);
 
  if ((valueVALUE_UPPER_BOUND)) {
    outOfRangeCount++;
  }
  if (outOfRangeCount > MAX_OUT_OF_RANGE_COUNT) {
    if (value < VALUE_LOWER_BOUND) {
      calibrate(-CALIBRATION_INCREASE);
    }
    else {
      calibrate(CALIBRATION_INCREASE);
    }
    outOfRangeCount=0;
  }
 
  delay(50);

}
