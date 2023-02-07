#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <arpa/inet.h>

#include "1bitvm.h"
#include "config.h"


uint16_t r_instruction; // prebrana isntrukcija xd  
unsigned char OutputBuffer;
unsigned char OutputBufferCounter;
unsigned char InputBuffer;
unsigned char InputBufferCounter;
unsigned char InputBufferLast = '\n';
unsigned char InputForce;
FILE *file_ptr;
instruction * instructions;
int_fast32_t ram[BITS];
int EOF_reached = 0; 
unsigned long long int counter = 0; // type and a half
unsigned long long int EOF_reached_counter; 

int main(int argc, char *argv[]) {
	file_ptr = fopen(argv[1], "rb");
	if(file_ptr == NULL) {
		fprintf(stderr,"Supplied file is invalid\n");
		return(10);
	}
	instructions = (instruction*)calloc(0xffff,sizeof(instruction));
#if DEBUG > 2
	fprintf(stderr, "PC  \traw_ins\tmo adr0 adr1\n");
#endif
	read_instructions();
	fclose(file_ptr);
	int return_value = loop();
#if DEBUG
	fprintf(stderr, "%llu instructions elapsed.\n", counter);
	if(return_value == 0) fprintf(stderr, "Exited gracefully\n");
	if(return_value == 1) fprintf(stderr, "Execution limit reached\n");
	if(return_value == 2) fprintf(stderr, "EOF on stdin\n");
#endif
	return return_value;

}
int loop() {
#if LIMIT > 0
	long long limit = LIMIT;
#endif
	instruction ins;
	uint16_t oldPC = 0xffff;
	uint16_t PC = 0;
	while (
#if LIMIT > 0
			limit > 0
#else
			1
#endif
			) {
		PC = get_PC();
		if(oldPC == PC)
			return 0; // exit reached in programm
		oldPC = PC; 
		ins = instructions[PC];
		PC++;
		#if DEBUG > 2
			fprintf(stderr, "0x%04x\t0x%04x\t%02b 0x%02x 0x%02x\n", get_PC(),ins.raw_instruction, ins.ins, ins.adr0, ins.adr1);
		#endif
		set_PC(PC); //checking for same PC, inc_PC/0 returns the incremented PC, so we decrement it
		switch(ins.ins) {
			case 0b00: copy16bits(ins.adr0, ins.adr1); break;
			case 0b01: copy2reg(ins.adr0, ins.adr1); break;
			case 0b10: nand(ins.adr0, ins.adr1); break;
			case 0b11: xor(ins.adr0, ins.adr1); break;
		}
		if(ins.adr0 == IN_A || ins.adr1 == IN_A) InputForce = 1;
		if(do_IO()) {
#if EXIT_ON_EOF
			if(counter - EOF_reached_counter > EXIT_ON_EOF_LIMIT) { // TODO: is this overflow safe?
				return 2; // EOF on stdin
			}
#endif
		}
#if LIMIT > 0	
		limit--;
#endif
		counter++;
	}
	return 1; // execution limit reached
}
int read_instruction(instruction * a) {
	int e = fread(&r_instruction, 1, 2, file_ptr);
	r_instruction = htons(r_instruction); //swap byte order to be 		
	if(e == 0) r_instruction = 0;
	a->ins = (unsigned char)(r_instruction & 0x0003);
	a->adr0 = (r_instruction >> 9) & 0x007f;
	a->adr1 = (r_instruction >> 2) & 0x007f;
	a->raw_instruction = r_instruction;
	return e;
}
void read_instructions() {
	for(int i = 0; i < 0xffff; i++) {
		read_instruction(&instructions[i]);
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
		buffer >>= 1;
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
#if STDIN_PROMPT
			if(InputBufferLast == '\n') {
				fprintf(STDIN_PROMPT_STREAM, ">");
			}
			
#endif
			int InputBufferBuffer = getchar(); //we put getchar into int beacause of EOF
			if(InputBufferBuffer == EOF && EOF_reached == 0) {
				EOF_reached = 1;
				EOF_reached_counter = counter;
			}
			InputBuffer = InputBufferBuffer;
			InputBufferLast = InputBuffer;
			InputBufferCounter = 8;
			InputBuffer = reverse(InputBuffer); // reverse bit order
		}
		if(!EOF_reached) {
			ram[IN] = InputBuffer & 1;
			InputBuffer = InputBuffer >> 1;
			InputBufferCounter--;
			ram[IN_A] = 1;
			InputForce = 0;
		}
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
	return EOF_reached;
}
uint16_t get_PC() {
	uint16_t PC = 0;
	for(int i = 0; i < 16; i++) {
		PC <<= 1;
		PC = PC | (ram[i] & 1);
	}
	return PC;
}
uint16_t set_PC(uint16_t PC) {
	for(int i = 15; i >= 0; i--) {
		ram[i] = PC & 1;
		PC >>= 1;
	}
	return PC;
}
uint16_t inc_PC() {
	// returns the incremented PC
	return set_PC(get_PC()+1);
}
unsigned char reverse(unsigned char b){
    return (b * 0x0202020202ULL & 0x010884422010ULL) % 0x3ff;
}
