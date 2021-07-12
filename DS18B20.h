#include "Arduino.h"

class DS18B20
{
protected:
	byte counter = 0;
	int tRead;
	int signBit;
	float value;
	int Pin;
	
public:
	DS18B20(int pin) {Pin = pin;};

	void OneWireReset(int Pin);
	void OneWireOutByte(int Pin, byte d);
	byte OneWireInByte(int Pin);
	void tempConvert(byte Pin);
	void tempRead(byte Pin);
	int tempCheck();
};