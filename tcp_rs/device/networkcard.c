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
	while(!(network->read_bank(network,ESTAT)& ESTAT_CLKRDY));
	/* ���ý��ջ�������ʼ��ַ �ñ�������ÿ�ζ�ȡ������ʱ������һ�������׵�ַ */
	network->next_pack_ptr = RXSTART_INIT;
	/* ���ý��ջ����� ��ָ�� */
	enc28j60_write(ERXRDPTL, RXSTART_INIT&0xFF);
	enc28j60_write(ERXRDPTH, RXSTART_INIT>>8);
	/* ���ý��ջ����� ����ָ�� */
	enc28j60_write(ERXNDL, RXSTOP_INIT&0xFF);
	enc28j60_write(ERXNDH, RXSTOP_INIT>>8);
	/* ���÷��ͻ����� ��ʼָ�� */
	enc28j60_write(ETXSTL, TXSTART_INIT&0xFF);
	enc28j60_write(ETXSTH, TXSTART_INIT>>8);
	/* ���÷��ͻ����� ����ָ�� */
	enc28j60_write(ETXNDL, TXSTOP_INIT&0xFF);
	enc28j60_write(ETXNDH, TXSTOP_INIT>>8);
	/* ʹ�ܵ������� ʹ��CRCУ�� ʹ�� ��ʽƥ���Զ�����*/
	enc28j60_write(ERXFCON, ERXFCON_UCEN|ERXFCON_CRCEN|ERXFCON_PMEN);
	enc28j60_write(EPMM0, 0x3f);
	enc28j60_write(EPMM1, 0x30);
	enc28j60_write(EPMCSL, 0xf9);
	enc28j60_write(EPMCSH, 0xf7);
	/* ʹ��MAC���� ����MAC������ͣ����֡ �����յ���ͣ����֡ʱֹͣ����*/
	/* �����ֲ�34ҳ */
	enc28j60_write(MACON1, MACON1_MARXEN|MACON1_TXPAUS|MACON1_RXPAUS);
	/* �˳���λ״̬ */
	enc28j60_write(MACON2, 0x00);
	/* ��0������ж�֡��60�ֽڳ� ��׷��һ��CRC ����CRCʹ�� ֡����У��ʹ�� MACȫ˫��ʹ��*/
	/* ��ʾ ����ENC28J60��֧��802.3���Զ�Э�̻��ƣ� ���ԶԶ˵����翨��Ҫǿ������Ϊȫ˫�� */
	enc28j60_writeop(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN|MACON3_FULDPX);
	/* ����Ĭ��ֵ */
	enc28j60_write(MAIPGL, 0x12);
	/* ����Ĭ��ֵ */
	enc28j60_write(MAIPGH, 0x0C);
	/* ����Ĭ��ֵ */
	enc28j60_write(MABBIPG, 0x15);
	/* ���֡���� */
	enc28j60_write(MAMXFLL, MAX_FRAMELEN & 0xFF);
	enc28j60_write(MAMXFLH, MAX_FRAMELEN >> 8);
	/* д��MAC��ַ */
	enc28j60_write(MAADR5, mac_addr[0]);
	enc28j60_write(MAADR4, mac_addr[1]);
	enc28j60_write(MAADR3, mac_addr[2]);
	enc28j60_write(MAADR2, mac_addr[3]);
	enc28j60_write(MAADR1, mac_addr[4]);
	enc28j60_write(MAADR0, mac_addr[5]);
	/* ����PHYΪȫ˫��  LEDBΪ������ */
	enc28j60_writephy(PHCON1, PHCON1_PDPXMD);
	/* LED״̬ */
	enc28j60_writephy(PHLCON,0x0476);
	/* ��˫���ػ���ֹ */
	enc28j60_writephy(PHCON2, PHCON2_HDLDIS);
	/* ����BANK0 */
	enc28j60_setbank(ECON1);
	/* ʹ���ж� ȫ���ж� �����ж� ���մ����ж� */
	enc28j60_writeop(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE|EIE_PKTIE|EIE_RXERIE);
	/* ����ʹ��λ */
	enc28j60_writeop(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);
}

