/*
 * Jacques Fracchia
 * CS120b Project
 * UCR
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <bit.h>
#include <timer.h>
#include <scheduler.h>
#include <stdio.h>
#include "io.c"
#include <avr/eeprom.h>
#include <string.h>
#include <levelOne.h>
#include <levelFunctions.h>


	
char* welcome = "Welcome to:";
char* name = " Spelling Game!";
char* pressStart = "Press to Start";
char* bluebutton = "Next: Blue            Button";
char* yellowbutton = "Accept: Yellow          Button";
char* displayLvlOne = "Level One...";
unsigned char position = 0x00;
unsigned char curs = 0x00;

unsigned char nameTimer = 0x00;
unsigned char welcomeTimer = 0x00;
unsigned char customTimer = 0x00;
unsigned char customCount = 0x00;
unsigned char buttonTimer = 0x00;
unsigned char blueTimer = 0x00;
unsigned char yellowTimer = 0x00;
unsigned char displayLvlOneTimer = 0x00;
unsigned char introPress = 0x00;




enum SM1_States { start_welcome, start_custom, start_name, start_press, start_debounce, start_blue, start_yellow, start_level, start_buffer, start_done };
	
int StartGame(int state) {

	// Local Variables
	introPress = PINA & 0x30;

	//State machine transitions
	switch (state) {
		
		case start_welcome:
			if(welcomeTimer < 20){
				state = start_welcome;
			}
			else{
				LCD_ClearScreen();
				state = start_custom;
			}
			break;
			
		case start_custom:
			if(customTimer < 32){
				state = start_custom;
			}
			else{
				LCD_ClearScreen();
				state = start_name;
			}
			break;

		case start_name:
			if(nameTimer < 50){
				state = start_name;
			}
			else{
				LCD_ClearScreen();
				state = start_press;
			}
			break;

		case start_press:
			if(introPress){
				state = start_debounce;
			}
			else{
				state = start_press;
			}
			break;
		
		case start_debounce:
			if(introPress){
				state = start_debounce;
			}
			else{
				state = start_blue;
			}

		case start_blue:
			if(blueTimer < 30){
				state = start_blue;
			}
			else{
				LCD_ClearScreen();
				state = start_yellow;
			}
			break;
		
		case start_yellow:
			if(yellowTimer < 30){
				state = start_yellow;
			}
			else{
				LCD_ClearScreen();
				state = start_level;
			}
			break;
		
		case start_level:
			if(displayLvlOneTimer < 30){
				state = start_level;
			}
			else{
				state = start_buffer;
			}
			break;
			
		case start_buffer:
			state = start_done;
			break;
		
		case start_done:
			if(continueGame == 0x01){
				state = start_done;
			}
			else{
				nameTimer = 0x00;
				welcomeTimer = 0x00;
				buttonTimer = 0x00;
				blueTimer = 0x00;
				yellowTimer = 0x00;
				displayLvlOneTimer = 0x00;
				state = start_welcome;
			}
			break;

		default:
			state = start_welcome; // default: Initial state
			break;
	}

	//State machine actions
	switch (state) {
		
		case start_welcome:
			LCD_DisplayString(1, welcome);
			welcomeTimer++;
			break;
			
		case start_custom:
			createCustom(position, curs);
			position = (position + 1) % 12;
			curs = (curs + 1) % 32;
			customTimer++;
			
			break;

		case start_name:
			LCD_DisplayString(1, name);
			nameTimer++;
			break;

		case start_press:
			LCD_DisplayString(1, pressStart);
			buttonTimer++;
			break;
		
		case start_debounce:
			LCD_ClearScreen();
			break;

		case start_blue:
			LCD_DisplayString(1, bluebutton);
			blueTimer++;
			break;
		
		case start_yellow:
			LCD_DisplayString(1, yellowbutton);
			yellowTimer++;
			break;
		
		case start_level:
			LCD_DisplayString(1, displayLvlOne);
			displayLvlOneTimer++;
			break;
			
		case start_buffer:
			continueGame = 0x01;
			currentLevel = 0x00;
			LCD_ClearScreen();
			break;
		
		case start_done:
			break;

		default:
			state = start_welcome; // default: Initial state
			break;
	}

	return state;
}

 

// Implement scheduler code from PES.
int main(){
	// Set Data Direction Registers
	// Buttons PORTA[0-7], set AVR PORTA to pull down logic
	DDRA = 0x0C; PORTA = 0xFF;
	DDRD = 0xFF; PORTD = 0x00;
	DDRB = 0xFF; PORTB = 0x00; //PORTA as output
	DDRC = 0xFF; PORTC = 0x00; //PORTC as output
	// . . . etc

	// Period for the tasks
	unsigned long int SMTick1_calc = 150;
	unsigned long int SMTick2_calc = 75;
	//unsigned long int SMTick3_calc = 100;
	//Calculating GCD
	unsigned long int tmpGCD = 1;
	tmpGCD = findGCD(SMTick1_calc, SMTick2_calc);
	//tmpGCD = findGCD(tmpGCD, SMTick3_calc);

	//Greatest common divisor for all tasks or smallest time unit for tasks.
	unsigned long int GCD = tmpGCD;

	//Recalculate GCD periods for scheduler
	unsigned long int SMTick1_period = SMTick1_calc/GCD;
	unsigned long int SMTick2_period = SMTick2_calc/GCD;
	//unsigned long int SMTick3_period = SMTick3_calc/GCD;
	//unsigned long int SMTick2_period = SMTick2_calc/GCD;

	//Declare an array of tasks 
	static task task1, task2;
	task *tasks[] = { &task1, &task2};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	// Task 1
	task1.state = 0;//Task initial state.
	task1.period = SMTick1_period;//Task Period.
	task1.elapsedTime = SMTick1_period;//Task current elapsed time.
	task1.TickFct = &StartGame;//Function pointer for the tick.


	task2.state = 0;//Task initial state.
	task2.period = SMTick2_period;//Task Period.
	task2.elapsedTime = SMTick2_period;//Task current elapsed time.
	task2.TickFct = &levelOneLED;//Function pointer for the tick.
	
	
/*	task3.state = 0;//Task initial state.
	task3.period = SMTick3_period;//Task Period.
	task3.elapsedTime = SMTick3_period;//Task current elapsed time.
	task3.TickFct = &levelTwo;//Function pointer for the tick.
*/	
	
	

	// Set the timer and turn it on
	TimerSet(GCD);
	TimerOn();
	LCD_init();
	ADC_init();

	unsigned short i; // Scheduler for-loop iterator
	while(1) {
		// Scheduler code
		for ( i = 0; i < numTasks; i++ ) {
			// Task is ready to tick
			if ( tasks[i]->elapsedTime == tasks[i]->period ) {
				// Setting next state for task
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				// Reset the elapsed time for next tick.
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}

	// Error: Program should not exit!
	return 0;
}

/*
0,0,0b01111111,0b11111111,0b11001100,0b11001100,0b11001100,0b11111111,0b01111111, //A
0,0,0b00111100,0b01111110,0b11011011,0b11011011,0b11011011,0b11111111,0b11111111 //B
0,0,0b11000011,0b11000011,0b11000011,0b11000011,0b11100111,0b01111110,0b00111100, //C
0,0,0b01111110,0b10111101,0b11000011,0b11000011,0b11000011,0b11111111,0b11111111, //D
0,0,0b11011011,0b11011011,0b11011011,0b11011011,0b11011011,0b11111111,0b11111111, //E
0,0,0b11011000,0b11011000,0b11011000,0b11011000,0b11011000,0b11111111,0b11111111, //F
0,0,0b00011111,0b11011111,0b11011000,0b11011011,0b11011011,0b11011011,0b11111111,0b11111111, //G
0,0,0b11111111,0b11111111,0b00011000,0b00011000,0b00011000,0b11111111,0b11111111, //H
0,0,0b11000011,0b11000011,0b11000011,0b11111111,0b11111111,0b11000011,0b11000011,0b11000011, //I
0,0,0b11000000,0b11000000,0b11000000,0b11111111,0b11111111,0b11000011,0b11001111,0b11001111, //J
0,0,0b11000011,0b11100111,0b01111110,0b00111100,0b00011000,0b11111111,0b11111111, //K
0,0,0b00000011,0b00000011,0b00000011,0b00000011,0b00000011,0b00000011,0b11111111,0b11111111, //L
0,0,0b11111111,0b11111111,0b01100000,0b01110000,0b01110000,0b01100000,0b11111111,0b11111111, //M
0,0,0b11111111,0b11111111,0b00011100,0b00111000,0b01110000,0b11100000,0b11111111,0b11111111, //N
0,0,0b01111110,0b11111111,0b11000011,0b11000011,0b11000011,0b11000011,0b11111111,0b01111110, //O
0,0,0b01110000,0b11111000,0b11001100,0b11001100,0b11001100,0b11111111,0b11111111, //P
0,0,0b01111110,0b11111111,0b11001111,0b11011111,0b11011011,0b11000011,0b11111111,0b01111110, //Q
0,0,0b01111001,0b11111011,0b11011111,0b11011110,0b11011100,0b11011000,0b11111111,0b11111111, //R
0,0,0b11001110,0b11011111,0b11011011,0b11011011,0b11011011,0b11011011,0b11111011,0b01110011, //S
0,0,0b11000000,0b11000000,0b11000000,0b11111111,0b11111111,0b11000000,0b11000000,0b11000000, //T
0,0,0b11111110,0b11111111,0b00000011,0b00000011,0b00000011,0b00000011,0b11111111,0b11111110, //U
0,0,0b11100000,0b11111100,0b00011110,0b00000011,0b00000011,0b00011110,0b11111100,0b11100000, //V
0,0,0b11111110,0b11111111,0b00000011,0b11111111,0b11111111,0b00000011,0b11111111,0b11111110, //W
0,0,0b01000010,0b11100111,0b01111110,0b00111100,0b00111100,0b01111110,0b11100111,0b01000010, //X
0,0,0b01000000,0b11100000,0b01110000,0b00111111,0b00111111,0b01110000,0b11100000,0b01000000, //Y
0,0,0b11000011,0b11100011,0b11110011,0b11111011,0b11011111,0b11001111,0b11000111,0b11000011,  //Z
	*/