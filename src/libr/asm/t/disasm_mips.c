/* radare - LGPL - Copyright 2009 nibble<.ds@gmail.com> */

#include <stdio.h>
#include <stdlib.h>

#include <r_types.h>
#include <r_asm.h>

int main()
{
	struct r_asm_t a;
	u8 *buf = "\x04\x28\x9c\x27\x21\xe0\x9f\x03";
	int ret = 0;
	u64 idx = 0, len = 8;

	r_asm_init(&a);
	r_asm_set_arch(&a, R_ASM_ARCH_MIPS);
	r_asm_set_bits(&a, 32);
	r_asm_set_big_endian(&a, R_FALSE);
	r_asm_set_syntax(&a, R_ASM_SYN_INTEL);

	while (idx < len) {
		r_asm_set_pc(&a, 0x000089d8 + idx);

		ret = r_asm_disasm(&a, buf+idx, len-idx);
		printf("DISASM %s HEX %s\n", a.buf_asm, a.buf_hex);

		idx += ret;
	}

	return 0;
}
