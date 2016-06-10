#include <inc/lib.h>

int send_packet(struct jif_pkt *pkt)
{
	int r;
	while ((r=sys_try_send_packet(pkt->jp_data, pkt->jp_len)) == -E_NO_MEM)
		sys_yield();

	return r;
}

int recv_packet(char *buffer, int len){
	int r;
	while ((r=sys_recv_packet(buffer, len)) == -E_NO_RCV)
		sys_yield();

	return r;
}

