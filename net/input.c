#include "ns.h"


#define RECV_SIZE 2048

extern union Nsipc nsipcbuf;

void
input(envid_t ns_envid)
{
	binaryname = "ns_input";

	// LAB 6: Your code here:
	// 	- read a packet from the device driver
	//	- send it to the network server
	// Hint: When you IPC a page to the network server, it will be
	// reading from it for a while, so don't immediately receive
	// another packet in to the same physical page.

	for(;;){
			char buffer[RECV_SIZE];
			int perm = PTE_U | PTE_P | PTE_W;
			int len;
			int r;
			while((r=sys_page_alloc(0, &nsipcbuf, perm))<0);
			while ((nsipcbuf.pkt.jp_len = sys_recv_packet(nsipcbuf.pkt.jp_data)) < 0){
				sys_yield();
			}
				cprintf("eiv hamood3\n");
			while ((r =sys_ipc_try_send(ns_envid, NSREQ_INPUT, &nsipcbuf ,perm))<0);
				cprintf("eiv hamood0\n");
		}
}
