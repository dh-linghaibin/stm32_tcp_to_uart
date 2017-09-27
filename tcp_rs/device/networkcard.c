#include "networkcard.h"

void network_cs_set(struct network_t * network,uint8_t var){
	GPIO_WriteBit(network->cs.port,network->cs.pin,(BitAction)(var));
}

uint8_t spi_send_data(uint8_t byte) {
	/* �ȴ����ͻ���Ĵ���Ϊ�� */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	/* �������� */
	SPI_I2S_SendData(SPI1, byte);
	/* �ȴ����ջ���Ĵ���Ϊ�ǿ� */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	return SPI_I2S_ReceiveData(SPI1);
}

uint8_t spi_receive_data(void) {
	/* �ȴ����ͻ���Ĵ���Ϊ�� */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	/* ��������,ͨ������0xFF,��÷������� */
	SPI_I2S_SendData(SPI1, 0xFF);
	/* �ȴ����ջ���Ĵ���Ϊ�ǿ� */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	/* ���ش�SPIͨ���н��յ������� */
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
	/* ����������� */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(network->sck.port, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = network->mosi.pin;
	GPIO_Init(network->mosi.port, &GPIO_InitStructure);
	/* SPI1 MISO@GPIOA.6 */
	GPIO_InitStructure.GPIO_Pin = network->miso.pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	/* �������� */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(network->miso.port, &GPIO_InitStructure);
	/* enc28j60 CS @GPIOA.4 */
	GPIO_InitStructure.GPIO_Pin = network->cs.pin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(network->cs.port, &GPIO_InitStructure);
	/* ˫��˫��ȫ˫�� */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	/* ����ģʽ */
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	/* 8λ֡�ṹ */
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	/* ʱ�ӿ���ʱΪ�� */
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	/* ��1�������ز������� */
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	/* MSS �˿�������� */
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	/* SPIʱ�� 72Mhz / 8 = 9M */
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
	/* ���ݴ����λ��ǰ */
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;

	SPI_InitStructure.SPI_CRCPolynomial = 7;
	/* ��ʼ��SPI1 */
	SPI_Init(SPI1, &SPI_InitStructure);

	/* ��ʹ��SPI�ڵ�SS������� GPIOA.4 */
	SPI_SSOutputCmd(SPI1,ENABLE);
	/* ʹ��SPI1 */
	SPI_Cmd(SPI1, ENABLE);

	network->cs_set(network,1);
	 /* ENC28J60�����λ �ú������ԸĽ� */
	network->write_op(network,ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
	/* ��ѯESTAT.CLKRDYλ */
	while(!(network->read_data(network,ESTAT)& ESTAT_CLKRDY));
	/* ���ý��ջ�������ʼ��ַ �ñ�������ÿ�ζ�ȡ������ʱ������һ�������׵�ַ */
	network->next_pack_ptr = RXSTART_INIT;
	/* ���ý��ջ����� ��ָ�� */
	network->write_data(network,ERXRDPTL, RXSTART_INIT&0xFF);
	network->write_data(network,ERXRDPTH, RXSTART_INIT>>8);
	/* ���ý��ջ����� ����ָ�� */
	network->write_data(network,ERXNDL, RXSTOP_INIT&0xFF);
	network->write_data(network,ERXNDH, RXSTOP_INIT>>8);
	/* ���÷��ͻ����� ��ʼָ�� */
	network->write_data(network,ETXSTL, TXSTART_INIT&0xFF);
	network->write_data(network,ETXSTH, TXSTART_INIT>>8);
	/* ���÷��ͻ����� ����ָ�� */
	network->write_data(network,ETXNDL, TXSTOP_INIT&0xFF);
	network->write_data(network,ETXNDH, TXSTOP_INIT>>8);
	/* ʹ�ܵ������� ʹ��CRCУ�� ʹ�� ��ʽƥ���Զ�����*/
	network->write_data(network,ERXFCON, ERXFCON_UCEN|ERXFCON_CRCEN|ERXFCON_PMEN);
	network->write_data(network,EPMM0, 0x3f);
	network->write_data(network,EPMM1, 0x30);
	network->write_data(network,EPMCSL, 0xf9);
	network->write_data(network,EPMCSH, 0xf7);
	/* ʹ��MAC���� ����MAC������ͣ����֡ �����յ���ͣ����֡ʱֹͣ����*/
	/* �����ֲ�34ҳ */
	network->write_data(network,MACON1, MACON1_MARXEN|MACON1_TXPAUS|MACON1_RXPAUS);
	/* �˳���λ״̬ */
	network->write_data(network,MACON2, 0x00);
	/* ��0������ж�֡��60�ֽڳ� ��׷��һ��CRC ����CRCʹ�� ֡����У��ʹ�� MACȫ˫��ʹ��*/
	/* ��ʾ ����ENC28J60��֧��802.3���Զ�Э�̻��ƣ� ���ԶԶ˵����翨��Ҫǿ������Ϊȫ˫�� */
	network->write_op(network,ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN|MACON3_FULDPX);
	/* ����Ĭ��ֵ */
	network->write_data(network,MAIPGL, 0x12);
	/* ����Ĭ��ֵ */
	network->write_data(network,MAIPGH, 0x0C);
	/* ����Ĭ��ֵ */
	network->write_data(network,MABBIPG, 0x15);
	/* ���֡���� */
	network->write_data(network,MAMXFLL, MAX_FRAMELEN & 0xFF);
	network->write_data(network,MAMXFLH, MAX_FRAMELEN >> 8);
	/* д��MAC��ַ */
	network->write_data(network,MAADR5, mac_addr[0]);
	network->write_data(network,MAADR4, mac_addr[1]);
	network->write_data(network,MAADR3, mac_addr[2]);
	network->write_data(network,MAADR2, mac_addr[3]);
	network->write_data(network,MAADR1, mac_addr[4]);
	network->write_data(network,MAADR0, mac_addr[5]);
	/* ����PHYΪȫ˫��  LEDBΪ������ */
	network->write_phy(network,PHCON1, PHCON1_PDPXMD);
	/* LED״̬ */
	network->write_phy(network,PHLCON,0x0476);
	/* ��˫���ػ���ֹ */
	network->write_phy(network,PHCON2, PHCON2_HDLDIS);
	/* ����BANK0 */
	network->set_bank(network,ECON1);
	/* ʹ���ж� ȫ���ж� �����ж� ���մ����ж� */
	network->write_op(network,ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE|EIE_PKTIE|EIE_RXERIE);
	/* ����ʹ��λ */
	network->write_op(network,ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
}

uint8_t network_read_op(struct network_t * network,uint8_t op,uint8_t address) {
	uint8_t dat = 0;
	network->cs_set(network,0);
	/* ������͵�ַ */
	dat = op | (address & ADDR_MASK);
	network->spi_send_data(dat);
	dat = network->spi_send_data(0xff);
	/* �����MAC��MII�Ĵ�������һ����ȡ���ֽ���Ч������Ϣ�����ڵ�ַ�����λ*/
	if(address & 0x80) {
		/* �ٴ�ͨ��SPI��ȡ���� */
		dat = network->spi_send_data(0xFF);
	}
	/* CS���� ��ֹENC28J60 */
	network->cs_set(network,1);
	/* �������� */
	return dat;
}

void network_write_op(struct network_t * network,uint8_t op,uint8_t address,uint8_t data) {
	uint8_t dat = 0;
	/* ʹ��ENC28J60 */
	network->cs_set(network,0);
	/* ͨ��SPI���� ������ͼĴ�����ַ */
	dat = op | (address & ADDR_MASK);
	/* ͨ��SPI1�������� */
	network->spi_send_data(dat);
	/* ׼���Ĵ�����ֵ */
	dat = data;
	/* ͨ��SPI�������� */
	network->spi_send_data(dat);
	/* ��ֹENC28J60 */
	network->cs_set(network,1);
}

void network_read_buf(struct network_t * network,uint8_t * pdata,uint16_t len) {
	/* ʹ��ENC28J60 */
	network->cs_set(network,0);
	/* ͨ��SPI���Ͷ�ȡ����������*/
	network->spi_send_data(ENC28J60_READ_BUF_MEM);
	/* ѭ����ȡ */
	while(len) {
		len--;
		/* ��ȡ���� */
		*pdata = (unsigned char)network->spi_send_data(0);
		/* ��ַָ���ۼ� */
		pdata++;
	}
	/* ��ֹENC28J60 */
	network->cs_set(network,1);
}

void network_write_buf(struct network_t * network,uint8_t * pdata,uint16_t len) {
	/* ʹ��ENC28J60 */
	network->cs_set(network,0);
	/* ͨ��SPI����дȡ����������*/
	network->spi_send_data(ENC28J60_WRITE_BUF_MEM);
	/* ѭ������ */
	while(len) {
		len--;
		/* �������� */
		network->spi_send_data(*pdata);
		/* ��ַָ���ۼ� */
		pdata++;
	}
	/* ��ֹENC28J60 */
	network->cs_set(network,1);
}

void network_set_bank(struct network_t * network,uint8_t address) {
	/* ���㱾�μĴ�����ַ�ڴ�ȡ�����λ�� */
	if((address & BANK_MASK) != network->bank) {
		/* ���ECON1��BSEL1 BSEL0 ��������ֲ�15ҳ */
		network->write_op(network,ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1|ECON1_BSEL0));
		/* ��ע��Ĵ�����ַ�ĺ궨�壬bit6 bit5����Ĵ����洢����λ�� */
		network->write_op(network,ENC28J60_BIT_FIELD_SET, ECON1, (address & BANK_MASK)>>5);
		/* ����ȷ����ǰ�Ĵ����洢���� */
		network->bank = (address & BANK_MASK);
	}
}

uint8_t network_read_data(struct network_t * network,uint8_t address) {
	/* �趨�Ĵ�����ַ���� */
	network->set_bank(network,address);
	/* ��ȡ�Ĵ���ֵ ���Ͷ��Ĵ�������͵�ַ */
	return network->read_op(network,ENC28J60_READ_CTRL_REG, address);
}

void network_write_data(struct network_t * network,uint8_t data,uint8_t address) {
	/* �趨�Ĵ�����ַ���� */
	network->set_bank(network,address);
	/* д�Ĵ���ֵ ����д�Ĵ�������͵�ַ */
	network->write_op(network,ENC28J60_WRITE_CTRL_REG, address, data);
}

void network_write_phy(struct network_t * network,uint8_t address,uint16_t data) {
	/* ��MIREGADRд���ַ ��������ֲ�19ҳ*/
	network->write_data(network,MIREGADR, address);
	/* д���8λ���� */
	network->write_data(network,MIWRL, data);
	/* д���8λ���� */
	network->write_data(network,MIWRH, data>>8);
	/* �ȴ�PHY�Ĵ���д����� */
	while( network->read_data(network,MISTAT) & MISTAT_BUSY );
}

void network_packet_send(struct network_t * network,uint8_t address,uint8_t * packet,int16_t len) {
	/* ��ѯ�����߼���λλ */
	while((network->read_data(network,ECON1) & ECON1_TXRTS)!= 0);
	/* ���÷��ͻ�������ʼ��ַ */
	network->write_data(network,EWRPTL, TXSTART_INIT & 0xFF);
	network->write_data(network,EWRPTH, TXSTART_INIT >> 8);
	/* ���÷��ͻ�����������ַ ��ֵ��Ӧ�������ݰ�����*/
	network->write_data(network,ETXNDL, (TXSTART_INIT + len) & 0xFF);
	network->write_data(network,ETXNDH, (TXSTART_INIT + len) >>8);
	/* ����֮ǰ���Ϳ��ư���ʽ�� */
	network->write_op(network,ENC28J60_WRITE_BUF_MEM, 0, 0x00);
	/* ͨ��ENC28J60�������ݰ� */
	network->write_buf(network,packet,len );
	/* ��ʼ����*/
	network->write_op(network,ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);
	/* ��λ�����߼������⡣�μ� Rev. B4 Silicon Errata point 12. */
	if( (network->read_data(network,EIR) & EIR_TXERIF) ) {
		network->set_bank(network,ECON1);
		network->write_op(network,ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRTS);
	}
}

uint16_t network_packet_read(struct network_t * network,uint8_t address,uint8_t * packet,int16_t maxlen) {
	uint16_t rxstat;
	uint16_t len;
	/* �Ƿ��յ���̫�����ݰ� */
	if( network->read_data(network,EPKTCNT) == 0 ){
		return(0);
	}
	/* ���ý��ջ�������ָ�� */
	network->write_data(network,ERDPTL, (network->next_pack_ptr));
	network->write_data(network,ERDPTH, (network->next_pack_ptr)>>8);
	/* �������ݰ��ṹʾ�� �����ֲ�43ҳ */
	/* ����һ������ָ�� */
	network->next_pack_ptr  = network->read_op(network,ENC28J60_READ_BUF_MEM, 0);
	network->next_pack_ptr |= network->read_op(network,ENC28J60_READ_BUF_MEM, 0)<<8;
	/* �����ĳ��� */
	len  = network->read_op(network,ENC28J60_READ_BUF_MEM, 0);
	len |= network->read_op(network,ENC28J60_READ_BUF_MEM, 0)<<8;
	/* ɾ��CRC���� */
	len-= 4;
	/* ��ȡ����״̬ */
	rxstat  = network->read_op(network,ENC28J60_READ_BUF_MEM, 0);
	rxstat |= network->read_op(network,ENC28J60_READ_BUF_MEM, 0) << 8;
	/* ���Ƽ����ĳ���	*/
	if (len > maxlen-1){
		len = maxlen-1;
	}
	/* ���CRC�ͷ��Ŵ��� */
	/* ERXFCON.CRCEN��Ĭ�����á�ͨ�����ǲ���Ҫ��� */
	if ((rxstat & 0x80)==0){
		len=0;
	} else {
		/* �ӽ��ջ������и������ݰ� */
		network->read_buf(network,packet,len );
	}
	/* �ƶ����ջ����� ��ָ��*/
	network->write_data(network,ERXRDPTL, (network->next_pack_ptr));
	network->write_data(network,ERXRDPTH, (network->next_pack_ptr)>>8);
	/* ���ݰ��ݼ� */
	network->write_op(network,ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
	/* ���س��� */
	return(len);
}


