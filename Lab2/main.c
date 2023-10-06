#include <msp430.h> 
#include <stdbool.h>

#define R_LED_DIR_REG       P1DIR
#define R_LED_OUT_REG       P1OUT
#define R_LED_PIN           BIT0

#define G_LED_DIR_REG       P6DIR
#define G_LED_OUT_REG       P6OUT
#define G_LED_PIN           BIT6


void init();
bool isSW1Pressed();
void setRedLED(bool state);
void setGreenLED(bool state);
void state0(bool state);
void state1(bool state);
void state2(bool state);
int  nextState(int* state);

//Initializes the current state to state 0
volatile unsigned int currState = 0;

int main(void){
   init();
	
	while(1){

	  if(isSW1Pressed()) nextState(&currState);

	  //State 0: Only Red LED is on
	  if(currState == 0) state0(true);

	  //State 1: Only Green LED is on
	  if(currState == 1) state1(true);

	  //State 2: Both the Green and Red LEDs are on
	  if(currState == 2) state2(true);

	  //Switch Debounce delay
	  __delay_cycles(200000);
   }
}
void init(){

       //Stop the watchdog timer
       WDTCTL = 0x5A80;
       //Disable the GPIO power-on default high-impedance mode
       PM5CTL0 = 0xFFFE;

       //Button config
       //Set P4.1 as input (all others also being an input)
       P4DIR = 0x00;
       //Enable P4.1 pull-up resistor
       P4REN = BIT1;
       //Set P4.1 as high to configure pull-up
       P4OUT = BIT1;

       //LED Config
       //Set P6 & P1 as outputs
       R_LED_DIR_REG = R_LED_PIN;
       G_LED_DIR_REG = G_LED_PIN;

}

bool isSW1Pressed(){
    return (P4IN & BIT1) == 0x00;
}

void setRedLED(bool state){
    if(state){
        R_LED_OUT_REG |= R_LED_PIN;
    } else {
        R_LED_OUT_REG &= ~R_LED_PIN;
    }
}

void setGreenLED(bool state){
    if(state){
        G_LED_OUT_REG |= G_LED_PIN;
    }
    else {
        G_LED_OUT_REG &= ~G_LED_PIN;
    }
}

void state0(bool state){
    if(state){
        setRedLED(true);
        setGreenLED(false);
    }
}

void state1(bool state){
    if(state){
        setRedLED(false);
        setGreenLED(true);
    }
}

void state2(bool state){
    if(state){
        setRedLED(true);
        setGreenLED(true);
    }
}

int nextState(int* state){
    if(*state == 2){
        *state = 0;
        return *state;
    }
    return *state += 1;
}
