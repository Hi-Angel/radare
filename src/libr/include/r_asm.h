/* radare - LGPL - Copyright 2009 nibble<.ds@gmail.com> */

#ifndef _INCLUDE_R_ASM_H_
#define _INCLUDE_R_ASM_H_

#include "r_types.h"

enum {
	R_ASM_ARCH_NULL  = 0,
	R_ASM_ARCH_X86   = 1,
	R_ASM_ARCH_ARM   = 2,
	R_ASM_ARCH_PPC   = 3,
	R_ASM_ARCH_M68K  = 4,
	R_ASM_ARCH_JAVA  = 5,
	R_ASM_ARCH_MIPS  = 6,
	R_ASM_ARCH_SPARC = 7,
	R_ASM_ARCH_CSR   = 8,
	R_ASM_ARCH_MSIL  = 9,
	R_ASM_ARCH_OBJD  = 10,
	R_ASM_ARCH_BF    = 11
};

enum {
	R_ASM_SYN_NULL  = 0,
	R_ASM_SYN_INTEL = 1,
	R_ASM_SYN_ATT   = 2,
	R_ASM_SYN_OLLY  = 3
};

enum {
	R_ASM_PAR_NULL    = 0,
	R_ASM_PAR_PSEUDO  = 1,
	R_ASM_PAR_REALLOC = 2
};

struct r_asm_t {
	int  arch;
	int  bits;
	int  big_endian;
	int  syntax;
	int  parser;
	u64  pc;
	char buf_asm[256];
	char buf_hex[256];
	char buf_err[256];
	void *aux;
	int  (*r_asm_disasm)(struct r_asm_t *a, u8 *buf, u64 len);
	int  (*r_asm_asm)(struct r_asm_t *a, char *buf);
	int  (*r_asm_parse)(struct r_asm_t *a);
	int  (*r_asm_parse_cb)(struct r_asm_t *a);
};

/* asm.c */
int r_asm_init(struct r_asm_t *a);
struct r_asm_t *r_asm_new();
void r_asm_free(struct r_asm_t *a);
int r_asm_set_arch(struct r_asm_t *a, int arch);
int r_asm_set_bits(struct r_asm_t *a, int bits);
int r_asm_set_big_endian(struct r_asm_t *a, int boolean);
int r_asm_set_syntax(struct r_asm_t *a, int syntax);
int r_asm_set_parser(struct r_asm_t *a, int parser, 
		int (*cb)(struct r_asm_t *a), void *aux);
int r_asm_set_pc(struct r_asm_t *a, u64 pc);
int r_asm_disasm(struct r_asm_t *a, u8 *buf, u64 len);
int r_asm_asm(struct r_asm_t *a, char *buf);
int r_asm_parse(struct r_asm_t *a);

/* arch/x86/asm.c */
int r_asm_x86_disasm(struct r_asm_t *a, u8 *buf, u64 len);
int r_asm_x86_asm(struct r_asm_t *a, char *buf);
/* arch/x86/pseudo.c */
int r_asm_x86_pseudo(struct r_asm_t *a);
/* arch/x86/realloc.c */
struct r_asm_realloc_t {
	u64 offset;
	u64 delta;
	char str[256];
};
int r_asm_x86_realloc(struct r_asm_t *a);

/* arch/arm/asm.c */
int r_asm_arm_disasm(struct r_asm_t *a, u8 *buf, u64 len);

/* arch/mips/asm.c */
int r_asm_mips_disasm(struct r_asm_t *a, u8 *buf, u64 len);

/* arch/sparc/asm.c */
int r_asm_sparc_disasm(struct r_asm_t *a, u8 *buf, u64 len);

/* arch/ppc/asm.c */
int r_asm_ppc_disasm(struct r_asm_t *a, u8 *buf, u64 len);

/* arch/bf/asm.c */
int r_asm_bf_disasm(struct r_asm_t *a, u8 *buf, u64 len);
#endif
