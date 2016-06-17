#include <kern/e1000.h>
#include <kern/e1000_consts.h>
#include <kern/picirq.h>

#include <kern/pmap.h>
#include <kern/env.h>

#include <inc/string.h>

volatile uint32_t *bar0 = NULL;

struct tx_desc_t tx_desc[TX_COUNT] = {{0}};
struct rx_desc_t rx_desc[RX_COUNT] = {{0}};

uint32_t tdt = 0;
uint32_t rdt = 0;

uint32_t mac_low  = -1;
uint32_t mac_high = -1;

uint32_t e1000_get_mac_low()  {return mac_low;}
uint32_t e1000_get_mac_high() {return mac_high;}
uint8_t irq = 0;
uint8_t e1000_get_irq() {return irq;}

int e1000_attach(struct pci_func *pcif)
{
	pci_func_enable(pcif);
	bar0 = mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);
	irq = pcif->irq_line;
	irq_setmask_8259A(irq_mask_8259A & ~( 1 << pcif->irq_line));

	uint32_t i;
	for (i = 0; i < TX_COUNT; ++i)
	{
		tx_desc[i].status |= DD_BIT;
	}

	for (i = 0; i < RX_COUNT; ++i)
	{
	        struct PageInfo *pp = page_alloc(1);
	        if (!pp) return -E_NO_MEM;

		++pp->pp_ref;
		rx_desc[i].addr = page2pa(pp) + HEAD_SIZE;
	}

	bar0[EERD] = EERD_START | (0 << EERD_ADDR_START);
	while (!((i = bar0[EERD]) & EERD_DONE)) ;
	mac_low = (i >> EERD_DATA_START);

	bar0[EERD] = EERD_START | (1 << EERD_ADDR_START);
	while (!((i = bar0[EERD]) & EERD_DONE)) ;
	mac_low |= (i >> EERD_DATA_START) << 16;

	bar0[EERD] = EERD_START | (2 << EERD_ADDR_START);
	while (!((i = bar0[EERD]) & EERD_DONE)) ;
	mac_high = (i >> EERD_DATA_START);

	bar0[TDBAL] = PADDR(tx_desc);
	bar0[TDBAH] = 0;
	bar0[TDLEN] = sizeof(tx_desc);

	bar0[TDH] = bar0[TDT] = 0;

	bar0[TIPG] = 0x60200A;

	bar0[RAL] = mac_low;
	bar0[RAH] = mac_high | (1<<31);

	bar0[RDBAL] = PADDR(rx_desc);
	bar0[RDBAH] = 0;
	bar0[RDLEN] = sizeof(rx_desc);

	bar0[RDH] = 0;
	bar0[RDT] = RX_COUNT - 1;

	bar0[IMS] = 0;

	bar0[TCTL] = 0x4010A;
	bar0[RCTL] = 0x4000002;

	return 0;
}

int e1000_try_send_packet(const char *buffer, size_t len)
{
	if (len > TX_BUFFER_MAX) return -E_INVAL;

	if (!(tx_desc[tdt].status & DD_BIT)) return -E_NOT_READY;

	struct PageInfo *pp = page_lookup(curenv->env_pgdir, (char*)buffer, NULL);
	if (!pp) return -E_INVAL;

	memset(&tx_desc[tdt], 0, sizeof(tx_desc[tdt]));

	tx_desc[tdt].addr = page2pa(pp) + PGOFF(buffer);
	tx_desc[tdt].length = len;
	tx_desc[tdt].cmd |= RS_BIT | TCP_BIT;

	++tdt;
	tdt = tdt % TX_COUNT;
	bar0[TDT] = tdt;

	return 0;
}

int e1000_try_recv_packet(void *page, size_t *out_len)
{
	if (!out_len) return -E_INVAL;

	if (!(rx_desc[rdt].status & DD_BIT)) return -E_NOT_READY;

	struct PageInfo *pp = pa2page(rx_desc[rdt].addr);
	if (page_insert(curenv->env_pgdir, pp, page ,PTE_W|PTE_U|PTE_P) < 0) return -E_NO_MEM;
	page_decref(pp);

	*out_len = rx_desc[rdt].length;

	memset(&rx_desc[rdt], 0, sizeof(rx_desc[rdt]));

	pp = page_alloc(1);
	if (!pp) return -E_NO_MEM;
	rx_desc[rdt].addr = page2pa(pp) + HEAD_SIZE;
	++pp->pp_ref;

	bar0[RDT] = rdt;
	++rdt;
	rdt = rdt % RX_COUNT;

	return 0;
}

void e1000_intr()
{
	cprintf("e1000 INTR %d %d\n", bar0[IMS], bar0[ICR]);
	cprintf("2nd: e1000 INTR %d %d\n", bar0[IMS], bar0[ICR]);
}

