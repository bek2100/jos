#ifndef JOS_KERN_E1000_CONSTS_H
#define JOS_KERN_E1000_CONSTS_H

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

#endif //JOS_KERN_E1000_CONSTS_H
