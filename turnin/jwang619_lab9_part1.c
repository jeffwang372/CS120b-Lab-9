/* Author: Jeffrey Wang jwang619@ucr.edu
 * Lab Section: 22
 * Assignment: Lab # 9  Exercise # 1
 * Exercise Description: [optional - include for your own benefit]
 *
 * I acknowledge all content contained herein, excluding template or example
 * code is my own original work.
 *
 *  Demo Link: https://drive.google.com/file/d/1sqtcPGRx7ZUg8EzRma1Py6zen3K-A7b-/view?usp=sharing
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn() {
	TCCR1B = 0x0B;

	OCR1A = 125;

	TIMSK1 = 0x02;

	TCNT1 = 0;

	_avr_timer_cntcurr = _avr_timer_M;

	SREG |= 0x80;
}

void TimerOff() {
	TCCR1B = 0x00;
}

void TimerISR() {
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect) {
	_avr_timer_cntcurr--;
	if(_avr_timer_cntcurr == 0) {
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

unsigned char threeLEDs = 0x00;
unsigned char blinkingLED = 0x00;

enum threeStates {SMStart, Bit0, Bit1, Bit2} threeState;
enum blinkStates {SMStart2, On, Off} blinkState;


void ThreeLEDsSM() {
       switch(threeState) {
	       case SMStart:
			threeState = Bit0;
	 		break;
		case Bit0:
			threeState = Bit1;
			break;
		case Bit1:
			threeState = Bit2;
			break;
		case Bit2:
			threeState = Bit0;
			break;
		default:
			threeState = SMStart;
			break;
       }
	switch(threeState) {
               case SMStart:
                        break;
                case Bit0:
                        threeLEDs = 0x01;
                        break;
                case Bit1:
                        threeLEDs = 0x02;
                        break;
                case Bit2:
                        threeLEDs = 0x04;
                        break;
                default:
                        break;
       }
}

void BlinkingLEDSM() {
       switch(blinkState) {
               case SMStart2:
                        blinkState = On;
                        break;
                case On:
                        blinkState = Off;
                        break;
                case Off:
                        blinkState = On;
                        break;
                default:
                        blinkState = SMStart2;
                        break;
       }
        switch(blinkState) {
               case SMStart2:
                        break;
                case On:
                        blinkingLED = 0x08;
                        break;
                case Off:
                        blinkingLED = 0x00;
                        break;
                default:
                        break;
       }

}

void CombineLEDsSM() {
	unsigned char outputB = 0x00;
	
	outputB = PORTB;

	outputB = threeLEDs | blinkingLED;

	PORTB = outputB;
}

int main(void) {
    /* Insert DDR and PORT initializations */
	unsigned long Timer1 = 1000;
        unsigned long Timer2 = 1000;
	const unsigned long timerPeriod = 1;

	//DDRA = 0x00; PORTA = 0xFF; // Configure port A's 8 pins as inputs
        DDRB = 0xFF; PORTB = 0x00;
	TimerSet(timerPeriod);
	TimerOn();

	threeState = SMStart;
	blinkState = SMStart2;
    /* Insert your solution below */
    while (1) {
	if (Timer1 >= 1000) {
		ThreeLEDsSM();
		Timer1 = 0;
	}
	if(Timer2 >= 1000) {
		BlinkingLEDSM();
		Timer2 = 0;
	}
	CombineLEDsSM();
	while(!TimerFlag);
	TimerFlag = 0;
	Timer1 += timerPeriod;
	Timer2 += timerPeriod;
    }
    return 0;
}
