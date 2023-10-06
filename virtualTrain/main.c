/**
 * Name:            Hunter Burnett
 * Date:            7/6/2023
 * Assignment:      Lab 3 - Virtual Train
 * YouTube:         https://www.youtube.com/watch?v=RdtSBL2sERY&ab_channel=HunterBurnett
 *
 * This program:
 * This program is meant to be run on the MSP-EXP430FR2355 with the four-digit seven segment display connected.
 * When the program is uploaded to the board the seven segment will display three lines that are initially stationary.
 * If either SW1 or SW2 is pressed on the board the line will travel along the perimeter of the display, clockwise (forward) or counter-clockwise (backward) respectively.
 * If in either the backward or forward state SW1 can be pressed again to stop the "train" in the position it is currently in.
 * If in a movement state SW2 can change the direction of the train to go backward or forward.
 * if the "train" is in a movement state (backward or forward state) then the green LED will be activated. If the train is stopped only the red LED is activated.
 *
 */

#include <msp430.h> 
#include "Four_Digit_Display.h"
#include <stdint.h>
#include <stdbool.h>

#define MCLK_FREQ_HZ    1000000

bool isSW1Pressed = false;
bool isSW2Pressed = false;

typedef enum{ STOP, FORWARD, BACKWARD, MAX_STATES } state_t;

typedef struct {
    bool isSW1Pressed;
    bool isSW2Pressed;
    uint16_t seconds;
} inputs_t;

void setRedLed(bool enable);
void setGreenLed(bool enable);

void position(uint16_t seconds);
void delay_s(uint16_t seconds);

state_t runStopState(inputs_t inputs);
state_t runForwardState(inputs_t inputs);
state_t runBackwardState(inputs_t inputs);

state_t (*state_table[MAX_STATES])(inputs_t) = {runStopState, runForwardState, runBackwardState};

void init();

int main(void)
{
    volatile inputs_t inputs;
    inputs.isSW1Pressed = false;
    inputs.isSW2Pressed = false;
    inputs.seconds = 0;

    state_t currentState = STOP;
    state_t nextState = currentState;

    init();

    while(1){

        //Accesses the state table and to run the correct state
        if(currentState < MAX_STATES){
            nextState = state_table[currentState](inputs);
        }

        switch(currentState){
        case STOP:
            //Update inputs if in the stop state
            delay_s(1);
            inputs.seconds = inputs.seconds;

            inputs.isSW1Pressed = isSW1Pressed;
            inputs.isSW2Pressed = isSW2Pressed;
            break;

        case FORWARD:
            //Update input if in the forward state
            delay_s(1);

            if(inputs.seconds == 11) inputs.seconds = 0;

            else inputs.seconds++;

            inputs.isSW1Pressed = isSW1Pressed;
            inputs.isSW2Pressed = isSW2Pressed;
            break;

        case BACKWARD:
            //Update inputs if in the backward state
            delay_s(1);
            if(inputs.seconds == 0) inputs.seconds = 11;

            else inputs.seconds--;

            inputs.isSW1Pressed = isSW1Pressed;
            inputs.isSW2Pressed = isSW2Pressed;
            break;
        }
        //Resets and clears any previous inputs
        four_digit_clear();
        isSW1Pressed = false;
        isSW2Pressed = false;

        //Update current state
        currentState = nextState;
    }
}

void init(){
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5; //Disable the GPIO power-on default high-impedance mode

    four_digit_init();// Initializes the hardware for the SSD (Seven Segment Display)

    //SW1 Enabled
    P4DIR &= ~BIT1; // set pin 1 as input
    P4REN |= BIT1;  // enable pin 1 resistor
    P4OUT |= BIT1;  // enable pin 1 pull-up resistor

    P4IE |= BIT1;   // enables interrupt
    P4IES |= BIT1;  // high-to-low trigger condition
    P4IFG &= ~BIT1;

    //SW2 Enabled
    P2DIR &= ~BIT3; // set pin 3 as input
    P2REN |= BIT3;  // enable pin 3 resistor
    P2OUT |= BIT3;  // enable pin 3 pull-up resistor

    P2IE |= BIT3;   // enables interrupt
    P2IES |= BIT3;  // high-to-low trigger condition
    P2IFG &= ~BIT3;

    //Enable Red LED
    P1DIR |= BIT0;
    //Enable Green LED
    P6DIR |= BIT6;

    __enable_interrupt();
}

void delay_s(uint16_t seconds){
    volatile uint16_t i;
    for(i = 0; i < seconds; i++){
        __delay_cycles(100000);
    }
}

state_t runStopState(inputs_t inputs){
    state_t nextState;

    delay_s(1);

    setGreenLed(false);
    setRedLed(true);

    position(inputs.seconds);

    if(inputs.isSW1Pressed){
        nextState = FORWARD;
    }
    else if(inputs.isSW2Pressed){
        nextState = BACKWARD;
    } else{
        nextState = STOP;
    }
    return nextState;
}

state_t runForwardState(inputs_t inputs){
    state_t nextState;

    delay_s(1);

    setGreenLed(true);
    setRedLed(false);

    position(inputs.seconds);

    if(inputs.isSW1Pressed){
        nextState = STOP;
    }
    else if(inputs.isSW2Pressed){
        nextState = BACKWARD;
    } else{
        nextState = FORWARD;
    }
    return nextState;
}

state_t runBackwardState(inputs_t inputs){
    state_t nextState;

    delay_s(1);

    setGreenLed(true);
    setRedLed(false);

    position(inputs.seconds);

    if(inputs.isSW1Pressed){
        nextState = STOP;
    }
    else if(inputs.isSW2Pressed){
       nextState = FORWARD;
    } else{
        nextState = BACKWARD;
    }
    return nextState;
}

void setRedLed(bool enable){
    if(enable){
        P1OUT |= BIT0;
    } else{
        P1OUT &= ~BIT0;
    }
}

void setGreenLed(bool enable){
    if(enable){
           P6OUT |= BIT6;
    } else{
           P6OUT &= ~BIT6;
      }
}

void position(uint16_t seconds) {

    switch (seconds) {
        case 0:
            display_segment(POS_2, SEG_D);
            display_segment(POS_3, SEG_D);
            display_segment(POS_4, SEG_D);
            break;
        case 1:
            display_segment(POS_1, SEG_D);
            display_segment(POS_2, SEG_D);
            display_segment(POS_3, SEG_D);
            break;
        case 2:
            display_segment(POS_2, SEG_D);
            display_segment(POS_1, SEG_D | SEG_E);
            break;
        case 3:
            display_segment(POS_1, SEG_D | SEG_E | SEG_F);
            break;
        case 4:
            display_segment(POS_1, SEG_A | SEG_E | SEG_F);
            break;
        case 5:
            display_segment(POS_1, SEG_A | SEG_F);
            display_segment(POS_2, SEG_A);
            break;
        case 6:
            display_segment(POS_1, SEG_A);
            display_segment(POS_2, SEG_A);
            display_segment(POS_3, SEG_A);
            break;
        case 7:
            display_segment(POS_2, SEG_A);
            display_segment(POS_3, SEG_A);
            display_segment(POS_4, SEG_A);
            break;
        case 8:
            display_segment(POS_3, SEG_A);
            display_segment(POS_4, SEG_A | SEG_B);
            break;
        case 9:
            display_segment(POS_4, SEG_A | SEG_B | SEG_C);
            break;
        case 10:
            display_segment(POS_4, SEG_B | SEG_C | SEG_D);
            break;
        case 11:
            display_segment(POS_4, SEG_C | SEG_D);
            display_segment(POS_3, SEG_D);
            break;
    }
}

#pragma vector=PORT2_VECTOR
__interrupt void port2ISR(){

    isSW2Pressed = true;

    P2IFG &= ~BIT3;
}

#pragma vector=PORT4_VECTOR
__interrupt void port4ISR(){

    isSW1Pressed = true;

    P4IFG &= ~BIT1;
}
