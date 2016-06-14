#include <inc/lib.h>

int send_packet(struct jif_pkt *pkt)
{
	int r;
	while ((r=sys_try_send_packet(pkt->jp_data, pkt->jp_len)) == -E_NO_MEM)
		sys_yield();

	return r;
}


