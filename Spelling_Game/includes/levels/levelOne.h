#ifndef LEVELONE_H
#define LEVELONE_H

#define F_CPU 11059200UL //defining crystal frequency
#include <util/delay.h>  //delay header
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <avr/eeprom.h>

#define HIGH_SCORE_ADDR 0

unsigned char continueGame;
unsigned char i = 0x00;
unsigned char j = 0x00;
unsigned char joystick = 0x00;
unsigned char key[];
unsigned char key_LED[];
unsigned char levelOne[20];
unsigned char next = 0x00;
unsigned char accept = 0x00;
unsigned char key_length = 0x00;
unsigned char temp;
int HORIZONTAL=141; //neutral value on x-axis
int HORIZONTALMOV =0;
int specialKey;

unsigned char success = 0x00;
const unsigned char* failed = "GAME OVER";
const unsigned char* successful = "LEVEL COMPLETE";
const unsigned char* nextLvlStr = "NEXT LEVEL...";
int levelSpeed = 15;
unsigned char successTimer = 0x00;
unsigned char failTimer = 0x00;
unsigned char one_nextLvlTimer = 0x00;
unsigned char currentLevel = 0;
unsigned char high_score = 0;
unsigned char displayCurr = 0x00;
unsigned char nextLevel = 0x00;

unsigned char alphabet[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '\0' };
unsigned char levels[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '10', '11', '12', '13', '14', '15', '16', '17', '18', '19', '20', '\0' };
unsigned char word1[] = {'O', 'C', 'C', 'L', 'U', 'D', 'E', '\0'};
unsigned char word2[] = {'Q', 'U', 'I', 'B', 'B', 'L', 'E', '\0'};
unsigned char word3[] = {'A', 'R', 'R', 'A', 'N', 'T', '\0'};
unsigned char word4[] = {'F', 'O', 'M', 'E', 'N', 'T', '\0'};
unsigned char word5[] = {'R', 'E', 'C', 'R', 'E', 'A', 'N', 'C', 'Y', '\0'};

unsigned char compare(unsigned char* user, unsigned char* game){
	unsigned char pass = 0x01;
	key_length = strlen(game);
	for(int x=0; x<key_length; x++){
		if(user[x] != game[x]){
			pass = 0x00;
		}
	}
	return pass;
}


enum SM2_States { one_wait, one_getKey, one_displayLED, one_displayLCD, check_string, one_debounce, one_enter, one_failed, one_success, display_CL, next_level, one_reset, one_scroll_up, one_scroll_down };

int levelOneLED(int state) {
	next = PINA & 0x10;
	accept = PINA & 0x20;
	
	switch (state) {
		
		case one_wait:			
			if (continueGame){    // Wait for button press
				state = one_getKey;
			}
			else{
				state = one_wait;
			}
			break;
		
		case one_getKey:
			state = one_displayLED;
			break;
			

		case one_displayLED:
			state = one_displayLCD;
			break;
			
		case one_displayLCD:
			joystick = getJoystick();
			
			if(next){
				state = one_debounce;
			}
			
			else if(joystick>(HORIZONTAL+50)){
				if(i>=0 && i<26){
					state = one_scroll_up;
					
				}
				else{
					state = one_displayLCD;
				}
			}
			
			else if(joystick<(HORIZONTAL-100)){
				if(i>=0 && i<26){
					state = one_scroll_down;
				}
				else{
					state = one_displayLCD;
				}
			}
			
			else if(accept){
				temp = alphabet[i];
				append(levelOne, temp);
				state = check_string;
			}
			
			else{	
				state = one_displayLCD;
			}	
			break;
		
		case one_debounce:
			if(next){
				state = one_debounce;
			}
			else{
				state = one_enter;
			}
			break;
		
		case one_enter:
			state = one_displayLCD;
			break;
			
		case check_string:
			if(specialKey == 0){
				success = compare(levelOne, word1);
			}
			else if(specialKey == 1){
				success = compare(levelOne, word2);
			}
			else if(specialKey == 2){
				success = compare(levelOne, word3);
			}
			else if(specialKey == 3){
				success = compare(levelOne, word4);
			}
			else {
				success = compare(levelOne, word5);
			}
			
			if(success){
				state = one_success;
			}
			else{
				state = one_failed;
			}
			break;
		
		case one_scroll_down:
			state = one_displayLCD;
			break;
		
		case one_scroll_up:
			state = one_displayLCD;
			break;
		
		case one_failed:
			if(failTimer < 50){
				state = one_failed;
			}
			else{
				LCD_ClearScreen();
				state = display_CL;
			}
			break;

		case one_success:
			if(successTimer < 50){
				state = one_success;
			}
			else{
				LCD_ClearScreen();
				currentLevel = currentLevel + 1;
				state = display_CL;	
			}
			break;
		
		case display_CL:
			if(displayCurr < 50){
				state = display_CL;
			}
			else{
				state = next_level;
			}
			break;
		
		case next_level:
			if(nextLevel < 30){
				state = next_level;
			}
			else{
				state = one_reset;
			}
			break;
			
		case one_reset:
			if(success){
				continueGame = 0x01;
				state = one_wait;
			}
			else{
				continueGame = 0x00;
				state = one_wait;
			}
			break;

		default:
			state = one_wait; // default: Initial state
			break;
	}

	//State machine actions
	switch (state) {
		case one_wait:
			
			break;
		
		case one_getKey:
			specialKey = getRandomKey();
			break;

		case one_displayLED:
			LCD_ClearScreen();
			levelOneLEDisplay();
			break;

		case one_displayLCD:
			LCD_Cursor(j+1);
			LCD_WriteData(alphabet[i]);
			break;
			
		case one_debounce:
			break;
			
		case one_enter:
			temp = alphabet[i];
			append(levelOne, temp);
			j++;
			break;
		
		case one_scroll_down:
			if(i>0){
				i--;
			}
			break;
		
		case one_scroll_up:
			if(i<25){
				i++;
			}
			break;
			
		case check_string:
			LCD_ClearScreen();
			break;
			
		case one_failed:
			LCD_DisplayString(1, failed);
			failTimer++;
			success = 0x00;
			break;
	
		case one_success:
			LCD_DisplayString(1, successful);
			successTimer++;
			continueGame = 0x01;
			success = 0x01;
			break;
			
		case display_CL:
			if(success){
				LCD_DisplayString(1, "Next Level: ");
				LCD_Cursor(13);
				LCD_WriteData(levels[currentLevel]);
			}
			else{
				LCD_DisplayString(1, "Last Level: ");
				LCD_Cursor(13);
				LCD_WriteData(levels[currentLevel]);
			}
			if(high_score < currentLevel){
				eeprom_write_byte((uint8_t*) 16, (uint8_t)currentLevel);
			}
			displayCurr++;
			break;
		
		case next_level:
			Cleanup: ;
			char display[32];
			unsigned char high_score = eeprom_read_byte((uint8_t*)16);
			char high_score_update[4];
			sprintf(high_score_update, "%d", high_score);
			strcpy(display, "High Score: ");
			strcat(display, high_score_update);
			LCD_DisplayString(1, display);
			nextLevel++;
			break;
			
		case one_reset:
			i = 0x00;
			j = 0x00;
			LCD_ClearScreen();
			LCD_Cursor(0);		
			levelOne[0] = 0;
			key[0] = 0;
			key_LED[0] = 0;
			successTimer = 0x00;
			failTimer = 0x00;
			nextLevel = 0x00;
			levelSpeed--;
			displayCurr = 0x00;
			break;
		
		default:
			state = one_wait; // default: Initial state
			break;
	}

	return state;
}

#endif