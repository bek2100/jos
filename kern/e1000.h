#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

#include <kern/pci.h>

#include <inc/error.h>

#define E1000_VENDOR_ID		0x8086
#define E1000_DEVICE_ID_DESKTOP	0x100E
#define E1000_DEVICE_ID_MOBILE	0x1015

#define TX_COUNT 64
#define RX_COUNT 256

#define DD_BIT   (1<<0)
#define TCP_BIT  (1<<0)
#define RS_BIT   (1<<3)
#define EOP_BIT  (1<<1)

#define TDBAL 0xe00
#define TDBAH 0xe01
#define TDLEN 0xe02
#define TDH   0xe04
#define TDT   0xe06

#define TCTL  0x100
#define TIPG  0x104

#define RAL   0x1500
#define RAH   0x1501

#define RDBAL 0xa00
#define RDBAH 0xa01
#define RDLEN 0xa02
#define RDH   0xa04
#define RDT   0xa06

#define RCTL  0x40
#define IMS   0x34

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

#define TX_BUFFER_MAX 2048
typedef char tx_buffer_t[TX_BUFFER_MAX];

struct rx_desc_t
{
        uint64_t addr;
        uint16_t length;
        uint16_t checksup;
        uint8_t status;
        uint8_t errors;
        uint16_t special;
} __attribute__((packed));

#define RX_BUFFER_MAX 2048
typedef char rx_buffer_t[RX_BUFFER_MAX];

int e1000_attach(struct pci_func *pcif);
int e1000_try_send_packet(const char *buffer, size_t len);
int e1000_recv_packet(char *buffer);

#endif	// JOS_KERN_E1000_H
