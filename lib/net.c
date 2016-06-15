#include <inc/lib.h>

int send_packet(struct jif_pkt *pkt)
{
	int r;
	while ((r=sys_try_send_packet(pkt->jp_data, pkt->jp_len)) == -E_NOT_READY)
		sys_yield();

	return r;
}

int recv_packet(struct jif_pkt *pkt)
{
	int r;
	while ((r=sys_try_recv_packet(pkt->jp_data, pkt->jp_len, (size_t*)&pkt->jp_len)) == -E_NOT_READY)
		sys_yield();

	return r;
}

