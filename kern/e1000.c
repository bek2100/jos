#include <kern/e1000.h>
#include <inc/stdio.h>

#include <kern/pmap.h>
#include <inc/string.h>

volatile uint32_t *bar0 = NULL;

struct tx_desc_t tx_desc[TX_COUNT] = {{0}};
struct rx_desc_t rx_desc[RX_COUNT] = {{0}};

tx_buffer_t tx_buffers[TX_COUNT] = {{0}};
rx_buffer_t rx_buffers[RX_COUNT] = {{0}};

uint32_t tdt = 0;
uint32_t rdt = 0;

int e1000_attach(struct pci_func *pcif)
{
	pci_func_enable(pcif);
	bar0 = mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);

	uint32_t i;
	for (i = 0; i < TX_COUNT; ++i)
	{
		memset(&tx_buffers[i], 0 , TX_BUFFER_MAX);
		tx_desc[i].status |= DD_BIT;
	}

	for (i = 0; i < RX_COUNT; ++i)
	{
		memset(&rx_buffers[i], 0 , RX_BUFFER_MAX);
		rx_desc[i].addr = PADDR(&rx_buffers[i]);
		//rx_desc[i].length = RX_BUFFER_MAX;
	}

	bar0[TDBAL] = PADDR(tx_desc);
	bar0[TDBAH] = 0;
	bar0[TDLEN] = sizeof(tx_desc);

	bar0[TDH] = bar0[TDT] = 0;

	bar0[TCTL] = 0x4010A;
	bar0[TIPG] = 0x60200A;

	//uint8_t mac[6] = {0x52, 0x54, 0x00, 0x12, 0x34, 0x56};
	//for (i = 0x1480; i < 0x1500; ++i) bar0[i] = 0;
	bar0[RAL] = 0x12005452;
	bar0[RAH] = 0x80005634;

	bar0[RDBAL] = PADDR(rx_desc);
	bar0[RDBAH] = 0;
	bar0[RDLEN] = sizeof(rx_desc);

	cprintf("RDLEN %d\n",bar0[RDLEN]);


	bar0[RDH] = 0;
	bar0[RDT] = RX_COUNT;
	//bar0[RDT] = 0;

	bar0[RCTL] = 0x4000002;
	bar0[IMS] = 0;

	return 0;
}

int e1000_try_send_packet(const char *buffer, size_t len)
{
		cprintf("eiv hamood-1\n");

	if (len > TX_BUFFER_MAX) return -E_INVAL;

	if (!(tx_desc[tdt].status & DD_BIT)) return -E_NO_MEM;

	memcpy(&tx_buffers[tdt], buffer, len);
	memset(&tx_desc[tdt], 0, sizeof(tx_desc[tdt]));

	tx_desc[tdt].addr = PADDR(&tx_buffers[tdt]);
	tx_desc[tdt].length = len;
	tx_desc[tdt].cmd |= RS_BIT | TCP_BIT;

	++tdt;
	tdt = tdt % TX_COUNT;
	bar0[TDT] = tdt;
	cprintf("eiv hamood1\n");
	return 0;
}

int e1000_recv_packet(char *buffer){

	if(!(rx_desc[rdt].status & DD_BIT)) return -E_NO_RCV;

	if(!(rx_desc[rdt].status & EOP_BIT)) panic("no long packet implemnted");

	memcpy(buffer, &rx_buffers[rdt], rx_desc[rdt].length);
	rx_desc[rdt].status = 0; 

	bar0[RDT] = rdt;
	cprintf("hello %d, %s\n",rx_desc[rdt].length, buffer);

	++rdt;
	rdt = rdt % RX_COUNT;

	return rx_desc[rdt].length;
}



