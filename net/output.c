#include "ns.h"

extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver

	for (;;)
	{
		envid_t e;
		int perm;
		if (ipc_recv(&e, UTEMP, &perm) == NSREQ_OUTPUT)
		{
			struct jif_pkt *pkt = (struct jif_pkt*)UTEMP;
//			cprintf("buf %p %x\n", UTEMP, *(uint32_t*)UTEMP);
			while (sys_try_send_packet(pkt->jp_data, pkt->jp_len) == -E_NO_MEM) ;
		}
	}
}
