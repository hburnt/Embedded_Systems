/**
 * Name:            Hunter Burnett
 * Date:            7/31/2023
 * Assignment:      Lab 4 - Ultrasonic Alarm
 * YouTube:         https://www.youtube.com/watch?v=J91vRdDH9Ls&ab_channel=HunterBurnett
 *
 * This program:
 * For this lab for the code to work properly the Grove BoosterPack must be connected to the MPS430FR2355.
 * With the board being upright the ultrasonic sensor is conencted to the bottom left position on the board
 * and the buzzer is connected to the bottom left. When the code is run one must press SW1 to arm the sensor.
 * When the sensor is armed it will continue to take distance readings and compare them to the initial distance value.
 * Once the sensor detects a new distance value that is significantly less that the original value the buzzer will go off.
 * To reset the buzzer and distance calibration one must press SW1 again to put it to the armed state again.
 */

#include <msp430.h> 
#include <stdbool.h>

volatile const float SMCLK_PERIOD = 0.000001;
volatile const float SPEED_OF_SOUND = 343;
volatile const float SENSOR_TOLERANCE = 0.01;
bool isButtonPressed = false;

typedef enum { INITIALIZE, ARMED, ALARM, MAX_STATES } state_t;

typedef struct {
    bool isButtonPressed;
    float initialDistance;
    float newDistance;
} inputs_t;

state_t runInitializeState(inputs_t inputs);
state_t runArmedState(inputs_t inputs);
state_t runAlarmState(inputs_t inputs);
state_t (*state_table[MAX_STATES])(inputs_t) = {runInitializeState, runArmedState, runAlarmState};

void init();
void startTimer();
void initGpio();
void stopTimer();
void setBuzzer(bool enable);
float getDistance();
int main(void)
{
    init();
    volatile inputs_t inputs;
    inputs.isButtonPressed = false;
    bool buzzerEnabled = false;
    state_t currentState = INITIALIZE;
    state_t nextState = currentState;

    while(1){
        if(currentState < MAX_STATES) nextState = state_table[currentState](inputs);

        if(currentState == INITIALIZE){
            inputs.initialDistance = getDistance();
            buzzerEnabled = false;
        }
        if(currentState == ARMED){
            isButtonPressed = false;
            buzzerEnabled = false;
        }
        if(currentState == ALARM){
            buzzerEnabled = true;
        }
        setBuzzer(buzzerEnabled);
        inputs.newDistance = getDistance();
        inputs.isButtonPressed = isButtonPressed;
        currentState = nextState;
    }
}

float getDistance(){
    P5DIR |= BIT4; //Setting P5.4 to an o/p
    P5OUT  = BIT4; // Setting P5.4 high
    __delay_cycles(10); //delaying 10 microseconds
    P5OUT &= ~BIT4; //Setting P5.4 low
    P5DIR &= ~BIT4; //Set P5.4 as i/p

    //Continue until P5.4 goes high
    while(!(P5IN & BIT4)) continue;
        TB0R = 0;
        startTimer();
    //Continue until P5.4 is low
    while((P5IN & BIT4)) continue;
        stopTimer();

    volatile int count = TB0R;
    volatile float distance = ((count*(SMCLK_PERIOD))*SPEED_OF_SOUND)*0.5;
    return distance;
}

void init(){
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5; //Disable the GPIO power-on default high-impedance mode
    initGpio();
}

state_t runInitializeState(inputs_t inputs){
    if(inputs.isButtonPressed){
        return ARMED;
    } return INITIALIZE;
}

state_t runArmedState(inputs_t inputs){
    if(inputs.newDistance < (inputs.initialDistance - SENSOR_TOLERANCE)){
        return ALARM;
    } return ARMED;
}

state_t runAlarmState(inputs_t inputs){
    if(inputs.isButtonPressed == true){
        return INITIALIZE;
    } return ALARM;
}

void initGpio(){
    //SW1 Enabled
    P4DIR &= ~BIT1; // set pin 1 as input
    P4REN |= BIT1;  // enable pin 1 resistor
    P4OUT |= BIT1;  // enable pin 1 pull-up resistor
    P4IE |= BIT1;   // enables interrupt
    P4IES |= BIT1;  // high-to-low trigger condition
    P4IFG &= ~BIT1;

    //Enabling the Buzzer (Connected to bottom right connector)
    P6DIR |= BIT3; //Set port 6 pin 3 as an output

    __enable_interrupt();
}

void startTimer(){
    TB0CTL = TBSSEL__SMCLK | MC__CONTINUOUS;
}

void stopTimer(){
    TB0CTL = TBSSEL__SMCLK | MC__STOP;
}

void setBuzzer(bool enable){
    if(enable){
        P6OUT |= BIT3;
        return;
    } P6OUT &= ~BIT3;
}

#pragma vector=PORT4_VECTOR
__interrupt void port4ISR(){
    isButtonPressed = true;

    P4IFG &= ~BIT1;
}
