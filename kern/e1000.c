#include <kern/e1000.h>

#include <kern/pmap.h>
#include <inc/string.h>

#define TX_COUNT 64

#define DD_BIT   (1<<0)
#define TCP_BIT  (1<<0)
#define RS_BIT   (1<<3)

volatile uint32_t *bar0 = NULL;

struct tx_desc_t tx_desc[TX_COUNT] = {{0}};
tx_buffer_t tx_buffers[TX_COUNT] = {{0}};
uint32_t tdt = 0;

int e1000_attach(struct pci_func *pcif)
{
	pci_func_enable(pcif);
	bar0 = mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);

	uint32_t i;
	for (i = 0; i < TX_COUNT; ++i)
	{
		tx_desc[i].status |= DD_BIT;
		/*
		tx_desc[i].addr = PADDR(&tx_buffers[i]);
		tx_desc[i].length = TX_BUFFER_MAX;
		*/
	}

	bar0[0xe00] = PADDR(tx_desc);
	bar0[0xe01] = 0;
	bar0[0xe02] = sizeof(tx_desc);

	bar0[0xe04] = bar0[0xe06] = 0;

	bar0[0x100] = 0x4010A;
	bar0[0x104] = 0x60200A;
	return 0;
}

int e1000_try_send_packet(const char *buffer, size_t len)
{
	if (len > TX_BUFFER_MAX) return -E_INVAL;

	if (!(tx_desc[tdt].status & DD_BIT)) return -E_NO_MEM;

	memcpy(&tx_buffers[tdt], buffer, len);
	memset(&tx_desc[tdt], 0, sizeof(tx_desc[tdt]));

	tx_desc[tdt].addr = PADDR(&tx_buffers[tdt]);
	tx_desc[tdt].length = len;
	tx_desc[tdt].cmd |= RS_BIT | TCP_BIT;

	++tdt;
	tdt = tdt % TX_COUNT;
	bar0[0xe06] = tdt;

	return true;
}
