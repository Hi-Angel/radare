#include <r_diff.h>

int cb(struct r_diff_t *d, void *user, struct r_diff_op_t *op)
{
	int i;

	printf(" 0x%08llx  ", op->a_off);
	for(i = 0;i<op->a_len;i++)
		printf("%02x", op->a_buf[i]);
	printf(" => ");
	for(i = 0;i<op->b_len;i++)
		printf("%02x", op->b_buf[i]);
	printf("  0x%08llx\n", op->b_off);
	return 1;
}

int test_equal()
{
	struct r_diff_t d;
	char *bufa = "helloworld";
	char *bufb = "heprswarld";

	printf("Diffing '%s' vs '%s'\n", bufa, bufb);
	r_diff_init(&d, 0, 0);
	r_diff_set_delta(&d, 0);
	r_diff_set_callback(&d, &cb, NULL);
	r_diff_buffers(&d, bufa, strlen(bufa), bufb, strlen(bufb));
	return 1;
}

int test_diff()
{
	struct r_diff_t d;
	char *bufa = "hello";
	char *bufb = "hellpworld";

	printf("Truncated diffing '%s' vs '%s'\n", bufa, bufb);
	r_diff_init(&d, 0, 0);
	r_diff_set_delta(&d, 0);
	r_diff_set_callback(&d, &cb, NULL);
	r_diff_buffers(&d, bufa, strlen(bufa), bufb, strlen(bufb));
	return 1;
}

int test_delta()
{
	struct r_diff_t d;
	char *bufa = "hello";
	char *bufb = "heprpworld";

	printf("Delta diffing '%s' vs '%s'\n", bufa, bufb);
	r_diff_init(&d, 0, 0);
	r_diff_set_delta(&d, 1);
	r_diff_set_callback(&d, &cb, NULL);
	r_diff_buffers(&d, bufa, strlen(bufa), bufb, strlen(bufb));
	return 1;
}

int main()
{
	test_equal();
	printf("--\n");
	test_equal();
	printf("--\n");
	test_diff();
	printf("--\n");
	test_delta();


	return 0;
}
