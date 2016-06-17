#include <inc/lib.h>

int send_packet(struct jif_pkt *pkt, unsigned int time_out_msec)
{
	int r;
	unsigned int timeout = time_out_msec == INFINIT ? (unsigned int)-1 : sys_time_msec() + time_out_msec;

	while ((r=sys_try_send_packet(pkt->jp_data, pkt->jp_len)) == -E_NOT_READY)
	{
		sys_yield();
		if (sys_time_msec() > timeout) return -E_TIMEOUT;
	}

	return r;
}

int recv_packet(struct jif_pkt *pkt, unsigned int time_out_msec)
{
	int r;
	unsigned int timeout = time_out_msec == INFINIT ? (unsigned int)-1 : sys_time_msec() + time_out_msec;

	size_t len;
	while ((r=sys_try_recv_packet(pkt, &len)) == -E_NOT_READY)
	{
		sys_yield();
		if (sys_time_msec() > timeout) return -E_TIMEOUT;
	}

	pkt->jp_len = len;
	return r;
}

