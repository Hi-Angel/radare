/* radare - LGPL - Copyright 2009 pancake<nopcode.org> */

#include "r_core.h"

#if 0
int r_core_shift()
{
	/* like rsc move does .. but optimal :) */
}
#endif

int r_core_write_op(struct r_core_t *core, const char *arg, char op) 
{
	char *str;
	u8 *buf;
	int i,j;
	int ret;
	int len;

	// XXX we can work with config.block instead of dupping it
	buf = (char *)malloc(core->blocksize);
	str = (char *)malloc(strlen(arg));
	if (buf == NULL || str == NULL) {
		free(buf);
		free(str);
		return 0;
	}
	memcpy(buf, core->block, core->blocksize);
	len = r_hex_str2bin(arg, (u8 *)str);

	switch(op) {
		case '2':
		case '4':
			op-='0';
			for(i=0;i<core->blocksize;i+=op) {
				/* endian swap */
				u8 tmp = buf[i];
				buf[i]=buf[i+3];
				buf[i+3]=tmp;
				if (op==4) {
					tmp = buf[i+1];
					buf[i+1]=buf[i+2];
					buf[i+2]=tmp;
				}
			}
			break;
		default:
			for(i=j=0;i<core->blocksize;i++) {
				switch(op) {
					case 'x': buf[i] ^= str[j]; break;
					case 'a': buf[i] += str[j]; break;
					case 's': buf[i] -= str[j]; break;
					case 'm': buf[i] *= str[j]; break;
					case 'd': buf[i] /= str[j]; break;
					case 'r': buf[i] >>= str[j]; break;
					case 'l': buf[i] <<= str[j]; break;
					case 'o': buf[i] |= str[j]; break;
					case 'A': buf[i] &= str[j]; break;
				}
				j++; if (j>=len) j=0; /* cyclic key */
			}
	}

	ret = r_core_write_at(core, core->seek, buf, core->blocksize);

	free(buf);
	return ret;
}

int r_core_write_at(struct r_core_t *core, u64 addr, const u8 *buf, int size)
{
	int ret;
	r_io_lseek(&core->io, core->file->fd, addr, R_IO_SEEK_SET);
	ret = r_io_write(&core->io, core->file->fd, buf, size);
	if (addr >= core->seek && addr <= core->seek+core->blocksize)
		r_core_block_read(core, 0);
	return (ret==-1)?R_FALSE:R_TRUE;
}

int r_core_read_at(struct r_core_t *core, u64 addr, const u8 *buf, int size)
{
	int ret;
	r_io_lseek(&core->io, core->file->fd, addr, R_IO_SEEK_SET);
	ret = r_io_read(&core->io, core->file->fd, buf, size);
	if (addr >= core->seek && addr <= core->seek+core->blocksize)
		r_core_block_read(core, 0);
	return (ret==-1)?R_FALSE:R_TRUE;
}
