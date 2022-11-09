/*
 * control_main.c
 *
 *  Created on: Nov 2, 2022
 *      Author: kMermah
 */
#include "buzzer.h"
#include <util/delay.h> /* For the delay functions */
#include "common_macros.h" /* For GET_BIT Macro */
#include "gpio.h"
#include <avr/io.h>
#include "uart.h"
#include "std_types.h"
#include "external_eeprom.h"
#include "PWM.h"
#include "DC_motor.h"
#include "twi.h"
#include "TIMER.h"
#include"lcd.h"
#define MC2_READY 0x10 //to send if the reciever is ready
#define Matched 0x11//to send if the two password are matched
#define NotMatched 0xFF //to send if the two password are not matched
#define contin 0x0D // to tell the control_ECU that the user press enter
//UART_sendByte(MC2_READY);
Timer1_ConfigType timer_config = {0,23438,F_CPU_1024,COMPARE};
TWI_ConfigType CONFIG = {400000,0x01};
/*
 twi config
 bit rate = 400000
 address = 0x01
\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
 timer configration struct with
 1- 0 initial value
 2- 23438 OCR value which means 3 sec
 3- prescaler 1024
 4- compare mode
 */
unsigned char pass_1[5],pass_2[5],eeprom[5],tick=0,succes=0,ans,k=0;
/*
 pass_1 array to store first password after recieve it by uart
 pass_2 array to store sec password after recieve it by uart
 ans for store + or - in the system
 tick the counter of the timer "increace every 3 sec"
 success to now if the passwords are matched --> success =1 in this case
 k to count the number of wrong tries
 eeprom array to read from eeprom before check
 */
// timer call back function
void timer_count (){
	tick++;

}
/*check current pass function
 recieve password and compare it with the one in eeprom
 */
void check_current_pass(char *current_pass1){
	LCD_clearScreen();
	char i=0,j=0;
	succes=0;
	for (i=0 ; i<5 ; i++){ // loop to recieve 5 bytes from HMI_ECU
		//UART_sendByte(MC2_READY);
		current_pass1[i]=UART_recieveByte(); //recieve in current pass array
		//LCD_intgerToString(current_pass1[i]); //display the recieved bytes
	}
	//LCD_clearScreen();
	//for (i=0 ; i<5 ; i++){// loop to read the password from eeprom
	//and store it in eeprom array

		//EEPROM_readByte(0x0311+i, eeprom[i]);

		//_delay_ms(100);
//LCD_intgerToString(eeprom[i]);
	//}
	for(i=0 ; i<5 ; i++){ //loop to check each byte if equal or not
		if( current_pass1[i]!=eeprom[i]){

			j=1; // j=1 if wrong password
		}
	}
	if(j==0){ // j=1 if right password
		succes=1; //to break from the do..while loop
		UART_sendByte(Matched); //send that the password is right to HMI
		//LCD_clearScreen();
	//	LCD_moveCursor(0, 0);
		//LCD_displayString("Matched"); //display this statement in controller lcd
		//_delay_ms(1000);
		//LCD_clearScreen();
	}
	else if(j==1){
		//while(UART_recieveByte() != MC2_READY){}

		UART_sendByte(NotMatched); //send that the password isn't right to HMI
		//LCD_clearScreen();
		//LCD_moveCursor(0, 0);
		//LCD_displayString("Not Matched");  //display this statement in controller lcd
		//_delay_ms(1000);
		//LCD_clearScreen();
	}
	while(UART_recieveByte()!= contin );
	return ;
}
/*
 check new password function
 recieve two password and check if matched and send the result to HMI
 */
void check_new_pass(char *pass1 , char *pass2){
	char i=0,j=0;
	succes=0;
	for (i=0 ; i<5 ; i++){ //loop to recieve the first password byte by byte

		pass1[i]=UART_recieveByte(); //store in pass1 array
		//LCD_displayCharacter('x');
		//LCD_intgerToString(pass1[i]);//display the first password in controller lcd

	}

	for (i=0 ; i<5 ; i++){ //loop to recieve the sec password byte by byte

		LCD_moveCursor(1,i);
		pass2[i]=UART_recieveByte(); //store in pass2 array
		//LCD_intgerToString(pass2[i]);//display the sec password in controller lcd

	}
	for(i=0 ; i<5 ; i++){ //loop to check the two passwords byte by byte
		if( pass1[i]!=pass2[i]){
			j=1; // j=1 if not matched
		}
	}
	if(j==0){ //if matched
		//while(UART_recieveByte() != MC2_READY){}
		succes=1;
		UART_sendByte(Matched); //send that the two passwords are matched to HMI
		//UART_sendString("Matched!#");
		LCD_clearScreen();
		LCD_moveCursor(0, 0);
		//LCD_displayString("Matched"); //display matched in controller lcd
		_delay_ms(1000);
//LCD_clearScreen();
//LCD_intgerToString(5);
		for (i=0 ; i<5 ; i++){
			EEPROM_writeByte(0x0311+i ,pass2[i]); //store the new pass in eeprom
			eeprom[i]=pass2[i];
			//LCD_intgerToString(pass2[i]);
			_delay_ms(100);
		}
	}
	else if(j==1){
		UART_sendByte(NotMatched);//send that the two passwords are not matched to HMI
		LCD_clearScreen();
		LCD_moveCursor(0, 0);
		//LCD_displayString("NOT Matched"); //display not matched in controller lcd
		//_delay_ms(1000);

	}
	while(UART_recieveByte()!= contin ); //polling until user press enter
	LCD_clearScreen();
	return ;
}
int main(){
	SREG |= (1<<7); //enable I bit
	TWI_init(&CONFIG); //initialization of the twi
	LCD_init();//initialization of the lcd
	/*
	 UART config struct
	 1- send 8 bits data
	 2- using even parity error detector
	 3- stop bit is one bit only
	 4- baud rate = 9600
	 */
	UART_ConfigType Config = {eight_bits,even_parity,one_bit,9600};
	UART_init( &Config); //initialization of the UART with UART config struct
	Timer1_init(&timer_config);// initialization of the timer
	Timer1_setCallBack(timer_count);
	//choose timer_count function as call back functin
	Buzzer_init();// initialization of the buzzer
	DcMotor_Init();// initialization of the motor
	//LCD_displayCharacter('x');


	do {
		/* looping until the user enter
			       two matched password to be
			       the current password of the system */

		check_new_pass(pass_1,pass_2);//calling check_new_pass function

	}while(succes==0); //if two pass matched succes=1 and break from the loop

	while(1){
//recieveing the choice of HMI in ans var
		ans=UART_recieveByte();


		if (ans ==11){ //if choose '+'

			do{
				if(k==3){ //check if the wrong trials is 3
					k=0;
					//buzzer
					tick=0;
					Buzzer_on(); // turn on the buzzer
					while(tick!=20);// polling 60 sec
					Buzzer_off(); // turn off the buzzer

					break; //break after the third trial
				}

				check_current_pass(pass_1); //check the Kth trial
				k++; //increace tries
			}while(succes==0); /*break from loop if onr of two conditions happen
			1- three wrong trials
			2- the password is true
			*/
			k=0;
			if(succes==1){
				//the user enter the right password
				//UART_sendByte(Matched);
				tick=0; //start timer from 0
				DcMotor_Rotate (1,100);
				//rotate motor clockwise with max speed
               // _delay_ms(1000);
				while(tick !=5); //polling 15 sec
				DcMotor_Rotate (0,0); //stop the motor
				//_delay_ms(1000);
				while(tick !=6);//polling 3 sec
				DcMotor_Rotate (2,100); //rotate motor anti clockwise with max speed
				//_delay_ms(1000);
				while(tick !=11);//polling 15 sec
				DcMotor_Rotate (0,0);
				//_delay_ms(1000);
			}
		}
		else if (ans == 13){
			do{
				//like the the last one
				if(k==3){

					k=0;
					tick=0;
					Buzzer_on(); // turn on the buzzer
					while(tick!=20); //polling 60 sec
					Buzzer_off(); // turn off the buzzer

					break;
				}

				check_current_pass(pass_1);
				k++;
			}while(succes==0);
			/*break from loop if onr of two conditions happen
						1- three wrong trials
						2- the password is true
						*/
			k=0;
			if(succes==1){
				do {
						/* looping until the user enter
							       two matched password to be
							       the current password of the system */

						check_new_pass(pass_1,pass_2);//calling check_new_pass function

					}while(succes==0);
				//k=0;
			}

		}

		/*check_current_pass(pass_1);
			//buzzer
			if(!(strcmp(Str,"third try !"))){
				Buzzer_on();
				_delay_ms(60000);


			}*/
	}

	return 0;
}

