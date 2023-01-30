#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "1bitvm.h"


uint16_t oldPC;
uint16_t r_instruction; // prebrana isntrukcija xd  
unsigned char OutputBuffer;
unsigned char OutputBufferCounter;
unsigned char InputBuffer;
unsigned char LastCharacter = '\n';
unsigned char InputBufferCounter;
unsigned char InputForce;
FILE *file_ptr;
instruction * instructions;
int ram[BITS];
unsigned long long int counter = 0; // type and a half

int main(int argc, char *argv[]) {
	file_ptr = fopen(argv[1], "rb");
	if(file_ptr == NULL) {
		fprintf(stderr,"Supplied file is invalid\n");
		return(10);
	}
	instructions = (instruction*)malloc(sizeof(instruction) * 32768);
#if DEBUG > 2
	fprintf(stderr, "r_instruction    PC     m o adr0 adr1\n");
#endif
	read_instructions();
	int return_value = loop();
#if DEBUG
	fprintf(stderr, "%llu instructions elapsed\n", counter);
	if(return_value == 0) fprintf(stderr, "Exited gracefully\n");
	if(return_value == 1) fprintf(stderr, "Execution limit reached\n");
	if(return_value == 2) fprintf(stderr, "EOF on stdin\n");
#endif
	return return_value;

}
int loop() {
	long long limit = LIMIT;
	instruction ins;
	while (limit > 0 || limit == -1) {
		ins = instructions[get_PC()];
		#if DEBUG > 2
			fprintf(stderr, "%016b 0x%04x %b %b 0x%02x 0x%02x\n",r_instruction, get_PC(), ins.meta, ins.opp, ins.adr0, ins.adr1);
		#endif
		oldPC = inc_PC() - 1; //checking for same PC, inc_PC/0 returns the incremented PC, so we decrement it
		if(ins.opp) {
			if(ins.meta)
				xor(ins.adr0, ins.adr1);
			else
				nand(ins.adr0, ins.adr1);
		} else {
			if(ins.meta)
				copy2reg(ins.adr0, ins.adr1);
			else
				copy16bits(ins.adr0, ins.adr1);
		}
		if(ins.adr0 == IN_A || ins.adr1 == IN_A) InputForce = 1;
		if(do_IO()) return 2; // EOF on stdin
		if(oldPC == get_PC())
			return 0; // exit reached in programm
#if LIMIT > 0	
		limit--;
#endif
#if DEBUG > 0
		counter++;
#endif
	}
	return 1; // execution limit reached
}
int read_instruction(instruction * a, int PC) {
	fseek(file_ptr,PC*2, SEEK_SET); 
	int e = fread(&r_instruction, 2, 1, file_ptr);
	r_instruction = (r_instruction >> 8) | (r_instruction << 8); //swap byte order TODO: make this platform independant
	if(!e) r_instruction = 0;
	a->meta = r_instruction & 0x1;
	a->opp = (r_instruction >> 1) & 0x1;
	a->adr0 = (r_instruction >> 9) & 0x7f;
	a->adr1 = (r_instruction >> 2) & 0x7f;
	a->raw_instruction = r_instruction;
	return e;
}
void read_instructions() {
	instruction a;
	for(int i = 0; i < 32768; i++) {
		if(!read_instruction(&a, i)) break;
		instructions[i] = a;
	}
}
void copy16bits(unsigned char src, unsigned char dst) {
	// TODO: test overflow
	int buffer [16];
	for(int i = 0; i < 16; i++) {
		buffer[i] = ram[(i+src)%BITS];
	}
	for(int i = 0; i < 16; i++) {
		ram[(i+dst)%BITS] = buffer[i];
	}
	
}
void copy2reg(unsigned char src, unsigned char dst) {
	uint16_t buffer = instructions[src].raw_instruction;
	for(int i = 15; i >= 0; i--) {
		ram[(dst+i)%BITS] = buffer & 1;
		buffer = buffer >> 1;
	}
}
void nand(unsigned char src, unsigned char dst) {
	ram[dst] = !(ram[src] && ram[dst]);
}
void xor(unsigned char src, unsigned char dst) {
	ram[dst] = (!ram[src] != !ram[dst]);
}
int do_IO() {
	if(ram[OUT_A]) {
		OutputBuffer = OutputBuffer << 1 | (ram[OUT] & 1);
		OutputBufferCounter++;
		ram[OUT_A] = 0;
	}
	if(!ram[IN_A] && InputForce) {
		if(InputBufferCounter == 0) {
			fprintf(stdout, ">");
			int InputBufferBuffer = getchar(); //we put getchar into int beacause of EOF
			if(InputBufferBuffer == EOF) {
#if EXIT_ON_EOF
				return 1;
#endif
				InputBuffer = 0xff;
			} else {
				InputBuffer = InputBufferBuffer;
			}
			InputBufferCounter = 8;
			InputBuffer = reverse(InputBuffer); // reverse bit order
		}
		ram[IN] = InputBuffer & 1;
		InputBuffer = InputBuffer >> 1;
		InputBufferCounter--;
		ram[IN_A] = 1;
		InputForce = 0;
		
	}
	if(OutputBufferCounter > 7) {
#if DEBUG > 0
		fprintf(stdout, "%08b ", OutputBuffer);
#endif
		fprintf(stdout, "%c", OutputBuffer);
#if DEBUG > 0
		fprintf(stdout, "\n");
#endif
		OutputBufferCounter = 0;
	}
	return 0;
}
uint16_t get_PC() {
	uint16_t PC = 0;
	for(int i = 0; i < 16; i++) {
		PC <<= 1;
		PC = PC | (ram[i] & 1);
	}
	return PC;
}
void set_PC(uint16_t PC) {
	for(int i = 15; i >= 0; i--) {
		ram[i] = PC & 1;
		PC >>= 1;
	}
}
uint16_t inc_PC() {
	// returns the incremented PC
	set_PC(get_PC()+1);
	return get_PC();
}
void print_ram() {
	for(int i = 0; i < BITS; i++) {
		fprintf(stderr, "%b", ram[i]);
		if(i%8 == 7) fprintf(stderr, "\n"); // 8 bitov skupi na vrstico
	}	
}
unsigned char reverse(unsigned char b)
{
    return (b * 0x0202020202ULL & 0x010884422010ULL) % 0x3ff;
}
