#ifndef  __EGG_H__
#define  __EGG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f10x.h"

struct gpio_t {
	GPIO_TypeDef* 	port;
	uint16_t        pin;
};

#include "led.h"
#include "networkcard.h"

#ifdef __cplusplus
}
#endif

#endif
