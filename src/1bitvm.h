typedef struct instruction {
	uint16_t ins;
	unsigned char adr0;
	unsigned char adr1;
	uint16_t raw_instruction;
} instruction;

int loop();
int read_instruction(instruction * );
void read_instructions();
void copy16bits(unsigned char, unsigned char );
void copy2reg(unsigned char, unsigned char );
void nand(unsigned char, unsigned char );
void xor(unsigned char, unsigned char);
int do_IO();
uint16_t get_PC();
uint16_t set_PC(uint16_t);
uint16_t inc_PC();
void print_ram();
unsigned char reverse(unsigned char);

