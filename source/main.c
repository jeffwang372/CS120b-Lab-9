/* Author: Jeffrey Wang jwang619@ucr.edu
 * Lab Section: 22
 * Assignment: Lab # 9  Exercise # 4
 * Exercise Description: [optional - include for your own benefit]
 *
 * I acknowledge all content contained herein, excluding template or example
 * code is my own original work.
 *
 *  Demo Link: https://drive.google.com/file/d/1t0k-sA_xo3lvfqX4FzGBJYPM6p1dFqun/view?usp=sharing
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
unsigned char speakChar = 0x00;
unsigned char speakCount = 0x00;
unsigned char frequency = 0x02;

enum threeStates {SMStart, Bit0, Bit1, Bit2} threeState;
enum blinkStates {SMStart2, On, Off} blinkState;
enum speakStates {SMStart3, Wait, SoundOn, SoundOff} speakState;
enum adjustStates {SMStart4, WaitPress, Inc, IncBuffer, Dec, DecBuffer} adjustState;

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

void speakSM() {
	unsigned char inputA = 0x00;
	inputA = ~PINA;
       switch(speakState) {
               case SMStart3:
                        speakState = Wait;
                        break;
                case Wait:
                        if ((inputA & 0x04) == 0x04) {
				speakState = SoundOn;
			}
                        break;
                case SoundOn:
			if ((inputA & 0x04) != 0x04) {
                                speakState = Wait;
				break;
                        }
                        if(speakCount == frequency) {
				speakState = SoundOff;
				speakCount = 0x00;
				break;
			}
			++speakCount;
                        break;
		case SoundOff:
			if ((inputA & 0x04) != 0x04) {
                                speakState = Wait;
				break;
                        }
                        if(speakCount == frequency) {
                                speakState = SoundOn;
                                speakCount = 0x00;
				break;
                        }
			++speakCount;
                        break;
                default:
                        speakState = SMStart3;
                        break;
       }
        switch(speakState) {
               case SMStart3:
                        break;
                case Wait:
			speakChar = 0x00;
                        break;
                case SoundOn:
                        speakChar = 0x10;
                        break;
		case SoundOff:
			speakChar = 0x00;
			break;
                default:
                        break;
       }

}

void adjustSound() {
	unsigned char inputA = 0x00;
        inputA = ~PINA;
	switch(adjustState) {
               case SMStart4:
		       adjustState = WaitPress;
                        break;
                case WaitPress:
                        if((inputA & 0x01) == 0x01) {
				adjustState = Inc;
			}
			if((inputA & 0x02) == 0x02) {
                                adjustState = Dec;
                        }
                        break;
                case Inc:
                        adjustState = IncBuffer;
                        break;
                case IncBuffer:
                        if((inputA & 0x01) != 0x01) {
                                adjustState = WaitPress;
                        }
                        break;
		case Dec:
			adjustState = DecBuffer;
			break;
		case DecBuffer:
			if((inputA & 0x02) != 0x02) {
                                adjustState = WaitPress;
                        }
			break;
                default:
                        break;
       }
	switch(adjustState) {
               case SMStart4:
                        break;
                case WaitPress:
                        break;
                case Inc:
			--frequency;
                        break;
                case IncBuffer:
                        break;
                case Dec:
			++frequency;
                        break;
                case DecBuffer:
                        break;
                default:
                        break;
       }


}

void CombineLEDsSM() {
	unsigned char outputB = 0x00;
	
	outputB = PORTB;

	outputB = (threeLEDs | blinkingLED) | speakChar;

	PORTB = outputB;
}

int main(void) {
    /* Insert DDR and PORT initializations */
	unsigned long Timer1 = 300;
        unsigned long Timer2 = 1000;
	const unsigned long timerPeriod = 1;

	DDRA = 0x00; PORTA = 0xFF; // Configure port A's 8 pins as inputs
        DDRB = 0xFF; PORTB = 0x00;
	TimerSet(timerPeriod);
	TimerOn();

	threeState = SMStart;
	blinkState = SMStart2;
	speakState = SMStart3;
	adjustState = SMStart4;
    /* Insert your solution below */
    while (1) {
	if (Timer1 >= 300) {
		ThreeLEDsSM();
		Timer1 = 0;
	}
	if(Timer2 >= 1000) {
		BlinkingLEDSM();
		Timer2 = 0;
	}
	speakSM();
	adjustSound();
	CombineLEDsSM();
	while(!TimerFlag);
	TimerFlag = 0;
	Timer1 += timerPeriod;
	Timer2 += timerPeriod;
    }
    return 0;
}
