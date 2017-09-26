#ifndef __LED_H
#define __LED_H

#ifdef __cplusplus
extern "C" {
#endif

#include "egg.h"

typedef enum TYPE_LED_SET{
    ON = 1,
    OFF = 0
}TYPE_LED_SET;



struct led_t {
	struct gpio_t run;
	struct gpio_t can;
	struct gpio_t rs485;
	struct gpio_t rs232;

    void (*init)(struct led_t * led);
    void (*set_run)(struct led_t * led,uint8_t var);
    void (*set_can)(struct led_t * led,uint8_t var);
    void (*set_rs485)(struct led_t * led,uint8_t var);
    void (*set_rs232)(struct led_t * led,uint8_t var);

    void (*tag_run)(struct led_t * led);
    void (*tag_can)(struct led_t * led);
    void (*tag_rs485)(struct led_t * led);
    void (*tag_rs232)(struct led_t * led);
};

void clock_set(GPIO_TypeDef* port);

void led_init(struct led_t * led);
void led_set_run(struct led_t * led,uint8_t var);
void led_set_can(struct led_t * led,uint8_t var);
void led_set_rs232(struct led_t * led,uint8_t var);
void led_set_rs485(struct led_t * led,uint8_t var);
void led_run_tiger(struct led_t * led);
void led_can_tiger(struct led_t * led);
void led_rs232_tiger(struct led_t * led);
void led_rs485_tiger(struct led_t * led);

#ifdef __cplusplus
}
#endif

#endif /*__LED__*/
