#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define DEBUG 0
#define LIMIT 10000000

#define BITS 128
#define IN 0x10
#define IN_A 0x11
#define OUT 0x12
#define OUT_A 0x13

int ram [BITS];
typedef struct instruction {
	bool meta;
	bool opp;
	unsigned char adr0;
	unsigned char adr1;
} instruction;

int read_instruction(instruction * ins, int PC);
uint16_t get_PC();
void copy16bits(unsigned char src, unsigned char dst);
void copy2reg(unsigned char src0, unsigned char src1);
void nand(unsigned char src, unsigned char dst);
void xor(unsigned char src, unsigned char dst);
int do_IO();
void set_PC(uint16_t);
int loop();
uint16_t inc_PC();
void print_ram();
unsigned char reverse(unsigned char);

uint16_t oldPC;
uint16_t r_instruction; // prebrana isntrukcija xd  
unsigned char OutputBuffer;
unsigned char OutputBufferCounter;
unsigned char InputBuffer;
unsigned char InputBufferCounter;
unsigned char InputForce;
FILE *ptr;

int main() {
	ptr = fopen("example3.out", "rb");
#if ERROR > 2
	fprintf(stderr, "r_instruction    PC     m o adr0 adr1\n");
#endif
	int v = loop();
	//print_ram();
	return v;

}
int loop() {
	long long limit = LIMIT;
	instruction ins;
	while (limit > 0 || limit == -1) {
		if(!read_instruction(&ins,get_PC())) return 2; // end of program file
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
		if(do_IO()) return 3; // EOF on stdin
		if(oldPC == get_PC())
			return 0; // exit reached in programm
#if LIMIT > 0	
		limit--;
#endif
	}
	return 1; // execution limit reached
}
int read_instruction(instruction * a, int PC) {
	fseek(ptr,PC*2, SEEK_SET); 
	int e = fread(&r_instruction, 2, 1, ptr);
	r_instruction = (r_instruction >> 8) | (r_instruction << 8); //swap byte order TODO: make this platform independant
	if(!e) r_instruction = 0;
	a->meta = r_instruction & 0x1;
	a->opp = (r_instruction >> 1) & 0x1;
	//a->adr0 = (r_instruction >> 9) & 0x7f;
	//a->adr1 = (r_instruction >> 2) & 0x7f;
	a->adr0 = (r_instruction & 0xfe00) >> 9;
	a->adr1 = (r_instruction & 0x01fc) >> 2;
	return e;
}
void copy16bits(unsigned char src, unsigned char dst) {
	int buffer [16];
	for(int i = 0; i < 16; i++) {
		buffer[i] = ram[(i+src)%BITS];
	}
	for(int i = 0; i < 16; i++) {
		ram[(i+dst)%BITS] = buffer[i];
	}
	
}
void copy2reg(unsigned char src, unsigned char dst) {
	uint16_t buffer;
	fseek(ptr, src*2, SEEK_SET);
	fread(&buffer, 2, 1, ptr);
	buffer = (buffer >> 8) | (buffer << 8); //swap byte order TODO: make this platform independant
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
			//fflush(stdout);
			InputBuffer = getchar();
			if(InputBuffer == EOF) return 1;
			
			//fprintf(stdout,"%c", InputBuffer);
			//fflush(stdin);
			InputBufferCounter = 8;
			InputBuffer = reverse(InputBuffer);
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
