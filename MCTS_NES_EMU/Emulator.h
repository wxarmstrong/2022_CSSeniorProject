#ifndef EMU_H
#define EMU_H

#include <functional>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdint>
#include "State.h"
#include "Node.h"
#include "Action.h"

class Emulator
{
public:
	State state;
	byte* header;
	byte* rom;
	void savestate(Node* n);
	void loadstate(Node* n);
	void setBreak(std::string type, word addr);
	bool isBreak();
	bool is_multiply();
	bool is_switch();
	void run();
	int bestScore = -9999;
private:
	const int static HEADER_SIZE = 0x10;
	const int static BANK_SIZE = 0x2000;
	const int static EDIT_SIZE = 0x8000;
	const int static RAM_SIZE = 0x10000;
	friend std::istream& operator >> (std::istream& g, Emulator& e);
	void initialize();
	void load_bank(int bank_num, int page_num);
	void push_byte(byte val);
	void push_word(word val);
	byte pull_byte();
	word pull_word();
};

#endif EMU_H