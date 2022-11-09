/*
 * buzzer.c
 *
 *  Created on: Nov 1, 2022
 *      Author: kMermah
 */
#include "BUZZER.h"
#include "common_macros.h" /* For GET_BIT Macro */
#include "gpio.h"

void Buzzer_init(){
	GPIO_setupPinDirection(BUZZER_PORT_ID,BUZZER_PIN_ID,PIN_OUTPUT);
	GPIO_writePin(BUZZER_PORT_ID,BUZZER_PIN_ID,LOGIC_LOW);
}
void Buzzer_on(void){
	GPIO_writePin(BUZZER_PORT_ID,BUZZER_PIN_ID,LOGIC_HIGH);
}

void Buzzer_off(void){
	GPIO_writePin(BUZZER_PORT_ID,BUZZER_PIN_ID,LOGIC_LOW);
}

