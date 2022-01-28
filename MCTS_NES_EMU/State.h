#ifndef STATE_H
#define STATE_H

#include <cstdint>

#define byte uint8_t
#define word uint16_t

class State
{
public:

	State()
	{
		ram = new byte[0x10000];
	}

	struct Flags
	{
		bool c;
		bool z;
		bool i;
		bool d;
		bool v;
		bool n;
	};

	byte A;
	byte X;
	byte Y;

	byte SP;
	word PC;
	Flags f;

	byte* ram;

	int frame = 0;
};

#endif STATE_H