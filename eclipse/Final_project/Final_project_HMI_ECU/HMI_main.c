/*
 * main.c
 *
 *  Created on: Oct 31, 2022
 *      Author: kMermah
 */
#include <util/delay.h> /* For the delay functions */
#include "common_macros.h" /* For GET_BIT Macro */
#include "lcd.h"
#include "gpio.h"
#include "keypad.h"
#include <avr/io.h>
#include "uart.h"
#include "std_types.h"
#include "TIMER.h"

#define MC2_READY 0x10 //to send if the reciever is ready
#define Matched 0x11 //to send if the two password are matched
#define contin 0x0D  // to tell the control_ECU that the user press enter
#define NotMatched 0xFF //to send if the two password are not matched
Timer1_ConfigType timer_config = {0,23438,F_CPU_1024,COMPARE};
/*
 timer configration struct with
 1- 0 initial value
 2- 23438 OCR value which means 3 sec
 3- prescaler 1024
 4- compare mode
 */
char pass[5],reenter_pass[5],ans,tick=0,succes=0;
/*
 pass array to store first password before send it by uart
 reenter pass to store secound password before send it by uart
 ans for store + or - in the system
 tick the counter of the timer "increace every 3 sec"
 success to now if the passwords are matched --> success =1 in this case
 */
// timer call back function
void timer_count (){
	tick++;

}
/* choose pass function take two passwords from the user
 *  and check if matched or not to set the new password of the system
*/
void choose_pass(void){
	int i=0;
	LCD_clearScreen();
	LCD_moveCursor(0,0);
	LCD_displayString("plz enter pass:");
	for(i=0 ; i<5 ; i++){ // loop to take 5 inputs from the user
		while(KEYPAD_getPressedKey()==13);
		pass[i]=KEYPAD_getPressedKey(); //store the inputs in pass array

		LCD_moveCursor(1,i);
		LCD_displayString("*"); //display * with each press
		_delay_ms(500);
		//_delay_ms(500);
		//while(UART_recieveByte() != MC2_READY){}
		UART_sendByte(pass[i]); //send password byte by byte to the control ecu
	}
	while(KEYPAD_getPressedKey()!=13); // polling until press enter
	_delay_ms(500);
	LCD_clearScreen();
	LCD_moveCursor(0,0);
	LCD_displayString("plz re-enter the:");
	LCD_moveCursor(1,0);
	LCD_displayString("same pass:");
	for(i=0 ; i<5 ; i++){ // loop to take 5 inputs from the user
		while(KEYPAD_getPressedKey()==13);
		reenter_pass[i]=KEYPAD_getPressedKey();//store the inputs in reenterpass array

		_delay_ms(500);
		LCD_moveCursor(1,i+10);
		LCD_displayString("*"); //display * with each press
		//while(UART_recieveByte() != MC2_READY){}
		UART_sendByte(reenter_pass[i]); //send password byte by byte to the control ecu
	}
	while(KEYPAD_getPressedKey()!=13); // polling until press enter
	UART_sendByte(contin); //send to the controller that user press enter
	_delay_ms(500);
	return ;
}
// function to take password from the user to check if the password is true or not
void enter_pass(void){
	int i=0;
	LCD_clearScreen();
	LCD_moveCursor(0,0);
	LCD_displayString("plz enter pass:");
	for(i=0 ; i<5 ; i++){ // loop to take 5 inputs from the user
		pass[i]=KEYPAD_getPressedKey(); //store the inputs in pass array
		_delay_ms(500);
		LCD_moveCursor(1,i);
		LCD_displayString("*"); //display * with each press


		UART_sendByte(pass[i]); //send password byte by byte to the control ecu
	}
	while(KEYPAD_getPressedKey()!=13);
	UART_sendByte(contin);// polling until press enter
	_delay_ms(500);
	return ;
}

int main(){

	SREG |= (1<<7); //enable I bit
/*
 UART config struct
 1- send 8 bits data
 2- using even parity error detector
 3- stop bit is one bit only
 4- baud rate = 9600
 */
	UART_ConfigType Config = {eight_bits,even_parity,one_bit,9600};
	// initialization of the timer
	Timer1_init(&timer_config);
	//choose timer_count function as call back functin
	Timer1_setCallBack(timer_count);
//initialization of the lcd
	LCD_init();

	//initialization of the UART with UART config struct
	UART_init( &Config);

	do { /* looping until the user enter
	       two matched password to be
	       the current password of the system */
		choose_pass(); //calling choose pass function
		//UART_sendByte(MC2_READY);
		if(UART_recieveByte()==Matched){ /*recieve after check in control ecu
			if the two password are matched make success=1
			to break from the loop   */
			succes=1;
		}

	}while(succes==0);

	while(1){ //enter the system
		LCD_clearScreen();
		LCD_moveCursor(0,0);
		LCD_displayString("+ : Open Door");
		LCD_moveCursor(1,0);
		LCD_displayString("- : Change pass");
		ans = KEYPAD_getPressedKey(); //store the input to ans var
		_delay_ms(500); /*delay half sec after each press in the keypad
		   to avoid any repetitions of the inputs */

		UART_sendByte(ans); //send the ans var to the control ecu
		//}
		//else if(ans=='-'){
		//UART_sendByte('y');
		//}
		if (ans =='+'){ //choose open door
			enter_pass(); // ask to enter the current password
			//UART_sendByte(MC2_READY);

			if(UART_recieveByte()==Matched){ //if the password is matching the old one
				//the control ecu send matched to the HMI_ecu
				tick=0; //start the timer from the beginning
				LCD_clearScreen();
				LCD_moveCursor(0,0);
				LCD_displayString("Door is Unlocking"); // display this statement
				while(tick !=5); // polling for 15 sec
				LCD_clearScreen();
				LCD_displayString("  WAIT !");// display this statement
				while(tick !=6); // polling for 3 sec
				LCD_clearScreen();

				LCD_displayString("Door is locking"); // display this statement
				while(tick !=11);// polling for 3 sec

			}
			else {// secound try
				// THE SAME COMMENTS AS FIRST TRY
				enter_pass();
				//UART_sendByte(MC2_READY);
				if(UART_recieveByte()==Matched){
					tick=0;
									LCD_clearScreen();
									LCD_moveCursor(0,0);
									LCD_displayString("Door is Unlocking");
									while(tick !=5);
									LCD_clearScreen();
									LCD_displayString("  WAIT !");
									while(tick !=6);
									LCD_clearScreen();

									LCD_displayString("Door is locking");
									while(tick !=11);

				}
				else{ //third try
					//THE SAME COMMENTS AS FIRST AND SEC TRY
					enter_pass();
					//UART_sendByte(MC2_READY);
					if(UART_recieveByte()==Matched){
						tick=0;
										LCD_clearScreen();
										LCD_moveCursor(0,0);
										LCD_displayString("Door is Unlocking");
										while(tick !=5);
										LCD_clearScreen();
										LCD_displayString("  WAIT !");
										while(tick !=6);
										LCD_clearScreen();

										LCD_displayString("Door is locking");
										while(tick !=11);


					}
					else { // if the user enter the pssword 3 times wrong

						//while(UART_recieveByte() != MC2_READY){}
						//	UART_sendString("third try !#");
						tick=0;
						LCD_clearScreen();
						LCD_moveCursor(0,0);
						LCD_displayString("WRONG PASS !");
						while(tick!=20);
						LCD_clearScreen();
					}
				}
			}
		}
		else if (ans == '-'){

			enter_pass();
			//UART_sendByte(MC2_READY);
			if(UART_recieveByte()==Matched){
				do { /* looping until the user enter
					       two matched password to be
					       the current password of the system */
						choose_pass(); //calling choose pass function
						//UART_sendByte(MC2_READY);
						if(UART_recieveByte()==Matched){ /*recieve after check in control ecu
							if the two password are matched make success=1
							to break from the loop   */
							succes=1;
						}

					}while(succes==0);
			}
			else { // secound try
				enter_pass();
				//UART_sendByte(MC2_READY);
				if(UART_recieveByte()==Matched){
					do { /* looping until the user enter
						       two matched password to be
						       the current password of the system */
							choose_pass(); //calling choose pass function
							//UART_sendByte(MC2_READY);
							if(UART_recieveByte()==Matched){ /*recieve after check in control ecu
								if the two password are matched make success=1
								to break from the loop   */
								succes=1;
							}

						}while(succes==0);

				}
				else{ //third try
					enter_pass();
					//UART_sendByte(MC2_READY);
					if(UART_recieveByte()==Matched){
						do { /* looping until the user enter
							       two matched password to be
							       the current password of the system */
								choose_pass(); //calling choose pass function
								//UART_sendByte(MC2_READY);
								if(UART_recieveByte()==Matched){ /*recieve after check in control ecu
									if the two password are matched make success=1
									to break from the loop   */
									succes=1;
								}

							}while(succes==0);

					}
					else {

						//while(UART_recieveByte() != MC2_READY){}
						//UART_sendString("third try !#");
						tick=0;
						LCD_clearScreen();
						LCD_moveCursor(0,0);
						LCD_displayString("WRONG PASS !"); // display this statement
						while(tick!=20); //polling 60 sec
						LCD_clearScreen();

					}

				}
			}

		}

	}
	return 0;

}
