#include "networkcard.h"

void network_cs_set(struct network_t * network,uint8_t var){
	GPIO_WriteBit(network->cs.port,network->cs.pin,(BitAction)(var));
}

uint8_t spi_send_data(uint8_t byte) {
	/* 等待发送缓冲寄存器为空 */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	/* 发送数据 */
	SPI_I2S_SendData(SPI1, byte);
	/* 等待接收缓冲寄存器为非空 */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	return SPI_I2S_ReceiveData(SPI1);
}

uint8_t spi_receive_data(void) {
	/* 等待发送缓冲寄存器为空 */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	/* 发送数据,通过发送0xFF,获得返回数据 */
	SPI_I2S_SendData(SPI1, 0xFF);
	/* 等待接收缓冲寄存器为非空 */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	/* 返回从SPI通信中接收到的数据 */
	return SPI_I2S_ReceiveData(SPI1);
}

void network_init(struct network_t * network,uint8_t * mac_addr){
	GPIO_InitTypeDef  GPIO_InitStructure;
	SPI_InitTypeDef SPI_InitStructure;
	//open clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE );
	clock_set(network->cs.port);
	clock_set(network->sck.port);
	clock_set(network->miso.port);
	clock_set(network->mosi.port);
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
	while(!(network->read_data(network,ESTAT)& ESTAT_CLKRDY));
	/* 设置接收缓冲区起始地址 该变量用于每次读取缓冲区时保留下一个包的首地址 */
	network->next_pack_ptr = RXSTART_INIT;
	/* 设置接收缓冲区 读指针 */
	network->write_data(network,ERXRDPTL, RXSTART_INIT&0xFF);
	network->write_data(network,ERXRDPTH, RXSTART_INIT>>8);
	/* 设置接收缓冲区 结束指针 */
	network->write_data(network,ERXNDL, RXSTOP_INIT&0xFF);
	network->write_data(network,ERXNDH, RXSTOP_INIT>>8);
	/* 设置发送缓冲区 起始指针 */
	network->write_data(network,ETXSTL, TXSTART_INIT&0xFF);
	network->write_data(network,ETXSTH, TXSTART_INIT>>8);
	/* 设置发送缓冲区 结束指针 */
	network->write_data(network,ETXNDL, TXSTOP_INIT&0xFF);
	network->write_data(network,ETXNDH, TXSTOP_INIT>>8);
	/* 使能单播过滤 使能CRC校验 使能 格式匹配自动过滤*/
	network->write_data(network,ERXFCON, ERXFCON_UCEN|ERXFCON_CRCEN|ERXFCON_PMEN);
	network->write_data(network,EPMM0, 0x3f);
	network->write_data(network,EPMM1, 0x30);
	network->write_data(network,EPMCSL, 0xf9);
	network->write_data(network,EPMCSH, 0xf7);
	/* 使能MAC接收 允许MAC发送暂停控制帧 当接收到暂停控制帧时停止发送*/
	/* 数据手册34页 */
	network->write_data(network,MACON1, MACON1_MARXEN|MACON1_TXPAUS|MACON1_RXPAUS);
	/* 退出复位状态 */
	network->write_data(network,MACON2, 0x00);
	/* 用0填充所有短帧至60字节长 并追加一个CRC 发送CRC使能 帧长度校验使能 MAC全双工使能*/
	/* 提示 由于ENC28J60不支持802.3的自动协商机制， 所以对端的网络卡需要强制设置为全双工 */
	network->write_op(network,ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN|MACON3_FULDPX);
	/* 填入默认值 */
	network->write_data(network,MAIPGL, 0x12);
	/* 填入默认值 */
	network->write_data(network,MAIPGH, 0x0C);
	/* 填入默认值 */
	network->write_data(network,MABBIPG, 0x15);
	/* 最大帧长度 */
	network->write_data(network,MAMXFLL, MAX_FRAMELEN & 0xFF);
	network->write_data(network,MAMXFLH, MAX_FRAMELEN >> 8);
	/* 写入MAC地址 */
	network->write_data(network,MAADR5, mac_addr[0]);
	network->write_data(network,MAADR4, mac_addr[1]);
	network->write_data(network,MAADR3, mac_addr[2]);
	network->write_data(network,MAADR2, mac_addr[3]);
	network->write_data(network,MAADR1, mac_addr[4]);
	network->write_data(network,MAADR0, mac_addr[5]);
	/* 配置PHY为全双工  LEDB为拉电流 */
	network->write_phy(network,PHCON1, PHCON1_PDPXMD);
	/* LED状态 */
	network->write_phy(network,PHLCON,0x0476);
	/* 半双工回环禁止 */
	network->write_phy(network,PHCON2, PHCON2_HDLDIS);
	/* 返回BANK0 */
	network->set_bank(network,ECON1);
	/* 使能中断 全局中断 接收中断 接收错误中断 */
	network->write_op(network,ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE|EIE_PKTIE|EIE_RXERIE);
	/* 接收使能位 */
	network->write_op(network,ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
}

uint8_t network_read_op(struct network_t * network,uint8_t op,uint8_t address) {
	uint8_t dat = 0;
	network->cs_set(network,0);
	/* 操作码和地址 */
	dat = op | (address & ADDR_MASK);
	network->spi_send_data(dat);
	dat = network->spi_send_data(0xff);
	/* 如果是MAC和MII寄存器，第一个读取的字节无效，该信息包含在地址的最高位*/
	if(address & 0x80) {
		/* 再次通过SPI读取数据 */
		dat = network->spi_send_data(0xFF);
	}
	/* CS拉高 禁止ENC28J60 */
	network->cs_set(network,1);
	/* 返回数据 */
	return dat;
}

void network_write_op(struct network_t * network,uint8_t op,uint8_t address,uint8_t data) {
	uint8_t dat = 0;
	/* 使能ENC28J60 */
	network->cs_set(network,0);
	/* 通过SPI发送 操作码和寄存器地址 */
	dat = op | (address & ADDR_MASK);
	/* 通过SPI1发送数据 */
	network->spi_send_data(dat);
	/* 准备寄存器数值 */
	dat = data;
	/* 通过SPI发送数据 */
	network->spi_send_data(dat);
	/* 禁止ENC28J60 */
	network->cs_set(network,1);
}

void network_read_buf(struct network_t * network,uint8_t * pdata,uint16_t len) {
	/* 使能ENC28J60 */
	network->cs_set(network,0);
	/* 通过SPI发送读取缓冲区命令*/
	network->spi_send_data(ENC28J60_READ_BUF_MEM);
	/* 循环读取 */
	while(len) {
		len--;
		/* 读取数据 */
		*pdata = (unsigned char)network->spi_send_data(0);
		/* 地址指针累加 */
		pdata++;
	}
	/* 禁止ENC28J60 */
	network->cs_set(network,1);
}

void network_write_buf(struct network_t * network,uint8_t * pdata,uint16_t len) {
	/* 使能ENC28J60 */
	network->cs_set(network,0);
	/* 通过SPI发送写取缓冲区命令*/
	network->spi_send_data(ENC28J60_WRITE_BUF_MEM);
	/* 循环发送 */
	while(len) {
		len--;
		/* 发送数据 */
		network->spi_send_data(*pdata);
		/* 地址指针累加 */
		pdata++;
	}
	/* 禁止ENC28J60 */
	network->cs_set(network,1);
}

void network_set_bank(struct network_t * network,uint8_t address) {
	/* 计算本次寄存器地址在存取区域的位置 */
	if((address & BANK_MASK) != network->bank) {
		/* 清除ECON1的BSEL1 BSEL0 详见数据手册15页 */
		network->write_op(network,ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1|ECON1_BSEL0));
		/* 请注意寄存器地址的宏定义，bit6 bit5代码寄存器存储区域位置 */
		network->write_op(network,ENC28J60_BIT_FIELD_SET, ECON1, (address & BANK_MASK)>>5);
		/* 重新确定当前寄存器存储区域 */
		network->bank = (address & BANK_MASK);
	}
}

uint8_t network_read_data(struct network_t * network,uint8_t address) {
	/* 设定寄存器地址区域 */
	network->set_bank(network,address);
	/* 读取寄存器值 发送读寄存器命令和地址 */
	return network->read_op(network,ENC28J60_READ_CTRL_REG, address);
}

void network_write_data(struct network_t * network,uint8_t data,uint8_t address) {
	/* 设定寄存器地址区域 */
	network->set_bank(network,address);
	/* 写寄存器值 发送写寄存器命令和地址 */
	network->write_op(network,ENC28J60_WRITE_CTRL_REG, address, data);
}

void network_write_phy(struct network_t * network,uint8_t address,uint16_t data) {
	/* 向MIREGADR写入地址 详见数据手册19页*/
	network->write_data(network,MIREGADR, address);
	/* 写入低8位数据 */
	network->write_data(network,MIWRL, data);
	/* 写入高8位数据 */
	network->write_data(network,MIWRH, data>>8);
	/* 等待PHY寄存器写入完成 */
	while( network->read_data(network,MISTAT) & MISTAT_BUSY );
}

void network_packet_send(struct network_t * network,uint8_t address,uint8_t * packet,int16_t len) {
	/* 查询发送逻辑复位位 */
	while((network->read_data(network,ECON1) & ECON1_TXRTS)!= 0);
	/* 设置发送缓冲区起始地址 */
	network->write_data(network,EWRPTL, TXSTART_INIT & 0xFF);
	network->write_data(network,EWRPTH, TXSTART_INIT >> 8);
	/* 设置发送缓冲区结束地址 该值对应发送数据包长度*/
	network->write_data(network,ETXNDL, (TXSTART_INIT + len) & 0xFF);
	network->write_data(network,ETXNDH, (TXSTART_INIT + len) >>8);
	/* 发送之前发送控制包格式字 */
	network->write_op(network,ENC28J60_WRITE_BUF_MEM, 0, 0x00);
	/* 通过ENC28J60发送数据包 */
	network->write_buf(network,packet,len );
	/* 开始发送*/
	network->write_op(network,ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
	/* 复位发送逻辑的问题。参见 Rev. B4 Silicon Errata point 12. */
	if( (network->read_data(network,EIR) & EIR_TXERIF) ) {
		network->set_bank(network,ECON1);
		network->write_op(network,ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRTS);
	}
}

uint16_t network_packet_read(struct network_t * network,uint8_t address,uint8_t * packet,int16_t maxlen) {
	uint16_t rxstat;
	uint16_t len;
	/* 是否收到以太网数据包 */
	if( network->read_data(network,EPKTCNT) == 0 ){
		return(0);
	}
	/* 设置接收缓冲器读指针 */
	network->write_data(network,ERDPTL, (network->next_pack_ptr));
	network->write_data(network,ERDPTH, (network->next_pack_ptr)>>8);
	/* 接收数据包结构示例 数据手册43页 */
	/* 读下一个包的指针 */
	network->next_pack_ptr  = network->read_op(network,ENC28J60_READ_BUF_MEM, 0);
	network->next_pack_ptr |= network->read_op(network,ENC28J60_READ_BUF_MEM, 0)<<8;
	/* 读包的长度 */
	len  = network->read_op(network,ENC28J60_READ_BUF_MEM, 0);
	len |= network->read_op(network,ENC28J60_READ_BUF_MEM, 0)<<8;
	/* 删除CRC计数 */
	len-= 4;
	/* 读取接收状态 */
	rxstat  = network->read_op(network,ENC28J60_READ_BUF_MEM, 0);
	rxstat |= network->read_op(network,ENC28J60_READ_BUF_MEM, 0) << 8;
	/* 限制检索的长度	*/
	if (len > maxlen-1){
		len = maxlen-1;
	}
	/* 检查CRC和符号错误 */
	/* ERXFCON.CRCEN是默认设置。通常我们不需要检查 */
	if ((rxstat & 0x80)==0){
		len=0;
	} else {
		/* 从接收缓冲器中复制数据包 */
		network->read_buf(network,packet,len );
	}
	/* 移动接收缓冲区 读指针*/
	network->write_data(network,ERXRDPTL, (network->next_pack_ptr));
	network->write_data(network,ERXRDPTH, (network->next_pack_ptr)>>8);
	/* 数据包递减 */
	network->write_op(network,ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
	/* 返回长度 */
	return(len);
}


