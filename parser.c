#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "opcodes.c"

/*
 * Instruction Prefix: 0xF0 0xF2 0xF3 0xF3
 * Address-size override prefix: 0x67
 * Operand-size override prefix: 0x66
 * Segment override prefix: 0x2E 0x36 0x3E 0x26 0x64 0x65
 *
 *
 *		80386 Instruction Format
 * INSTR PREF | ADDR PREF | OP-SIZE PREF |  SEG OVRD
 *   0 or 1       0 or 1      0 or 1         0 or 1
 *			NUMBER OF BYTES
 * OPCODE | MODR\M |  SIB | DISPLCEMNT | IMMDT
 * 1 or 2  0 or 1   0 or 1   0,1,2 or 4  0,1,2 or 4
 *			NUMBER OF BYTES
 *
 * MODR/M & SIB Bytes contain indexing type or reg # in
 * instruction and reg to be used  or more info to slct
 * instruction Also the base, index and scale info.
 *
 * 			MODR/M Byte
 * 	           7  6  |  5  4  3  |  2  1  0 
 * 	            MOD    REG/OPCODE	R/M
 * MOD has 2 most significant bits of byte which can form
 * 32 values when combined to R/M field: 8 regs and 24 
 * indexing modes. Reg field specifies a register number
 *  or three more bits of opcode info. The meaning of reg
 *  Field is determined by first (opcode) Byte of 
 *  instruction
 * 	         SIB (SCALE INDEX BASE) BYTE
 * 	        7  6  |  5  4  3  |  2  1  0 
 *     	          SS      INDEX       BASE
 *     	          EG:   [eax*4 + esp]
 *     	     base = esp, scale = 4, index = eax
 *
 *
 *
 *
 * INFO TAKEN FROM 80386 programmers reference manual
 * AT https://pdos.csail.mit.edu/6.828/2008/readings/i386/s17_02.htm
 */
//EAX = 000, ECX = 001, EDX = 010, EBX = 11, ESP = 100, EBP= 101, ESI = 110, EDI = 111
//8, 16, and 32  bit registers
typedef struct reg {
	char val;
	union {
		struct {
			char * name8;
			char * name16;
			char * name32;
		};
		char * names[3];
	};
} reg;

const reg registers[8] = {
	{0x00, "al", "ax", "eax" },
	{0x01, "cl", "cx", "ecx" },
	{0x02, "dl", "dx", "edx" },
	{0x03, "bl", "bx", "ebx" },
	{0x04, "ah", "sp", "esp" },
	{0x05, "ch", "bp", "ebp" },
	{0x06, "sh", "si", "esi" },
	{0x07, "bh", "di", "edi" }
};

/*Step 1). Check if current byte is an instruction byte
 * prefix, if so, then you've got a REP/REPE/REPNE/LOCK prefix
 *
 * STEP 2). Check if current bye is an address size bye (F3, F2, or F0)
 * if so, decode addresses in rest of instruction in 16 bit mode if 
 * currently in 32 bit mode, or decode addresses in 32 bit mode if 
 * currently in 16 bit mode
 *
 *Step 3). Check if current byte is operand size byte (66) if so, 
 *decode immediate operands in 16 bit mode if currently in 32 bit mode
 * or opposite
 *
 * Step 4). Check if current bye is a segment override byte
 * (2E, 36, 3E, 26, 64, 65) if so, use corresponding segment reg for
 * decoding instead of default
 *
 * Step 5). The next byte is the opcode, if the opcode is 0F, then it
 * is an extended opcode, and read the next byte as extended opcode
 *
 * Step 6). Depending on opcode, reading in and decode a Mod R/M byte,
 * a scale index base byte a displacement, and or an immediate value.
 *  The sizes of these fields depend on the opcode, address size override
 *  and operand size overrides previously decoded
 *
 *  Basic:
 *  Step 1: Check if instruction byte prefix
 *  Step 2: Check if address size byte, if so, handle
 *  Step 3: Check if operand size byte, if so, handle
 *  Step 4: Check if segment override byte, if so, handle
 *  Step 5: Decode opcode and check for extended opcode
 *  Step 6: Decode Mod R/M byte or SIB byte a displacement and or immediate value depending on opcode.
 *  Size depends on previous steps
 */
enum segment_regs
{
	CS,
	SS,
	DS,
	ES,
	FS,
	GS
};

int is_prefix(char b)
{
	if (b==0xF0) {

	} else if (b==0xF2) {

	} else if (b==0xF3) {

	}
	return 0;
}
int is_address_size(char b)
{
	return 0;
}

int is_operand_size(char b)
{
	return 0;
}

int is_seg_override(char b)
{
	// 2e = cs 36 = ss 3e = ds 26 = es 64 = fs 65 = gs
	if (b==0x2E) return CS;
	if (b==0x36)  return SS;
	if (b==0x3E) return DS;
	if (b==0x26) return ES;
	if (b==0x64) return FS;
	if (b==0x65) return GS;
	return 0;
		
}
int is_extended_opcode(char b)
{
	return 0;
}
void decode_opcode();

#define BITS_01(b) ((b&0xC0) >> 6)
#define BITS_234(b) (((b&0x38) >> 3))
#define BITS_567(b) (b&0x7)

#define MASK_SIB_SCALE(b) (BITS_01(b))
#define MASK_SIB_INDEX(b) (BITS_234(b))
#define MASK_SIB_BASE(b) (BITS_567(b))

#define MASK_MODRM_MOD(b) (BITS_01(b))
#define MASK_MODRM_REG(b) (BITS_234(b))
#define MASK_MODRM_RM(b) (BITS_567(b))

#define RM_SIB 0x4
#define DISP_ONLY 0x5
#define MOD_INDIRECT_ADDRESS 0x0
#define MOD_ONE_BYTE_DISPLACEMENT 0x1
#define MOD_FOUR_BYTE_DISPLACEMENT 0x2
#define MOD_REG_ADDRESS 0x3

void parse_bin(const char * bin)
{
	int idx = 0;
	char cb = bin[0];
	char buffer[17];
	int current_seg = DS;
	while(1) {
		current_seg = DS;
		//Max size of instruction is 16 bytes but technically it will never reach that
		memset(buffer, '\0', 17);
		strncpy(buffer, bin + idx, 16);
		cb = buffer[0];
		char opcode[2];
		int is = 1;//Instruction size
		if (is_prefix(cb)) {
			is++;
			//Handle
		}
		if (is_address_size(cb)) {
			is++;
		}
		
		if (is_operand_size(cb)) {
			is++;
		}
		int seg = is_seg_override(cb);
		if (seg) {
			current_seg = seg;
			is++;
			
		}
		if (is_extended_opcode(cb)) {
			is++;
			opcode[0] = buffer[is-1];
			opcode[1] = buffer[is];
		}
		idx += is;
	}
}

void decode_sib(unsigned char b, unsigned char * index, unsigned char * base, int * scale)
{
	int s = MASK_SIB_SCALE(b);
	switch (s) {
		case 0:
			*scale = 1;
			break;
		case 1:
			*scale = 2;
			break;
		case 2:
			*scale = 4;
			break;
		case 3:
			*scale = 8;
			break;
		default:
			*scale = 1;
			break;
	}
	*index = MASK_SIB_INDEX(b);
	*base = MASK_SIB_BASE(b);
}
void print_hex(unsigned char v)
{
	//Vo=2^N-Vn
	//-Vo+2^N=Vn
	int p = !(v&0x40);
	signed char vn = !p ? 256 - v : v; 
	if (p) printf("+0x%x", vn);
	else printf("-0x%x", vn);
}
//Decodes MOD and RM field plus SIB byte and Displacement, returns total bytes used
int decode_rm(unsigned char * cb, int size)
{
	int offset = 0;
	unsigned char mod, rm;
	mod = MASK_MODRM_MOD(*cb);
	rm = MASK_MODRM_RM(*cb);

	unsigned char index, base;
	int scale;
	int sib = rm == RM_SIB;
	offset += sib && (mod!=MOD_REG_ADDRESS);
	decode_sib(*(cb+1), &index, &base, &scale);

	switch (mod) {
		case MOD_INDIRECT_ADDRESS:
			if (sib) {
				if (base == DISP_ONLY) {
					printf("[%04x+%s*%d]", *(cb+offset+1), registers[index].names[1+size], scale);
				} else {
					printf("[%s+%s*%d]", registers[base].names[1+size], registers[index].names[1+size], scale);
				}
			} else {
				if (rm == DISP_ONLY) {
					printf("%04x", *(cb+offset+1));
				} else {
					printf("[%s]", registers[rm].names[1+size]);
				}
			}
			break;
		case MOD_ONE_BYTE_DISPLACEMENT:
			if (sib) {
				printf("[%01x+%s+%s*%d]", *(cb+offset+1), registers[base].names[0], registers[index].names[1+size], scale);
			} else {
				printf("dword [%s", registers[rm].names[1+size]);
				print_hex(*(cb+offset+1));
				printf("]");
			}
			offset++;
			break;
		case MOD_FOUR_BYTE_DISPLACEMENT:
			if (sib) {
				printf("[disp+%s+%s*d]", registers[base].names[1+size], registers[index].names[1+size], scale);
			} else {
				printf("dword [%s+disp32]", registers[rm].names[1+size]);
			}
			offset += 4;
			break;
		case MOD_REG_ADDRESS:
			printf("%s", registers[rm].names[1+size]);
			break;
	}
	return offset+1;
}

void decode_operands(unsigned char * cb, int dir, int size, int immediate)
{
	unsigned char reg = MASK_MODRM_REG(*cb);
	if(immediate) {
		int o = 1;
		if (dir) o = decode_rm(cb, size);
		else printf("%s", registers[reg].names[1+size]);
		printf(", ");
		print_hex(*(cb+o));
	} else {
		if (dir) {
			//RM->REG so REG, RM
			printf("%s", registers[reg].names[1+size]);
			printf(", ");
			decode_rm(cb, size);
		} else {
			//REG->RM so RM, REG
			decode_rm(cb, size);
			printf(", ");
			printf("%s", registers[reg].names[1+size]);
		}
	}
	printf("\n");

}

opcode find_opcode(unsigned char v)
{
	for (int i = 0; i < sizeof(opcodes)/sizeof(opcode); i++) {
		if (!(opcodes[i].v^v)) { 
			return opcodes[i];
		}
	}
		opcode op  = {0xFF, 0, 0, NON, NON, NON, "non"};
		return op;
}
void decode_instruction(unsigned char * cb, int maxsize)
{
	int idx = 0;
	unsigned char cmd;
	if (is_prefix(cb[idx])) {
		idx++;
	}
	if (is_address_size(cb[idx])) {
		idx++;
	}
	int f32 = is_operand_size(cb[idx]);
	if (f32) idx++;
	int seg = is_seg_override(cb[idx]);
	if (seg) {
		idx++;
	}
	cmd = cb[idx];
	if (cmd == 0x0f) {
		idx++;
		cmd = cb[idx];
	}
	idx++;
	int dir, size, imm;
	dir = ((cb[idx])&0x02);
	size = ((cb[idx]&0x1);
	imm = ((*cb)&0x80);
	opcode op = find_opcode(cmd);
	printf("%s ", op.name);
	decode_operands(cb+idx, 0, 1, 0);
}
void string_to_hex(char * str, unsigned char * out)
{
	int s = strlen(str);
	if (s % 2 == 0) {
		unsigned int h, l, c=0;
		for (int i=0;i<s;i+=2) {
			h = str[i] > '9' ? str[i] - 'A' + 10 : str[i] - '0';
			l = str[i+1] > '9' ? str[i+1] - 'A' + 10 : str[i+1] - '0';
			out[c] = (h << 4) | l; 
			c++;
		}	
	} else {
		printf("ERROR INVALID STRING\n");
		exit(1);
	}
}
int main(int argc, char ** argv)
{
	if (argc < 2) {
		printf("format: %s bytes\n", argv[0]);
		return 1;
	}
	unsigned char buffer[256];
	int size = strlen(argv[1]);
	if (size > 255) {
		printf("Input too long\n");
	}
	memset(buffer, 0x00, 255);
	string_to_hex(argv[1], buffer);
	//decode_operands(buffer + 1, 0, 1, 0);
	decode_instruction(buffer, size);
	return 0;

	unsigned char cb[] = { 0x83, 0xC0, 0x01 };
	int dir = ((*cb)&0x02) != 0;
	int imm = ((*cb)&0x80) != 0;
	printf("%d and %d\n", dir, imm);
	decode_operands(cb+1, ((*cb)&0x02) != 0, 1, ((*cb)&0x80) != 0);
	//decode_operands(cb+1, 0, 1, 1);
	return 0;
}
