// Concurrent version of prime sieve of Eratosthenes.
// Invented by Doug McIlroy, inventor of Unix pipes.
// See http://swtch.com/~rsc/thread/.
// The picture halfway down the page and the text surrounding it
// explain what's going on here.
//
// Since NENVS is 1024, we can print 1022 primes before running out.
// The remaining two environments are the integer generator at the bottom
// of main and user/idle.

#include <inc/lib.h>
int data_ready_id1 = 0;
int data_ready_id2 = 0;
#define DATA_READY_IPC -1

int32_t abs(int16_t n) { return n >= 0 ? n : -n; }

void reduce_unpacked_ratio(int16_t *a, int16_t *b)
{
	if (*a == 0)
	{
		*b = 1;
		return;
	}
	
	int16_t i;
	for (i = 2; i <= abs(*a) && i <= abs(*b); ++i)
		while ((*a % i == 0) && (*b % i == 0))
		{
			*a /= i;
			*b /= i;
		}

	if (*b < 0)
	{
		*a *= -1;
		*b *= -1;
	}
}

uint32_t pack_ratio(int16_t a, int16_t b)
{
	reduce_unpacked_ratio(&a, &b);
	return (((a & 0xFFFF) << 16) + (b & 0xFFFF));
}

void unpack_ratio(uint32_t num, int16_t *a, int16_t *b)
{
	*a = (num >> 16) & 0xFFFF;
	*b = num & 0xFFFF;
}

uint32_t add_ratio(uint32_t n1, uint32_t n2)
{
	int16_t a1, a2, b1, b2;
	unpack_ratio(n1, &a1, &b1);
	unpack_ratio(n2, &a2, &b2);
	return pack_ratio((a1 * b2) + (a2 * b1), b1 * b2);
}

uint32_t mul_ratio(uint32_t n1, uint32_t n2)
{
	int16_t a1, a2, b1, b2;
	unpack_ratio(n1, &a1, &b1);
	unpack_ratio(n2, &a2, &b2);
	return pack_ratio(a1 * a2, b1 * b2);
}

uint32_t inv_ratio(uint32_t num)
{
	int16_t a, b;
	unpack_ratio(num, &a, &b);
	return pack_ratio(b, a);	
}

uint32_t recv(envid_t id) //for sync
{
	ipc_send(id, DATA_READY_IPC, 0, 0);
	uint32_t data;
	while ((data = (uint32_t)ipc_recv(&id, 0, 0)) == DATA_READY_IPC)
	{
		if (!data_ready_id1) data_ready_id1 = id;
		else if (!data_ready_id2) data_ready_id2 = id;
		else panic("recv");
	}
	return data;
}

void send(uint32_t data)
{
	envid_t id = data_ready_id1;
	if (!id) ipc_recv(&id, 0, 0);

	data_ready_id1 = data_ready_id2 = 0;
	ipc_send(id, (int)data, 0, 0);
}

void print_proc(int id)
{
	int16_t a, b;
	for (;;)
	{
		unpack_ratio(recv(id), &a, &b);
		cprintf("%d/%d\n", a, b);
	}
}

void sum_proc(int id1, int id2)
{
	for (;;)
		send(add_ratio(recv(id1), recv(id2)));
}

void sum3_proc(int id1, int id2, int id3)
{
	for (;;)
		send(add_ratio(recv(id1), add_ratio(recv(id2), recv(id3))));
}

void delay_proc(int id)
{
	send(pack_ratio(0, 1));
	for (;;)
		send(recv(id));
}

void dup_proc(int src)
{
	for (;;)
	{
		uint32_t data = recv(src);
		while (!data_ready_id1 || !data_ready_id2)
		{
			envid_t id;
			ipc_recv(&id, 0, 0);
			if (!data_ready_id1) data_ready_id1 = id;
			else if (!data_ready_id2) data_ready_id2 = id;
			else panic("dup_proc");
		}

		ipc_send(data_ready_id1, data, 0, 0);
		ipc_send(data_ready_id2, data, 0, 0);

		data_ready_id1 = data_ready_id2 = 0;
	}

}

void scalar_proc(int id, uint32_t scalar)
{
	for (;;) send(mul_ratio(recv(id), scalar));
}

void mul_proc(int id1, int id2)
{
	uint32_t f = recv(id1);
	uint32_t g = recv(id2);

	send(mul_ratio(f, g));

	int dup_f_id, dup_g_id, scal_f_id, scal_g_id, delay_id, recursion_id, sum3_id;

	if ((dup_f_id = fork()) < 0)
		panic("fork: %e", dup_f_id);
	if (dup_f_id == 0)
		dup_proc(id1);

	if ((dup_g_id = fork()) < 0)
		panic("fork: %e", dup_g_id);
	if (dup_g_id == 0)
		dup_proc(id2);

	if ((scal_f_id = fork()) < 0)
		panic("fork: %e", scal_f_id);
	if (scal_f_id == 0)
		scalar_proc(dup_g_id, f);

	if ((scal_g_id = fork()) < 0)
		panic("fork: %e", scal_g_id);
	if (scal_g_id == 0)
		scalar_proc(dup_f_id, g);

	if ((recursion_id = fork()) < 0)
		panic("fork: %e", recursion_id);
	if (recursion_id == 0)
		mul_proc(dup_f_id, dup_g_id);

	if ((delay_id = fork()) < 0)
		panic("fork: %e", delay_id);
	if (delay_id == 0)
		delay_proc(recursion_id);

	if ((sum3_id = fork()) < 0)
		panic("fork: %e", sum3_id);
	if (sum3_id == 0)
		sum3_proc(scal_f_id, scal_g_id, delay_id);

	for (;;) send(recv(sum3_id));
}

void sub_proc(int id1, int id2)
{
	uint32_t f = recv(id1);
	uint32_t g = recv(id2);
	send(f);

	int dup_g_id, delay_g_id, recursion_id, mul_id;

	if ((dup_g_id = fork()) < 0)
		panic("fork: %e", dup_g_id);
	if (dup_g_id == 0)
		dup_proc(id2);

	if ((delay_g_id = fork()) < 0)
		panic("fork: %e", delay_g_id);
	if (delay_g_id == 0)
		delay_proc(dup_g_id);

	if ((recursion_id = fork()) < 0)
		panic("fork: %e", recursion_id);
	if (recursion_id == 0)
		sub_proc(id1, delay_g_id);

	if ((mul_id = fork()) < 0)
		panic("fork: %e", mul_id);
	if (mul_id == 0)
		mul_proc(dup_g_id, recursion_id);

	for (;;) send(recv(mul_id));
}

void gen_x_proc(void)
{
	send(pack_ratio(0, 1));
	send(pack_ratio(1, 1));
	for (;;)
		send(pack_ratio(0, 1));
}
void gen_x3_proc(void)
{
	send(pack_ratio(0, 1));
	send(pack_ratio(0, 1));
	send(pack_ratio(0, 1));
	send(pack_ratio(1, 1));
	for (;;)
		send(pack_ratio(0, 1));

}

void gen_sin_proc(void)
{
	int sign = 1;
	uint32_t power = 1, i;
	for (i = 1; ; i += 2)
	{
		send(pack_ratio(0, 1));
		send(pack_ratio(sign, power));

		sign *= -1;
		power *= ((i+1) * (i+2));
	}
}

void
umain(int argc, char **argv)
{
	int gen_x_id, gen_x2_id, gen_x3_id, gen_sin_id, sum_id, sub_id, print_id;

	if ((gen_x_id = fork()) < 0)
		panic("fork: %e", gen_x_id);
	if (gen_x_id == 0)
		gen_x_proc();

	if ((gen_x2_id = fork()) < 0)
		panic("fork: %e", gen_x2_id);
	if (gen_x2_id == 0)
		gen_sin_proc();

	if ((gen_x3_id = fork()) < 0)
		panic("fork: %e", gen_x3_id);
	if (gen_x3_id == 0)
		gen_x3_proc();

	if ((gen_sin_id = fork()) < 0)
		panic("fork: %e", gen_sin_id);
	if (gen_sin_id == 0)
		gen_sin_proc();

	if ((sum_id = fork()) < 0)
		panic("fork: %e", sum_id);
	if (sum_id == 0)
		sum_proc(gen_x_id, gen_x3_id);

	if ((sub_id = fork()) < 0)
		panic("fork: %e", sub_id);
	if (sub_id == 0)
		sub_proc(gen_sin_id, sum_id);

	if ((print_id = fork()) < 0)
		panic("fork: %e", print_id);
	if (print_id == 0)
		print_proc(sub_id);

	for (;;); //never ends
}

