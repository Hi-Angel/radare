#include "r_print.h"

int main()
{
	u8 *buf = (u8*)main;

	r_cons_init();
	r_print_hexdump(main, buf, 128, 1, 78, 1);
	r_cons_flush();

	return 0;
}
