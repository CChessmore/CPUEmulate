#include <stdio.h>
#include <stdlib.h>

/*
* Based on a video walkthrough tutorial by Dave Poo.
* This program approximately emulates a 6502 microprocessor from ~40 years ago.
*
* This project is done with the intent to get a guided experience through
* more complex code than previous self-driven projects and dip my toes into
* simulation and emulation.
*
* Start Date: 11/6/2023
* Completion Date:
*/


//Defining types the processor will use/need to recognize
using Byte = unsigned char;
using Word = unsigned short;
using u32 = unsigned int;

struct Memory
{
	static constexpr u32 MAX_MEM = 1024 * 64;
	Byte Data[MAX_MEM];

	//Function to initialize memory
	void Init()
	{
		//Change all possible memory bits to zero
		for (u32 i = 0; i < MAX_MEM; i++)
		{
			Data[i] = 0;
		}
	}

	//Read 1 byte from memory
	Byte operator[](u32 Address) const
	{
		//Address should always be < MAX_MEM
		return Data[Address];
	}

	Byte& operator[] (u32 Address)
	{
		return Data[Address];
	}

	void WriteWord(u32 Cycles, Word Value, u32& Address)
	{
		Data[Address] = Value & 0xFF;
		Data[Address + 1] = (Value >> 8);
		Cycles -= 2;
	}
};

struct CPU
{


	//Program Counter
	Word PC;
	//Stack Pointer
	Byte SP;

	//Accumulator
	Byte A;
	//Index Registers X and Y
	Byte X, Y;

	//Status Flag initialization start
	Byte C : 1;
	Byte Z : 1;
	Byte I : 1;
	Byte D : 1;
	Byte B : 1;
	Byte V : 1;
	Byte N : 1;
	//Status Flag initialization end

	//Reset function
	void reset(Memory mem)
	{
		//Reset Program Counter
		PC = 0xFFFC;
		//Reset Stack Pointer
		SP = 0x0100;
		//Clear all status flags
		C = Z = I = D = B = V = N = 0;
		//Clear Accumulator and other registers
		A = X = Y = 0;

		mem.Init();
	}

	Byte FetchByte(u32& cpuCycles, Memory& memory)
	{
		Byte Data = memory[PC];
		PC++;
		cpuCycles--;
		return Data;
	}

	Byte ReadByte (u32& Cycles, Byte Address, Memory& memory )
	{
		Byte Data = memory[Address];
		Cycles--;
		return Data;
	}

	Word FetchWord( u32& Cycles, Memory& memory )
	{
		//The 6502 processor is little endian
		Word Data = memory[PC];
		PC++;

		//Bitwise or the memory[PC] value, shifted left 8 bits
		Data |= (memory[PC] << 8);
		PC++;

		//Cycles -2 here, since we've done 2 things
		Cycles -= 2;

		return Data;
	}



	//Operation codes
	static constexpr Byte
		INS_LDA_IM = 0xA9,
		INS_LDA_ZP = 0xA5,
		INS_LDA_ZPX = 0xB5,
		INS_JSR = 0x20;

	void LDASetStatus()
	{
		Z = (A == 0);
		N = (A & 0b10000000) > 0;
	}

	void Execute( u32 cpuCycles, Memory& memory)
	{

		while (cpuCycles > 0)
		{
			Byte Inst = FetchByte(cpuCycles, memory);
			switch (Inst)
			{
				case INS_LDA_IM:
				{
					Byte Value = FetchByte(cpuCycles, memory);
					A = Value;
					LDASetStatus();

				} break;

				case INS_LDA_ZP:
				{
					Byte ZeroPageAddress = FetchByte(cpuCycles, memory);

					A = ReadByte(cpuCycles, ZeroPageAddress, memory);
					LDASetStatus();

				}break;

				case INS_LDA_ZPX:
				{
					Byte ZeroPageAddress = FetchByte(cpuCycles, memory);
					ZeroPageAddress += X;
					cpuCycles--;
					A = ReadByte(cpuCycles, ZeroPageAddress, memory);
					LDASetStatus();
				}

				case INS_JSR:
				{
					Word SubAddress = FetchWord(cpuCycles, memory);
					
					memory.WriteWord(PC-1, SP, cpuCycles);

					PC = SubAddress;
					cpuCycles--;
				}

				default:
				{
					printf("Instruction not handled %d", Inst);
				} 

			}
		}

	}

};





int main()
{

	CPU myCPU;
	Memory memory;

	u32 cycles = 3;

	myCPU.reset( memory );

	//Inline code for testing; loads Accumulator (A) with 42
	memory[0xFFFC] = CPU::INS_LDA_ZP;
	memory[0xFFFD] = 0x42;
	memory[0x0042] = 0x84;
	//End of Inline code

	myCPU.Execute( cycles, memory );
	return 0;
}