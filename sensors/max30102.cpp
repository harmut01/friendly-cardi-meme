/*
 * Library for the Maxim MAX30102 pulse oximetry system, I modified a copy of the MAX30100 library in May 2016
 */
 
#include "max30102.h"
#include "functions.h"
 
Serial pc(USBTX, USBRX);
 
void MAX30102::setLEDs(pulseWidth pw, ledCurrent red, ledCurrent ir){
    char data_read[1];
    char data_write[1];
    _i2c.read(MAX30102_ADDRESS, MAX30102__oxygen_sat_CONFIG, data_read, 1);
    data_read[0] = data_read[0] & 0xFD;                                 // Set LED_PW to 01
    data_write[0] = data_read[0] | pw;
    _i2c.write(MAX30102_ADDRESS, MAX30102__oxygen_sat_CONFIG, data_write, 1);   // Mask LED_PW
    data_write[0] = (red);
    _i2c.write(MAX30102_ADDRESS, MAX30102_LED_CONFIG_1, data_write, 1);  // write LED1 configs
    data_write[0] = (ir);
    _i2c.write(MAX30102_ADDRESS, MAX30102_LED_CONFIG_2, data_write, 1);  // write LED2 configs
}
 
void MAX30102::set_oxygen_sat(sampleRate sr, high_resolution hi_res){
    char data_read[1];
    char data_write[1];
    _i2c.read(MAX30102_ADDRESS, MAX30102__oxygen_sat_CONFIG, data_read, 1);
    data_read[0] = data_read[0] & 0x85;                                 // Set ADC_rge to 00, _oxygen_sat_SR to 001 = sr100 and LEDpw to 01 = 118
    data_write[0] = data_read[0] | (sr<<2) | (hi_res<<6);
    _i2c.write(MAX30102_ADDRESS, MAX30102__oxygen_sat_CONFIG, data_write, 1);   // Mask _oxygen_sat_SR
    _i2c.read(MAX30102_ADDRESS, MAX30102_CONFIG, data_read, 1);
    data_write[0] = data_read[0] & 0xF8; // Set Mode to 000
    _i2c.write(MAX30102_ADDRESS, MAX30102_CONFIG, data_write, 1);        // Mask MODE
}
 
void MAX30102::setInterrupt_oxygen_sat(void){
    char data_read[1];
    char data_write[1];
    _i2c.read(MAX30102_ADDRESS, MAX30102_INT_ENABLE, data_read, 1);
    data_write[0] = data_read[0] & 0x00;                                // Set Interrupt enable for _oxygen_sat | 0x10   // New: disable prox! & ~0x10
    _i2c.write(MAX30102_ADDRESS, MAX30102_INT_ENABLE, data_write, 1);    // Mask ENB__oxygen_sat_RDY
} 
 
int MAX30102::getNumSamp(void){
    char data_read[1] = {0};
//    _i2c.write(MAX30102_ADDRESS, MAX30102_FIFO_W_POINTER, data_read, 1);
//    _i2c.write(MAX30102_ADDRESS, MAX30102_FIFO_R_POINTER, data_read, 1);
//    wait(0.148);
    _i2c.read(MAX30102_ADDRESS, MAX30102_FIFO_W_POINTER, data_read, 1);
    char wrPtr = data_read[0];
    _i2c.read(MAX30102_ADDRESS, MAX30102_FIFO_R_POINTER, data_read, 1);
    char rdPtr = data_read[0];
    return ((int)wrPtr - (int)rdPtr);                                   // New counting
 //   return (abs( 16 + (int)wrPtr - (int)rdPtr ) % 16);
}
 
void MAX30102::setTemp(void){
    char data_read[1];
    char data_write[1];
    _i2c.read(MAX30102_ADDRESS, MAX30102_TEMP_CONFIG, data_read, 1);
    data_write[0] = data_read[0] | 0x01;    // Enable temperature
    _i2c.write(MAX30102_ADDRESS, MAX30102_TEMP_CONFIG, data_write, 1);   // Mask MODE
    _i2c.read(MAX30102_ADDRESS, MAX30102_TEMP_CONFIG, data_read, 1);
}
 
void MAX30102::set_oxygen_satmode(void){
    char data_read[1];
    char data_write[1];
    _i2c.read(MAX30102_ADDRESS, MAX30102_CONFIG, data_read, 1);
    data_write[0] = data_read[0] | 0x03;    // Set _oxygen_sat Mode
    _i2c.write(MAX30102_ADDRESS, MAX30102_CONFIG, data_write, 1);
}
 
int MAX30102::readTemp(void){
    char data_read[1];
    char temp_int, temp_fract;
    int temp_measured;
    _i2c.read(MAX30102_ADDRESS, MAX30102_TEMP_INTEGER, data_read, 1);
    temp_int = data_read[0];
    _i2c.read(MAX30102_ADDRESS, MAX30102_TEMP_FRACTION, data_read, 1);
    temp_fract = data_read[0] & 0x0F;
    temp_measured = ((int)temp_int)+(((int)temp_fract) >> 4);
    return temp_measured;
}
 
void MAX30102::readSensor(void){
    char data_read[6] = {0}; 
    _heart_rate = 0;
    _oxygen_sat = 0;
    _i2c.read(MAX30102_ADDRESS, MAX30102_FIFO_DATA_REG, data_read, 6);   // Read six times from the FIFO
    _heart_rate = (data_read[0]<<16) | (data_read[1]<<8) | data_read[2];         // Combine values to get the actual number
    _heart_rate = _heart_rate>>2;
    _oxygen_sat = (data_read[3]<<16) | (data_read[4]<<8) | data_read[5];       // Combine values to get the actual number
    _oxygen_sat = _oxygen_sat>>2;
}
 
void MAX30102::shutdown(void){
    char data_read[1];
    char data_write[1];
    _i2c.read(MAX30102_ADDRESS, MAX30102_CONFIG, data_read, 1);          // Get the current register
    data_write[0] = data_read[0] | 0x80;
    _i2c.write(MAX30102_ADDRESS, MAX30102_CONFIG, data_write, 1);        // mask the SHDN bit
}
 
void MAX30102::reset(void){
    char data_read[1];
    char data_write[1];
    _i2c.read(MAX30102_ADDRESS, MAX30102_CONFIG, data_read, 1);          // Get the current register
    data_write[0] = data_read[0] | 0x40;
    _i2c.write(MAX30102_ADDRESS, MAX30102_CONFIG, data_write, 1);        // mask the RESET bit
}
 
void MAX30102::startup(void){
    char data_read[1];
    char data_write[1];
    _i2c.read(MAX30102_ADDRESS, MAX30102_CONFIG, data_read, 1);          // Get the current register
    data_write[0] = data_read[0] & 0x7F;
    _i2c.write(MAX30102_ADDRESS, MAX30102_CONFIG, data_write, 1);        // mask the SHDN bit
}
 
char MAX30102::getRevID(void){
    char data_read[1];
    _i2c.read(MAX30102_ADDRESS, MAX30102_REVISION_ID, data_read, 1);
    return data_read[0];
}
 
char MAX30102::getPartID(void){
    char data_read[1];
    _i2c.read(MAX30102_ADDRESS, MAX30102_PART_ID, data_read, 1);
    return data_read[0];
}
 
void MAX30102::begin(pulseWidth pw, ledCurrent ir, sampleRate sr){
    char data_write[1];
    data_write[0] = 0x02;
    _i2c.write(MAX30102_ADDRESS, MAX30102_CONFIG, data_write, 1);        // Heart rate only
    data_write[0] = ir;
    _i2c.write(MAX30102_ADDRESS, MAX30102_LED_CONFIG_1, data_write, 1);
    data_write[0] = ((sr<<2)|pw);
    _i2c.write(MAX30102_ADDRESS, MAX30102__oxygen_sat_CONFIG, data_write, 1);
}
 
void MAX30102::init(pulseWidth pw, sampleRate sr, high_resolution hi_res, ledCurrent red, ledCurrent ir){
    char data_write[1];
    
    setLEDs(pw, red, ir);
    set_oxygen_sat(sr, hi_res);
    
    data_write[0] = 0x10;
    _i2c.write(MAX30102_ADDRESS, MAX30102_FIFO_CONFIG, data_write, 1);
      
}
 
void MAX30102::printRegisters(void){
    char data_read[1];
    _i2c.read(MAX30102_ADDRESS, MAX30102_INT_STATUS, data_read, 1);
    pc.printf("INT_STATUS: %#4X\r\n", data_read[0]);
    _i2c.read(MAX30102_ADDRESS, MAX30102_INT_ENABLE, data_read, 1);
    pc.printf("INT_ENABLE: %#4X\r\n", data_read[0]);
    _i2c.read(MAX30102_ADDRESS, MAX30102_FIFO_W_POINTER, data_read, 1);
    pc.printf("FIFO_W_POINTER: %#4X\r\n", data_read[0]);
    _i2c.read(MAX30102_ADDRESS, MAX30102_OVR_COUNTER, data_read, 1);
    pc.printf("OVR_COUNTER: %#4X\r\n", data_read[0]);
    _i2c.read(MAX30102_ADDRESS, MAX30102_FIFO_R_POINTER, data_read, 1);
    pc.printf("FIFO_R_POINTER: %#4X\r\n", data_read[0]);
    _i2c.read(MAX30102_ADDRESS, MAX30102_FIFO_DATA_REG, data_read, 1);
    pc.printf("FIFO_DATA_REG: %#4X\r\n", data_read[0]);
    _i2c.read(MAX30102_ADDRESS, MAX30102_CONFIG, data_read, 1);
    pc.printf("CONFIG: %#4X\r\n", data_read[0]);
    _i2c.read(MAX30102_ADDRESS, MAX30102__oxygen_sat_CONFIG, data_read, 1);
    pc.printf("_oxygen_sat_CONFIG: %#4X\r\n", data_read[0]);
    _i2c.read(MAX30102_ADDRESS, MAX30102_LED_CONFIG_2, data_read, 1);
    pc.printf("LED_CONFIG: %#4X\r\n", data_read[0]);
    _i2c.read(MAX30102_ADDRESS, MAX30102_TEMP_INTEGER, data_read, 1);
    pc.printf("TEMP_INTEGER: %#4X\r\n", data_read[0]);
    _i2c.read(MAX30102_ADDRESS, MAX30102_TEMP_FRACTION, data_read, 1);
    pc.printf("TEMP_FRACTION: %#4X\r\n", data_read[0]);
    _i2c.read(MAX30102_ADDRESS, MAX30102_TEMP_CONFIG, data_read, 1);
    pc.printf("TEMP_CONFIG: %#4X\r\n", data_read[0]);
    _i2c.read(MAX30102_ADDRESS, MAX30102_REVISION_ID, data_read, 1);
    pc.printf("REVISION_ID: %#4X\r\n", data_read[0]);
    _i2c.read(MAX30102_ADDRESS, MAX30102_PART_ID, data_read, 1);
    pc.printf("PART_ID: %#4X\r\n", data_read[0]);
}


uint32_t MAX30102::getHeartRate(void){
    return _heart_rate;
}

uint32_t MAX30102::getSPO2(void){
    return _oxygen_sat;
}
