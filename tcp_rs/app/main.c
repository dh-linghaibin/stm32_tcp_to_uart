//*****************************************************************************
//
//! \file main.c
//! \brief main application
//! \version 1.0.0.0
//! \date $Creat_time$
//! \author $Creat_author$
//! \copy
//!
//! Copyright (c) 2014 CooCox.  All rights reserved.
//
//! \addtogroup project
//! @{
//! \addtogroup main
//! @{
//*****************************************************************************
#include "egg.h"

int main(void) {
	struct led_t led = {
		{GPIOB, GPIO_Pin_11},
		{GPIOB, GPIO_Pin_0},
		{GPIOB, GPIO_Pin_10},
		{GPIOB, GPIO_Pin_1},
		&led_init,
		&led_set_run,
		&led_set_can,
		&led_set_rs232,
		&led_set_rs485,
		&led_run_tiger,
		&led_can_tiger,
		&led_rs232_tiger,
		&led_rs485_tiger,
	};
	led.init(&led);
	led.set_run(&led,0);
	while(1) {

	}
}
//! @}
//! @}
