#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

#include <kern/pci.h>

#define VENDOR_ID_E1000 0x8086h
#define DEVICE_ID_E1000_DESKTOP 0x100E
#define DEVICE_ID_E1000_MOBILE 0x1015

int e1000_attach(struct pci_func *pcif);

struct tx_desc
{
        uint64_t addr;
        uint16_t length;
        uint8_t cso;
        uint8_t cmd;
        uint8_t status;
        uint8_t css;
        uint16_t special;
} __attribute__((packed));

#define TX_BUFFER_MAX 1518
typedef char tx_buffer_t[TX_BUFFER_MAX];

#endif	// JOS_KERN_E1000_H
