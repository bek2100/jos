#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

#include <kern/pci.h>

#include <inc/error.h>

#define E1000_VENDOR_ID		0x8086
#define E1000_DEVICE_ID_DESKTOP	0x100E
#define E1000_DEVICE_ID_MOBILE	0x1015

struct tx_desc_t
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

int e1000_attach(struct pci_func *pcif);
int e1000_try_send_packet(const char *buffer, size_t len);

#endif	// JOS_KERN_E1000_H
