#include "networkcard.h"

void network_cs_set(struct network_t * network,uint8_t var){
	GPIO_WriteBit(network->cs.port,network->cs.pin,(BitAction)(var));
}

void network_init(struct network_t * network){
	GPIO_InitTypeDef  GPIO_InitStructure;
	SPI_InitTypeDef SPI_InitStructure;

	//open clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE );
	clock_set(network->cs);
	clock_set(network->sck);
	clock_set(network->miso);
	clock_set(network->mosi);

	/* SPI1 SCK@GPIOA.5 SPI1 MOSI@GPIOA.7 */
	GPIO_InitStructure.GPIO_Pin = network->sck.pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	/* 复用推挽输出 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(network->sck.port, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = network->mosi.pin;
	GPIO_Init(network->mosi.port, &GPIO_InitStructure);
	/* SPI1 MISO@GPIOA.6 */
	GPIO_InitStructure.GPIO_Pin = network->miso.pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	/* 浮动输入 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(network->miso.port, &GPIO_InitStructure);
	/* enc28j60 CS @GPIOA.4 */
	GPIO_InitStructure.GPIO_Pin = network->cs.pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(network->cs.port, &GPIO_InitStructure);
	/* 双线双向全双工 */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	/* 主机模式 */
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	/* 8位帧结构 */
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	/* 时钟空闲时为低 */
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	/* 第1个上升沿捕获数据 */
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	/* MSS 端口软件控制 */
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	/* SPI时钟 72Mhz / 8 = 9M */
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
	/* 数据传输高位在前 */
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;

	SPI_InitStructure.SPI_CRCPolynomial = 7;
	/* 初始化SPI1 */
	SPI_Init(SPI1, &SPI_InitStructure);

	/* 把使能SPI口的SS输出功能 GPIOA.4 */
	SPI_SSOutputCmd(SPI1,ENABLE);
	/* 使能SPI1 */
	SPI_Cmd(SPI1, ENABLE);

	network->cs_set(network,1);
	 /* ENC28J60软件复位 该函数可以改进 */
	network->write_op(network,ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
	/* 查询ESTAT.CLKRDY位 */
	while(!(network->read_bank(network,ESTAT)& ESTAT_CLKRDY));
	/* 设置接收缓冲区起始地址 该变量用于每次读取缓冲区时保留下一个包的首地址 */
	network->next_pack_ptr = RXSTART_INIT;
	/* 设置接收缓冲区 读指针 */
	enc28j60_write(ERXRDPTL, RXSTART_INIT&0xFF);
	enc28j60_write(ERXRDPTH, RXSTART_INIT>>8);
	/* 设置接收缓冲区 结束指针 */
	enc28j60_write(ERXNDL, RXSTOP_INIT&0xFF);
	enc28j60_write(ERXNDH, RXSTOP_INIT>>8);
	/* 设置发送缓冲区 起始指针 */
	enc28j60_write(ETXSTL, TXSTART_INIT&0xFF);
	enc28j60_write(ETXSTH, TXSTART_INIT>>8);
	/* 设置发送缓冲区 结束指针 */
	enc28j60_write(ETXNDL, TXSTOP_INIT&0xFF);
	enc28j60_write(ETXNDH, TXSTOP_INIT>>8);
	/* 使能单播过滤 使能CRC校验 使能 格式匹配自动过滤*/
	enc28j60_write(ERXFCON, ERXFCON_UCEN|ERXFCON_CRCEN|ERXFCON_PMEN);
	enc28j60_write(EPMM0, 0x3f);
	enc28j60_write(EPMM1, 0x30);
	enc28j60_write(EPMCSL, 0xf9);
	enc28j60_write(EPMCSH, 0xf7);
	/* 使能MAC接收 允许MAC发送暂停控制帧 当接收到暂停控制帧时停止发送*/
	/* 数据手册34页 */
	enc28j60_write(MACON1, MACON1_MARXEN|MACON1_TXPAUS|MACON1_RXPAUS);
	/* 退出复位状态 */
	enc28j60_write(MACON2, 0x00);
	/* 用0填充所有短帧至60字节长 并追加一个CRC 发送CRC使能 帧长度校验使能 MAC全双工使能*/
	/* 提示 由于ENC28J60不支持802.3的自动协商机制， 所以对端的网络卡需要强制设置为全双工 */
	enc28j60_writeop(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN|MACON3_FULDPX);
	/* 填入默认值 */
	enc28j60_write(MAIPGL, 0x12);
	/* 填入默认值 */
	enc28j60_write(MAIPGH, 0x0C);
	/* 填入默认值 */
	enc28j60_write(MABBIPG, 0x15);
	/* 最大帧长度 */
	enc28j60_write(MAMXFLL, MAX_FRAMELEN & 0xFF);
	enc28j60_write(MAMXFLH, MAX_FRAMELEN >> 8);
	/* 写入MAC地址 */
	enc28j60_write(MAADR5, mac_addr[0]);
	enc28j60_write(MAADR4, mac_addr[1]);
	enc28j60_write(MAADR3, mac_addr[2]);
	enc28j60_write(MAADR2, mac_addr[3]);
	enc28j60_write(MAADR1, mac_addr[4]);
	enc28j60_write(MAADR0, mac_addr[5]);
	/* 配置PHY为全双工  LEDB为拉电流 */
	enc28j60_writephy(PHCON1, PHCON1_PDPXMD);
	/* LED状态 */
	enc28j60_writephy(PHLCON,0x0476);
	/* 半双工回环禁止 */
	enc28j60_writephy(PHCON2, PHCON2_HDLDIS);
	/* 返回BANK0 */
	enc28j60_setbank(ECON1);
	/* 使能中断 全局中断 接收中断 接收错误中断 */
	enc28j60_writeop(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE|EIE_PKTIE|EIE_RXERIE);
	/* 接收使能位 */
	enc28j60_writeop(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
}

