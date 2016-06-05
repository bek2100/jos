#include <kern/e1000.h>

// LAB 6: Your driver code here

volatile uint32_t *bar0;
#define TX_COUNT 16

int e1000_attach(struct pci_func *pcif){
	pci_func_enable(pcif);
	bar0 = mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);

	uint32_t i;
	for (i = 0; i < TX_COUNT; ++i)
	{
		tx_desc[i].addr = PADDR(&tx_buffers[i]);
		tx_desc[i].length = TX_BUFFER_MAX;
	}
	bar0[0xe00] = PADDR(tx_desc);
	bar0[0xe01] = 0;
	bar0[0xe02] = sizeof(tx_desc);

	bar0[0xe04] = bar0[0xe06] = 0;

	bar0[0x100] = 0x4010A;
	bar0[0x104] = 0x60200A;
}
