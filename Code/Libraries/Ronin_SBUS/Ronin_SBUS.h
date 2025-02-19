
#ifndef Ronin_SBUS_h
#define Ronin_SBUS_h

#include <Arduino.h>

#define BAUDRATE 100000
#define SERIALPORT Serial1 //Here I am using the second HW serial port of ESP8266, if you want to use the default serial port change 'Serial1' to 'Serial'

class Ronin_SBUS
{
	public:
		uint8_t sbusData[25];
		int16_t channelValues[18];
		void begin(void);
		void SetValue(uint8_t ch, int16_t position);
		void Send(void);
		void Update(void);

	private:
		uint8_t byte_in_sbus;
		uint8_t bit_in_sbus;
		uint8_t ch;
		uint8_t bit_in_servo;
};

#endif