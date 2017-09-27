#define EIE 0x1b
#define EIR 0x1c
#define ESTAT 0x1d
#define ECON2 0x1e
#define ECON1 0x1f

// Bank 0 registers
#define ERDPTL 0x00
#define ERDPTH 0x01
#define EWRPTL 0x02
#define EWRPTH 0x03
#define ETXSTL 0x04
#define ETXSTH 0x05
#define ETXNDL 0x06
#define ETXNDH 0x07
#define ERXSTL 0x08
#define ERXSTH 0x09
#define ERXNDL 0x0a
#define ERXNDH 0x0b
#define ERXRDPTL 0x0c
#define ERXRDPTH 0x0d

// Bank 1 registers
#define EPMM0 0x28
#define EPMM1 0x29
#define EPMCSL 0x30
#define EPMCSH 0x31
#define ERXFCON 0x38
#define EPKTCNT 0x39

// Bank 2 registers
#define MACON1 0xc0
#define MACON2 0xc1
#define MACON3 0xc2
#define MABBIPG 0xc4
#define MAIPGL 0xc6
#define MAIPGH 0xc7
#define MAMXFLL 0xca
#define MAMXFLH 0xcb
#define MIREGADR 0xd4
#define MIWRL 0xd6
#define MIWRH 0xd7

// Bank 3 registers
#define MAADR1 0xe0
#define MAADR0 0xe1
#define MAADR3 0xe2
#define MAADR2 0xe3
#define MAADR5 0xe4
#define MAADR4 0xe5
#define MISTAT 0xea

// PHY registers
#define PHCON1 0x00
#define PHCON2 0x10
#define PHLCON 0x14

#define ERXFCON_UCEN 0x80
#define ERXFCON_CRCEN 0x20
#define ERXFCON_PMEN 0x10
#define EIE_INTIE 0x80
#define EIE_PKTIE 0x40
#define EIE_RXERIE 0x01
#define EIR_TXERIF 0x02
#define ESTAT_CLKRDY 0x01
#define ECON2_PKTDEC 0x40
#define ECON1_TXRTS 0x08
#define ECON1_RXEN 0x04
#define ECON1_BSEL1 0x02
#define ECON1_BSEL0 0x01
#define MACON1_TXPAUS 0x08
#define MACON1_RXPAUS 0x04
#define MACON1_MARXEN 0x01
#define MACON3_PADCFG0 0x20
#define MACON3_TXCRCEN 0x10
#define MACON3_FRMLNEN 0x02
#define MACON3_FULDPX 0x01
#define MISTAT_BUSY 0x01
#define PHCON1_PDPXMD 0x0100
#define PHCON2_HDLDIS 0x0100

/* 指令集, 详见数据手册26页 */
#define ENC28J60_READ_CTRL_REG 0x00 // 读控制寄存器
#define ENC28J60_READ_BUF_MEM 0x3a // 读缓冲区
#define ENC28J60_WRITE_CTRL_REG 0x40 // 写控制寄存器
#define ENC28J60_WRITE_BUF_MEM 0x7a // 写缓冲区
#define ENC28J60_BIT_FIELD_SET 0x80 // 位域置位
#define ENC28J60_BIT_FIELD_CLR 0xa0 // 位域清零
#define ENC28J60_SOFT_RESET 0xff // 系统复位

#define RXSTART_INIT 0 // 接收缓冲区起始地址
#define RXSTOP_INIT (0x1fff - 0x0600 - 1) // 接收缓冲区停止地址
#define TXSTART_INIT (0x1fff - 0x0600) // 发送缓冲区起始地址, 大小约1500字节
#define TXSTOP_INIT 0x1fff // 发送缓冲区停止地址
#define MAX_FRAMELEN 1500 // 以太网报文最大长度

void ENC28J60_BeginSend(void);
void ENC28J60_EndReceive(void);
uint16_t ENC28J60_GetPacketLength(void);
uint8_t ENC28J60_GetPacketNum(void);
void ENC28J60_Init(uint8_t *mac_addr);
void ENC28J60_InitSend(uint16_t len);
uint8_t ENC28J60_Read(uint8_t addr);
void ENC28J60_ReadBuf(uint8_t *p, uint16_t len);
uint8_t ENC28J60_ReadOP(uint8_t op, uint8_t addr);
void ENC28J60_SetBank(uint8_t addr);
void ENC28J60_Write(uint8_t addr, uint8_t data);
void ENC28J60_WriteBuf(uint8_t *p, uint16_t len);
void ENC28J60_WriteOP(uint8_t op, uint8_t addr, uint8_t data);
void ENC28J60_WritePhy(uint8_t addr, uint16_t data);
