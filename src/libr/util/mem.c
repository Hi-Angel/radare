/* radare - LGPL - Copyright 2007-2009 pancake<nopcode.org> */

#include <r_util.h>
#include <stdlib.h>

void r_mem_copyloop (u8 *dest, u8 *orig, int dsize, int osize)
{
        int i=0,j;
        while(i<dsize)
                for(j=0;j<osize && i<dsize;j++)
                        dest[i++] = orig[j];
}

/* TODO check and use system endian */
void r_mem_copyendian (u8 *dest, u8 *orig, int size, int endian)
{
        if (endian) {
                memcpy(dest, orig, size);
        } else {
                unsigned char buffer[8];
                switch(size) {
                case 2:
                        buffer[0] = orig[0];
                        dest[0]   = orig[1];
                        dest[1]   = buffer[0];
                        break;
                case 4:
                        memcpy(buffer, orig, 4);
                        dest[0] = buffer[3];
                        dest[1] = buffer[2];
                        dest[2] = buffer[1];
                        dest[3] = buffer[0];
                        break;
                case 8:
                        memcpy(buffer, orig, 8);
                        dest[0] = buffer[7];
                        dest[1] = buffer[6];
                        dest[2] = buffer[5];
                        dest[3] = buffer[4];
                        dest[4] = buffer[3];
                        dest[5] = buffer[2];
                        dest[6] = buffer[1];
                        dest[7] = buffer[0];
                        break;
                default:
                        printf("Invalid size: %d\n", size);
                }
        }
}
