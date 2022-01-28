#include "Emulator.h"
#include <cassert>

std::istream& operator >> (std::istream& g, Emulator& e) {
	g.seekg(0, g.end);
	int file_size = g.tellg();
	g.seekg(0, g.beg);

	e.header = new byte[Emulator::HEADER_SIZE];
	g.read((char*)e.header, Emulator::HEADER_SIZE);

	int rom_size = file_size - Emulator::HEADER_SIZE;
	e.rom = new byte[rom_size];
	g.read((char*)e.rom, rom_size);

	e.initialize();

	return g;
}

void Emulator::initialize()
{
	std::cout << "Emulator::initialize" << std::endl;
	state.f.c = false;
	state.f.z = false;
	state.f.i = false;
	state.f.d = false;
	state.f.v = false;
	state.f.n = false;

	for (int i = 0; i < Emulator::EDIT_SIZE; i++)
		state.ram[i] = 0x00;

	load_bank(3, 0xFE);
	load_bank(4, 0xFF);
	word RESET_VECTOR = *(word*)&(state.ram[0xFFFC]);
	std::cout << "Reset vector detected at: " << std::hex << (unsigned int)RESET_VECTOR << std::endl;
	state.PC = RESET_VECTOR;

	// game specific
	state.ram[0xFAC8] == 0x60;
	state.ram[0xFCA3] == 0x60;
	state.ram[0xFB66] == 0x60;
}

void Emulator::load_bank(int bank_num, int page_num)
{
//	std::cout << "Emulator::load_bank(" << bank_num << ", " << std::hex << page_num << ")" << std::endl;
//	system("pause");
	if (bank_num == 0)
	{
		return;
	}
	int source = Emulator::BANK_SIZE * (page_num - 0xE0);
	int destination = 0x8000 + Emulator::BANK_SIZE * (bank_num - 1);
	for (int i = 0; i < Emulator::BANK_SIZE; i++)
		state.ram[destination + i] = rom[source + i];
}

void Emulator::savestate(Node* n)
{
	std::cout << "Emulator: savestate" << std::endl;
	if (n->state == nullptr)
		n->state = new State;

	n->state->f.c = state.f.c;
	n->state->f.z = state.f.z;
	n->state->f.i = state.f.i;
	n->state->f.d = state.f.d;
	n->state->f.v = state.f.v;
	n->state->f.n = state.f.n;

	n->state->A = state.A;
	n->state->X = state.X;
	n->state->Y = state.Y;

	n->state->SP = state.SP;
	n->state->PC = state.PC;

	n->state->frame = state.frame;

	std::cout << "State saved at PC addr: " << std::hex << (unsigned int)n->state->PC << std::endl;

	for (int i = 0; i < Emulator::RAM_SIZE; i++)
		n->state->ram[i] = state.ram[i];
}

void Emulator::loadstate(Node* n)
{
	std::cout << "Emulator::loadstate" << std::endl;
	state.f.c = n->state->f.c;
	state.f.z = n->state->f.z;
	state.f.i = n->state->f.i;
	state.f.d = n->state->f.d;
	state.f.v = n->state->f.v;
	state.f.n = n->state->f.n;

	state.A = n->state->A;
	state.X = n->state->X;
	state.Y = n->state->Y;

	state.SP = n->state->SP;
	state.PC = n->state->PC;

	state.frame = n->state->frame;

	for (int i = 0; i < Emulator::RAM_SIZE; i++)
		state.ram[i] = n->state->ram[i];
}

void Emulator::setBreak(std::string type, word addr)
{
	std::cout << "Emulator::setBreak(" << type << ", " << std::hex << addr << ")" << std::endl;
}

bool Emulator::isBreak()
{
	return false;
}

void Emulator::push_byte(byte val)
{
	State* s = &state;
	s->ram[0x100 + (s->SP)--] = val;
}

void Emulator::push_word(word val)
{
	State* s = &state;

	byte high = (val & 0xFF00)>>8;
	byte low = val & 0x00FF;
	s->ram[0x100 + (s->SP)--] = high;
	s->ram[0x100 + (s->SP)--] = low;
}

byte Emulator::pull_byte()
{
	State* s = &state;
	byte val = s->ram[0x100 + ++(s->SP)];
	return val;
}

word Emulator::pull_word()
{
	State* s = &state;
	word val = s->ram[0x100 + ++(s->SP)];
	val += 256* s->ram[0x100 + ++(s->SP)];
	return val;
}

bool Emulator::is_multiply()
{
//	std::cout << "Multiply detected" << std::endl;
//	system("pause");
	State* s = &state;
	int test = s->ram[s->PC] + 256 * s->ram[s->PC + 1];
	if (test == 0x5206)
	{
		std::cout << "Multiply detected" << std::endl;
		system("pause");
		return true;
	}
		
	return false;
}

bool Emulator::is_switch()
{
	State* s = &state;
	int test = s->ram[s->PC] + 256 * s->ram[s->PC + 1];
	if (test >= 0x5113 && test <= 0x5117)
		return true;
	return false;
}

void Emulator::run()
{
	std::cout << "Emulator::run" << std::endl;
	State* s = &state;
	bool flagHit = false;
	bool m = false;
//	if (s->frame == 1) flagHit = true;
	do
	{

//		if (s->ram[0x03] >= 6)
//		{
//			std::cout << "Error @ " << "Stack ptr: " << std::hex << (unsigned int)s->PC << std::endl;
//			system("pause");
//		}

		if (false)
//		if (m || flagHit)
		{
			std::cout << "@ " << std::hex << (unsigned int)s->PC << std::endl;
			int sp = 0x100 + s->SP;
			std::cout << "Stack ptr: " << std::hex << (unsigned int)sp << std::endl;
			std::cout << "Stack: " << std::endl;
			for (int i = sp; i < 0x1FF; i++)
				std::cout << std::hex << (unsigned int)s->ram[i + 1] << " ";
			std::cout << std::endl;
			std::cout << "Zero page: " << std::endl;
			for (int i = 0; i < 0x10; i++)
				std::cout << std::hex << (unsigned int)s->ram[i] << " ";
			std::cout << std::endl;
			std::cout << "A: " << std::hex << (unsigned int)s->A << std::endl;
			std::cout << "X: " << std::hex << (unsigned int)s->X << std::endl;
			std::cout << "Y: " << std::hex << (unsigned int)s->Y << std::endl;
//			system("pause");
			flagHit = true;
			system("pause");
		}

		

//		std::cout << "PC: " << std::hex << (unsigned int)s->PC << std::endl;
//		std::cout << "E0EF: " << std::hex << (unsigned int)s->ram[0xE0EF] << std::endl;
		if (s->PC == 0xE517)
		{
			int vm_addr;
			vm_addr = s->ram[0x06] + 256 * s->ram[0x07];
//			std::cout << "Executing VM instruction at " << std::hex << vm_addr << ": " << std::hex << (unsigned int)s->ram[vm_addr] << std::endl;
		}

		byte cur_op = s->ram[(s->PC++)];
		/*
		if (flagHit)
		{
			std::cout << std::hex << (unsigned int)(s->PC - 1) << ": " << std::hex << (unsigned int)(cur_op) << std::endl;
			std::cout << "A: " << std::hex << (unsigned int)s->A << std::endl;
			std::cout << "X: " << std::hex << (unsigned int)s->X << std::endl;
			std::cout << "Y: " << std::hex << (unsigned int)s->Y << std::endl;
			std::cout << "$02 == " << std::hex << (unsigned int)s->ram[0x02] + 256 * (unsigned int)s->ram[0x03] << std::endl;
			//std::cout << std::hex << (unsigned int)s->ram[0x00] << " " << std::hex << (unsigned int)s->ram[0x01] << std::endl;
			//std::cout << "X = " << std::hex << (unsigned int)s->X << std::endl;
		}
		*/

		word dest;
		byte val;
		word prev;
		int sum;
		switch (cur_op)
		{
			//BIT (test BITs)
		case(0x24): // BIT $44
			dest = s->ram[(s->PC++)];
			val = s->ram[dest];
			s->f.n = !!(val & 0x80);
			s->f.v = !!(val & 0x40);
			s->f.z = s->A & val;
			break;
		case(0x2C): // BIT $4400
			dest = s->ram[(s->PC++)];
			dest += 256 * s->ram[(s->PC++)];
			val = s->ram[dest];
			s->f.n = !!(val & 0x80);
			s->f.v = !!(val & 0x40);
			s->f.z = s->A & val;
			break;

			//NOP (No OPeration)
		case(0xEA):
			break;

			//Register Instructions
		case (0xAA): // TAX
			val = s->A;
			s->X = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case (0x8A): // TXA
			val = s->X;
			s->A = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case (0xCA): // DEX
			val = s->X - 1;
			s->X = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case (0xE8): // INX
			val = s->X + 1;
			s->X = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;

		case (0xA8): // TAY
			val = s->A;
			s->Y = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case (0x98): // TYA
			val = s->Y;
			s->A = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case (0x88): // DEY
			val = s->Y - 1;
			s->Y = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case (0xC8): // INY
			val = s->Y + 1;
			s->Y = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;

		//INC (INCremement memory)
		case(0xE6): // INC $44
			dest = s->ram[(s->PC++)];
			val = s->ram[dest] + 1;
			s->ram[dest] = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case(0xF6): // INC $44,X
			dest = s->ram[(s->PC++)];
			dest += s->X;
			val = s->ram[dest] + 1;
			s->ram[dest] = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case(0xEE): // INC $4400
			dest = s->ram[(s->PC++)];
			dest += 256*s->ram[(s->PC++)];
			val = s->ram[dest] + 1;
			s->ram[dest] = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case(0xFE): // INC $4400,X
			dest = s->ram[(s->PC++)];
			dest += 256 * s->ram[(s->PC++)];
			dest += s->X;
			val = s->ram[dest] + 1;
			s->ram[dest] = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;

		//DEC (DECremement memory)
		case(0xC6): // DEC $44
			dest = s->ram[(s->PC++)];
			val = s->ram[dest] - 1;
			s->ram[dest] = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case(0xD6): // DEC $44,X
			dest = s->ram[(s->PC++)];
			dest += s->X;
			val = s->ram[dest] - 1;
			s->ram[dest] = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case(0xCE): // DEC $4400
			dest = s->ram[(s->PC++)];
			dest += 256 * s->ram[(s->PC++)];
			val = s->ram[dest] - 1;
			s->ram[dest] = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case(0xDE): // DEC $4400,X
			dest = s->ram[(s->PC++)];
			dest += 256 * s->ram[(s->PC++)];
			dest += s->X;
			val = s->ram[dest] - 1;
			s->ram[dest] = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;

		//AND
		case(0x29): // AND #$44
			val = s->ram[(s->PC++)];
			val &= s->A;
			s->A = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case(0x25): // AND $44
			dest = s->ram[(s->PC++)];
			val = s->ram[dest];
			val &= s->A;
			s->A = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;

		//ORA
		case(0x09): // ORA #$44
			val = s->ram[(s->PC++)];
			val |= s->A;
			s->A = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case(0x05): // ORA $44
			dest = s->ram[(s->PC++)];
			val = s->ram[dest];
			val |= s->A;
			s->A = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case(0x0D): // ORA $4400
			dest = s->ram[(s->PC++)];
			dest += 256 * s->ram[(s->PC++)];
			val = s->ram[dest];
			val |= s->A;
			s->A = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;

		//ASL
		case(0x0A): // ASL A
			s->f.c = !!(s->A & 0x80);
			val = ((s->A) << 1);
			s->A = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case(0x06): // ASL $44
			dest = s->ram[(s->PC++)];
			val = s->ram[dest];
			s->f.c = !!(val & 0x80);
			val = (val << 1);
			s->ram[dest] = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;

		//ROL
		case(0x26): // ROL $44
			dest = s->ram[(s->PC++)];
			val = s->ram[dest];
			sum = (val << 1) + s->f.c;
			s->f.c = !!(sum & 0x100);
			val = sum & 0xFF;
			s->ram[dest] = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		
		//ADC (ADd with Carry)
		case(0x69): // ADC #$44
			val = s->ram[(s->PC++)];
			if (s->f.c) val++;
			sum = s->A + val;
			(s->f.c) = (sum > 0xFF);
			(s->f.v) = (sum > 0xFF);

			val = s->A + val;
			s->A = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case(0x65): // ADC $44
			dest = s->ram[(s->PC++)];
			val = s->ram[dest];
			if (s->f.c) val++;
			sum = s->A + val;
			(s->f.c) = (sum > 0xFF);
			(s->f.v) = (sum > 0xFF);

			val = s->A + val;
			s->A = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case(0x75): // ADC $44,X
			dest = s->ram[(s->PC++)];
			dest += s->X;
			val = s->ram[dest];
			if (s->f.c) val++;
			sum = s->A + val;
			(s->f.c) = (sum > 0xFF);
			(s->f.v) = (sum > 0xFF);

			val = s->A + val;
			s->A = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case(0x6D): // ADC $4400
			dest = s->ram[(s->PC++)];
			dest += 256*s->ram[(s->PC++)];
			val = s->ram[dest];
			if (s->f.c) val++;
			sum = s->A + val;
			(s->f.c) = (sum > 0xFF);
			(s->f.v) = (sum > 0xFF);

			val = s->A + val;
			s->A = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case(0x7D): // ADC $4400,X
			dest = s->ram[(s->PC++)];
			dest += 256 * s->ram[(s->PC++)];
			dest += s->X;
			val = s->ram[dest];
			if (s->f.c) val++;
			sum = s->A + val;
			(s->f.c) = (sum > 0xFF);
			(s->f.v) = (sum > 0xFF);

			val = s->A + val;
			s->A = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case(0x79): // ADC $4400,Y
			dest = s->ram[(s->PC++)];
			dest += 256 * s->ram[(s->PC++)];
			dest += s->Y;
			val = s->ram[dest];
			if (s->f.c) val++;
			sum = s->A + val;
			(s->f.c) = (sum > 0xFF);
			(s->f.v) = (sum > 0xFF);

			val = s->A + val;
			s->A = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case(0x61): // ADC ($44,X)
			sum = s->ram[(s->PC++)];
			sum += s->X;
			dest = s->ram[sum] + 256 * s->ram[sum + 1];
			val = s->ram[dest];
			if (s->f.c) val++;
			sum = s->A + val;
			(s->f.c) = (sum > 0xFF);
			(s->f.v) = (sum > 0xFF);

			val = s->A + val;
			s->A = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case(0x71): // ADC ($44),Y
//			std::cout << "0x71: ADC ($";
			sum = s->ram[(s->PC++)];
//			std::cout << std::hex << (unsigned int)sum << "),Y";
			dest = s->ram[sum] + 256 * s->ram[sum + 1];
			dest += s->Y;
			val = s->ram[dest];
//			std::cout << "@ " << std::hex << (unsigned int)dest << " = " << std::hex << (unsigned int)val << std::endl;
			if (s->f.c) val++;
			sum = s->A + val;
			(s->f.c) = (sum > 0xFF);
			(s->f.v) = (sum > 0xFF);

			val = s->A + val;
			s->A = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;

		//SBC
		case(0xE9): // SBC #$44
			val = s->ram[(s->PC++)];
			if (!s->f.c) val++;
			s->f.v = (val > s->A);
			val = s->A - val;
			s->A = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			s->f.c = true;
			break;
		case(0xE5): //  SBC $44
			dest = s->ram[(s->PC++)];
			val = s->ram[dest];
			if (!s->f.c) val++;
			s->f.v = (val > s->A);
			val = s->A - val;
			s->A = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			s->f.c = true;
			break;
		case(0xE1): // SBC ($44,X)
			sum = s->ram[(s->PC++)];
			sum += s->X;
			dest = s->ram[sum] + 256 * s->ram[sum + 1];
			val = s->ram[dest];
			if (!s->f.c) val++;
			s->f.v = (val > s->A);
			val = s->A - val;
			s->A = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			s->f.c = true;
			break;
		case(0xF1): // SBC ($44),Y
			sum = s->ram[(s->PC++)];
			dest = s->ram[sum] + 256 * s->ram[sum + 1];
			dest += s->Y;
			val = s->ram[dest];
			if (!s->f.c) val++;
			s->f.v = (val > s->A);
			val = s->A - val;
			s->A = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			s->f.c = true;
			break;

		//Stack Instructions
		case(0x9A): // TXS
			s->SP = s->X;
			break;
		case(0xBA): // TSX
			s->X = s->SP;
			break;
		case(0x48): // PHA
			push_byte(s->A);
			break;
		case(0x68): // PLA
			s->A = pull_byte();
			break;

		//JSR (Jump to SubRoutine)
		case(0x20): // JSR
			dest = s->ram[(s->PC++)];
			dest += 256 * s->ram[(s->PC++)];
			prev = s->PC - 1;
			push_word(prev);
			s->PC = dest;
			break;

		// RTS (ReTurn from Subroutine)
		case(0x60): // RTS
			dest = pull_word() + 1;
			s->PC = dest;
			break;

		// RTI (ReTurn from Interrupt)
		case(0x40): // RTI
			val = pull_byte();
			s->f.c == !!(val & 0x01);
			s->f.z == !!(val & 0x02);
			s->f.i == !!(val & 0x04);
			s->f.d == !!(val & 0x08);
			s->f.v == !!(val & 0x40);
			s->f.n == !!(val & 0x80);
			dest = pull_word();
			s->PC = dest;
			break;

		// JMP
		case (0x4C): // JMP $5597
			dest = s->ram[(s->PC++)];
			dest += 256 * s->ram[(s->PC++)];
			s->PC = dest;
			break;
		case (0x6C): // JMP ($5597)
			
			dest = s->ram[(s->PC++)];
			dest += 256 * s->ram[(s->PC++)];

			sum = s->ram[dest] + 256* s->ram[dest + 1];
			s->PC = sum;
			break;

		//BRANCH
		case(0x10): // BPL
			dest = s->ram[(s->PC++)];
			if (!s->f.n) s->PC += (int8_t)dest;
			break;
		case(0x30): // BMI
			dest = s->ram[(s->PC++)];
			if (s->f.n) s->PC += (int8_t)dest;
			break;
		case(0x50): // BVC
			dest = s->ram[(s->PC++)];
			if (!s->f.v) s->PC += (int8_t)dest;
			break;
		case(0x70): // BVS
			dest = s->ram[(s->PC++)];
			if (s->f.v) s->PC += (int8_t)dest;
			break;
		case(0x90): // BCC
			dest = s->ram[(s->PC++)];
			if (!s->f.c) s->PC += (int8_t)dest;
			break;
		case(0xB0): // BCS
			dest = s->ram[(s->PC++)];
			if (s->f.c) s->PC += (int8_t)dest;
			break;
		case(0xD0): // BNE
			dest = s->ram[(s->PC++)];
			if (!s->f.z) s->PC += (int8_t)dest;
			break;
		case(0xF0): // BEQ
			dest = s->ram[(s->PC++)];
			if (s->f.z) s->PC += (int8_t)dest;
			break;

			int hp;


		//CMP (CoMPare accumulator)
		case (0xC9): // CMP #$44
			val = s->ram[(s->PC++)];
			s->f.c = (s->A >= val);
			s->f.z = (s->A == val);
			s->f.n = !!(s->A & 0x80);
			break;
		case (0xC5): // CMP $44
			dest = s->ram[(s->PC++)];
			val = s->ram[dest];
			s->f.c = (s->A >= val);
			s->f.z = (s->A == val);
			s->f.n = !!(s->A & 0x80);
			break;
		case (0xCD): // CMP $4400
			dest = s->ram[(s->PC++)];
			dest += 256 * s->ram[(s->PC++)];
			val = s->ram[dest];
			s->f.c = (s->A >= val);
			s->f.z = (s->A == val);
			s->f.n = !!(s->A & 0x80);
			break;

		//CPX (ComPare X register)
		case (0xE0): // CPX #$44
			val = s->ram[(s->PC++)];
			s->f.c = (s->X >= val);
			s->f.z = (s->X == val);
			s->f.n = !!(s->X & 0x80);
			break;

		//CPX (ComPare Y register)
		case (0xC0): // CPY #$44
			val = s->ram[(s->PC++)];
			s->f.c = (s->Y >= val);
			s->f.z = (s->Y == val);
			s->f.n = !!(s->Y & 0x80);
			break;

		//LDA (LoaD Accumulator)
		case(0xA9): // LDA #$44
			val = s->ram[(s->PC++)];
			s->A = val;
			s->f.n = !!(val &  0x80);
			s->f.z =   (val == 0x00);
			break;
		case(0xA5): // LDA $44
			dest = s->ram[(s->PC++)];
			val = s->ram[dest];
			s->A = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case(0xB5): // LDA $44,X
			dest = s->ram[(s->PC++)];
			dest += s->X;
			val = s->ram[dest];
			s->A = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case(0xAD): // LDA $4400
			dest = s->ram[(s->PC++)];
			dest += 256*s->ram[(s->PC++)];
			val = s->ram[dest];
			s->A = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case(0xBD): // LDA $4400,X
			dest = s->ram[(s->PC++)];
			dest += 256 * s->ram[(s->PC++)];
			dest += s->X;
			val = s->ram[dest];
			s->A = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case(0xB9): // LDA $4400,Y
			dest = s->ram[(s->PC++)];
			dest += 256 * s->ram[(s->PC++)];
			dest += s->Y;
			val = s->ram[dest];
			s->A = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case(0xA1): // LDA ($44,X)
			sum = s->ram[(s->PC++)];
			sum += s->X;
			dest = s->ram[sum] + 256*s->ram[sum +1];
			val = s->ram[dest];
			s->A = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case(0xB1): // LDA ($44),Y
			sum = s->ram[(s->PC++)];
			dest = s->ram[sum] + 256*s->ram[sum +1];
			dest += s->Y;
			val = s->ram[dest];
			s->A = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;

		//LDX (LoaD X register)
		case(0xA2): // LDX #$44
			val = s->ram[(s->PC++)];
			s->X = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case(0xA6): // LDX $44
			dest = s->ram[(s->PC++)];
			val = s->ram[dest];
			s->X = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;

		//LDY (LoaD Y register)
		case(0xA0): // LDY #$44
			val = s->ram[(s->PC++)];
			s->Y = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;
		case(0xA4): // LDY #$44
			dest = s->ram[(s->PC++)];
			val = s->ram[dest];
			s->Y = val;
			s->f.n = !!(val & 0x80);
			s->f.z = (val == 0x00);
			break;

		//STA (STore Accumulator)
		case(0x85): // STA $44
			dest = s->ram[(s->PC++)];
			s->ram[dest] = s->A;
			if (dest == 0x02)
			{
//				std::cout << std::hex << (unsigned int)s->PC << ": " << std::hex << (unsigned int)s->A << " written to $02" << std::endl;
//				system("pause");
			}
			break;
		case(0x95): // STA $44,X
			dest = s->ram[(s->PC++)];
			dest += s->X;
			s->ram[dest] = s->A;
			break;
		case(0x8D): // STA $4400

			m = is_multiply();

			if (is_switch())
			{
				dest = s->ram[(s->PC++)];
				dest += 256 * s->ram[(s->PC++)];
				load_bank(dest - 0x5113, s->A);
				break;
			}

			dest = s->ram[(s->PC++)];
			dest += 256 * s->ram[(s->PC++)];
			s->ram[dest] = s->A;

			if (m)
			{
				sum = ((unsigned int)s->ram[0x5205] * (unsigned int)s->ram[0x5206]) & 0xFFFF;
				s->ram[0x5205] = (sum & 0xFF00) >> 2;
				s->ram[0x5206] = sum & 0xFF;
			}

			break;
		case(0x9D): // STA $4400,X
			if (is_switch())
			{
				dest = s->ram[(s->PC++)];
				dest += 256 * s->ram[(s->PC++)];
				dest += s->X;
				load_bank(dest - 0x5113, s->A);
				break;
			}

			dest = s->ram[(s->PC++)];
			dest += 256 * s->ram[(s->PC++)];
			dest += s->X;
			s->ram[dest] = s->A;
			break;
		case(0x99): // STA $4400,Y
			if (is_switch())
			{
				dest = s->ram[(s->PC++)];
				dest += 256 * s->ram[(s->PC++)];
				dest += s->Y;
				load_bank(dest - 0x5113, s->A);
				break;
			}

			dest = s->ram[(s->PC++)];
			dest += 256 * s->ram[(s->PC++)];
			dest += s->Y;
			s->ram[dest] = s->A;
			break;
		case(0x81): // STA ($44,X)
			sum = s->ram[(s->PC++)];
			sum += s->X;
			dest = s->ram[sum] + 256*s->ram[sum+1];
			s->ram[dest] = s->A;
			if (dest == 0x02)
			{
//				std::cout << std::hex << (unsigned int)s->PC << ": " << std::hex << (unsigned int)s->A << " written to $02" << std::endl;
//				system("pause");
			}
			break;
		case(0x91): // STA ($44),Y
			sum = s->ram[(s->PC++)];
			dest = s->ram[sum] + 256*s->ram[sum +1];
			dest += s->Y;
			s->ram[dest] = s->A;
			if (dest == 0x02)
			{
//				std::cout << std::hex << (unsigned int)s->PC << ": " << std::hex << (unsigned int)s->A << " written to $02" << std::endl;
//				system("pause");
			}
			break;

		//STX
		case(0x86): // STX $44
			dest = s->ram[(s->PC++)];
			s->ram[dest] = s->X;
			break;
		case(0x8E): // STX $4400

			m = is_multiply();

			if (is_switch())
			{
				dest = s->ram[(s->PC++)];
				dest += 256 * s->ram[(s->PC++)];
				load_bank(dest - 0x5113, s->A);
				break;
			}

			dest = s->ram[(s->PC++)];
			dest += 256 * s->ram[(s->PC++)];
			s->ram[dest] = s->X;

			if (m)
			{
				sum = ((unsigned int)s->ram[0x5205] * (unsigned int)s->ram[0x5206]) & 0xFFFF;
				s->ram[0x5205] = (sum & 0xFF00) >> 2;
				s->ram[0x5206] = sum & 0xFF;
			}

			break;

		//STY
		case(0x84): // STY $44
			dest = s->ram[(s->PC++)];
			s->ram[dest] = s->Y;
			break;

		//Flag (Processor Status) Instructions
		case(0x18): // CLC
			s->f.c = false;
			break;
		case(0x38): // SEC
			s->f.c = true;
			break;
		case(0x58): // CLI
			s->f.i = true;
			break;
		case(0x78): // SEI
			s->f.i = false;
			break;
		case(0xB8): // CLV
			s->f.v = false;
			break;
		case(0xD8): // CLD
			s->f.d = false;
			break;
		case(0xF8): // SED
			s->f.d = true;
			break;

		case(0x08): // PHP
			val = 0x00;
			if (s->f.c) val += 0x01;
			if (s->f.z) val += 0x02;
			if (s->f.i) val += 0x04;
			if (s->f.d) val += 0x08;
			if (s->f.v) val += 0x40;
			if (s->f.n) val += 0x80;
			push_byte(val);
			break;
		case(0x28): // PLP
			val = pull_byte();
			s->f.c == !!(val & 0x01);
			s->f.z == !!(val & 0x02);
			s->f.i == !!(val & 0x04);
			s->f.d == !!(val & 0x08);
			s->f.v == !!(val & 0x40);
			s->f.n == !!(val & 0x80);
			break;


		default:
			std::cout << "Unknown opcode at address " << std::hex << (unsigned int)((s->PC) - 1) << ": " << std::hex << (unsigned int)cur_op << std::endl;
			assert(false);
		}
	} while (s->PC != 0xFAC8 && s->PC != 0xFCA3 && s->PC != 0xFB66);

	std::cout << "Finished run w/ emu @ " << this << std::endl;
	std::cout << "Previous frame count: " << s->frame << std::endl;
	s->frame++;
	std::cout << "New frame count: " << s->frame << std::endl;
}