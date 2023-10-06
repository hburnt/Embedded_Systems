/**
 * Name:            Hunter Burnett
 * Date:            8/8/2023
 * Assignment:      Lab 5 - Game Controller
 * YouTube:         https://www.youtube.com/watch?v=MjVKTMPPXGg&ab_channel=HunterBurnett
 *
 * This program:
 * This program uses the UART communication to control a plane in a video game. Pressing SW1 or SW2 will make the plane shoot a bullet
 * and adjusting the potentiometer will move the plane around.
 *
 */

#include <msp430.h> 
#include <stdbool.h>
#include <stdint.h>

bool isSW1Pressed = false;
bool isSW2Pressed = false;
volatile const int SHOOT = 255;
volatile int DATA;
void init();
void initUART1();
void initClocks();
void initGPIO();
void writeUART(int data);
void initADC();
int getADC();
int shoot();
int main(void)
{
    init();
	while(1){
	    bool buttonPressed = (isSW1Pressed || isSW2Pressed);
	    __delay_cycles(100000); //lets data update
	    if(buttonPressed){
	        DATA = SHOOT;
	        writeUART(DATA);
	        isSW1Pressed = false;
	        isSW2Pressed = false;
	    }
	    else{
	        DATA = getADC();
	        writeUART(DATA);
	    }
	}
}

void init(){
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;
    initClocks();
    initUART1();
    initGPIO();
    initADC();
    __enable_interrupt();
}

void initUART1(){
    // Configure UART pins
    P4SEL0 |= (BIT3 | BIT2); // set 2-UART pin as second function
    P4SEL1 &= ~(BIT3 | BIT2);

    //Configure UART
    UCA1CTLW0 = UCSWRST; //Hold UART in reset state

    UCA1CTLW0 |= UCSSEL__ACLK; // CLK = ACLK

    UCA1BR0 = 3; // 32768/9600
    UCA1BR1 = 0;
    UCA1MCTLW |= 0X9200; // Using the table 22-4: 0x9200

    UCA1CTLW0 &= ~UCSWRST; //Exit reset state
    UCA1IE |= UCRXIE;   // Enable USCI_A0 RX interrupt
}

void initClocks(){
    // Set XT1CLK as ACLK source
    CSCTL4 |= SELA__XT1CLK;
    // Use external clock in low-frequency mode
    CSCTL6 |= XT1BYPASS_1 | XTS_0;
    // Use clock divider value of 1
    CSCTL5 |= DIVM_0;
}

void initGPIO(){

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
}

int getADC(){
    volatile int ADC_Result;
    ADCCTL0 |= ADCENC | ADCSC;
    while(ADCCTL1 & ADCBUSY);
    ADC_Result = ADCMEM0;
    if(ADC_Result == 255) ADC_Result -= 1;
    return ADC_Result;
}

int shoot(){
    return SHOOT;
}
void initADC(){
    ADCCTL0 |= ADCSHT_2 | ADCON_1; //sample and hold time to 16 ADC clock cycles
    ADCCTL1 |= ADCSHP; //Sample and hold pulse mode
    ADCCTL2 &= ~ADCRES;//Set Resolution to 0
    ADCCTL2 |= ADCRES_0; //8-bit resolution
    ADCMCTL0 |= ADCINCH_1 | ADCSREF_0; // sets V+ = 3.3V and V- = 0V
}
void writeUART(int data){
    UCA1TXBUF = data;
}

#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
    UCA1TXBUF = UCA1RXBUF;
    __no_operation();

    UCA1IFG = 0;
}

#pragma vector=PORT2_VECTOR
__interrupt void port2ISR(){

    isSW1Pressed = true;
    //DATA = SHOOT;
    P2IFG &= ~BIT3;
}

#pragma vector=PORT4_VECTOR
__interrupt void port4ISR(){

    isSW2Pressed = true;
    //DATA = SHOOT;
    P4IFG &= ~BIT1;
}

