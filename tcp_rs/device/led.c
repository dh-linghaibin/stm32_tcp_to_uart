#include "led.h"

void clock_set(GPIO_TypeDef* port){
    if (port == GPIOA)
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    else if(port == GPIOB)
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    else if(port == GPIOC)
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    else if(port == GPIOD)
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    else if(port == GPIOE)
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
    else if(port == GPIOF)
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
}

void led_init(struct led_t * led) {
	GPIO_InitTypeDef GPIO_InitStructure;

	clock_set(led->run.port);
	clock_set(led->can.port);
	clock_set(led->rs232.port);
	clock_set(led->rs485.port);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin = led->run.pin;
    GPIO_Init(led->run.port, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = led->can.pin;
    GPIO_Init(led->can.port, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = led->rs232.pin;
    GPIO_Init(led->rs232.port, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = led->rs485.pin;
    GPIO_Init(led->rs485.port, &GPIO_InitStructure);

    GPIO_WriteBit(led->run.port,led->run.pin,(BitAction)(1));
    GPIO_WriteBit(led->can.port,led->can.pin,(BitAction)(1));
    GPIO_WriteBit(led->rs232.port,led->rs232.pin,(BitAction)(1));
    GPIO_WriteBit(led->rs485.port,led->rs485.pin,(BitAction)(1));
}

void led_set_run(struct led_t * led,uint8_t var) {
    GPIO_WriteBit(led->run.port,led->run.pin,(BitAction)(var));
}

void led_set_can(struct led_t * led,uint8_t var) {
    GPIO_WriteBit(led->can.port,led->can.pin,(BitAction)(var));
}

void led_set_rs232(struct led_t * led,uint8_t var) {
    GPIO_WriteBit(led->rs232.port,led->rs232.pin,(BitAction)(var));
}

void led_set_rs485(struct led_t * led,uint8_t var) {
    GPIO_WriteBit(led->rs485.port,led->rs485.pin,(BitAction)(var));
}

void led_run_tiger(struct led_t * led) {
    GPIO_WriteBit(led->run.port, led->run.pin,
		               (BitAction)((1-GPIO_ReadOutputDataBit(led->run.port, led->run.pin))));
}

void led_can_tiger(struct led_t * led) {
    GPIO_WriteBit(led->can.port, led->can.pin,
		               (BitAction)((1-GPIO_ReadOutputDataBit(led->can.port, led->can.pin))));
}

void led_rs232_tiger(struct led_t * led) {
    GPIO_WriteBit(led->rs232.port, led->rs232.pin,
		               (BitAction)((1-GPIO_ReadOutputDataBit(led->rs232.port, led->rs232.pin))));
}

void led_rs485_tiger(struct led_t * led) {
    GPIO_WriteBit(led->rs485.port, led->rs485.pin,
		               (BitAction)((1-GPIO_ReadOutputDataBit(led->rs485.port, led->rs485.pin))));
}
/***************************************************************END OF FILE****/
