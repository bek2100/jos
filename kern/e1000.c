#include <kern/e1000.h>

// LAB 6: Your driver code here

volatile uint32_t *bar0;
#define TX_NUM 16

int e1000_attach(struct pci_func *pcif){
	pci_func_enable(&pcif);
	bar0 = mmio_map_region(pcif->register_base[0], pcif->register_size[0]);

	int i;
	for(i=0; i<TX_NUM; i++){
		
	}
}
