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
		if (ipc_recv(&e, &nsipcbuf, &perm) == NSREQ_OUTPUT && e == ns_envid)
		{
			if (send_packet(&nsipcbuf.pkt) < 0) panic("send");
		}
	}
}
