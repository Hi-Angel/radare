/*
 * This code was done 
 *    by an anonymous gnome
 * ------------------------
 * That's pure mathematics, so no sense to adding license shit here.
 *
 */

#include <stdlib.h>
#include <math.h>
#include "r_types.h"

static double get_px(u8 x, const u8 *data, u64 size)
{
        u32 i, count;

        count = 0;
        for (i = 0; i < size; i++)
                if (data[i] == x)
                        count++;

        return (double)count/size;
}

double r_hash_entropy(const u8 *data, u64 size)
{
        double h = 0, px, log2;
        unsigned char x;

        log2 = logf((double)2);

        for (x = 0; x < 255; x++) {
                px = get_px(x, data, size);
                if (px > 0)
                        h += -px * (log(px)/log2);
        }

        return h;
}

#if 0
// TRASH CODE ???
int i;
int index1,index2;
int windowsize= 512; // block size
uchar ek[256];
uchar ck[256];
uchar entropy[256];

for (i = 0; i < 256; i++)
  ek[i] = i/256 * Log(2,i); // pre-calculate the entropy for each possible probability

for (i = 0; i < windowsize; i++)
  ck[buffer[i]]++; // count how many of each byte there is in the window

for (i = 0; i < 256; i++)
  entropy[0] -= ek[ck[buffer[i]]]; // calculate the entropy for the first window position

for (i = 1; i < buffer.length - wsize; i++)
{               // slide the window position and update the new entropy as it slides.
  index1 = i-1;
  index2 = i+windowsize-1;

  entropy[i] = entropy[i-1] + ek[ck[buffer[index1]]] + ek[ck[buffer[index2]]];
  index1 = ck[buffer[index1]]--;
  index2 = ck[buffer[index2]]++;
  entropy[i] -= ek[index1] + ek[index2];
}
#endif
