/*
 * TIMER.c
 *
 *  Created on: Nov 1, 2022
 *      Author: kMermah
 */
#include "common_macros.h" /* For GET_BIT Macro */
#include "gpio.h"
#include <avr/io.h>
#include "TIMER.h"
#include "std_types.h"
#include <avr/interrupt.h>

static volatile void (*g_callBackPtr)(void) = NULL_PTR;

#if ( OVERFLOW==1)
ISR(TIMER1_OVF_vect)
{
	if(g_callBackPtr != NULL_PTR)
		{
			/* Call the Call Back function in the application after the edge is detected */
			(*g_callBackPtr)(); /* another method to call the function using pointer to function g_callBackPtr(); */
		}
}
#else
ISR(TIMER1_COMPA_vect){
	if(g_callBackPtr != NULL_PTR)
		{
			/* Call the Call Back function in the application after the edge is detected */
			(*g_callBackPtr)(); /* another method to call the function using pointer to function g_callBackPtr(); */
		}
}
#endif
void Timer1_init(const Timer1_ConfigType * Config_Ptr){

		TCNT1=(Config_Ptr->initial_value);
		TCCR1A |= (1<<FOC1A) | (1<<FOC1B);
		if ((Config_Ptr->mode)==1){
			OCR1A=Config_Ptr->compare_value;
			TIMSK = (1<<OCIE1A);
		}
		else{
			TIMSK = (1<<TOIE1);
		}
		TCCR1B = ( TCCR1B & 0xF8 ) | ( Config_Ptr->prescaler );
		TCCR1B = ( TCCR1B & 0xF7 ) | ( (Config_Ptr->mode)<<3 );

}

 void Timer1_deInit(void){
	 /* Clear All Timer1 Registers */
	 	TCCR1A = 0;
	 	TCCR1B = 0;
	 	TCNT1 = 0;
	 	OCR1A=0;
	 	TIMSK &= ~(1<<OCIE1A);
	 	TIMSK &= ~(1<<TOIE1);
 }

void Timer1_setCallBack(void(*a_ptr)(void)){
	/* Save the address of the Call back function in a global variable */
		g_callBackPtr = a_ptr;
}


