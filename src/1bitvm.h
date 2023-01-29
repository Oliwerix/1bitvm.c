
#define DEBUG 0
#define LIMIT -1

#define BITS 128
#define IN 0x10
#define IN_A 0x11
#define OUT 0x12
#define OUT_A 0x13

typedef struct instruction {
	bool meta;
	bool opp;
	unsigned char adr0;
	unsigned char adr1;
	uint16_t raw_instruction;
} instruction;

int read_instruction(instruction * ins, int PC);
void read_instructions();
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

