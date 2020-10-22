/* ------------------------------------------
       ECS642/ECS714 Lab3

   The LED is displayed at different brightness levels using PWM
   The PIT is used to time the transition between the brightness levels
   A simulated button switches between two rates (by changing the PIT load value): 
       * a fast one cycles through all the brightness levels in 2 s
       * a slow one takes 10 s
  -------------------------------------------- */

#include <MKL25Z4.h>
#include <stdbool.h>
#include "../include/SysTick.h"
#include "../include/pit.h"
#include "../include/TPMPWM.h"
#include "../include/triColorLedPWM.h"


/* --------------------------------------
     Documentation
     =============
     This is a cyclic system with a cycle time of 10ms

     The file has a main function, two tasks
       1. randomPress: this simulates presses at random intervals between 5 and 15 sec
       2. toggleRate: this toggles between a fast and slow rate for changing the LED brightness
     and the PIT interrupt service routine which changes the brightness of 
     one of the LEDs
 -------------------------------------- */
 
 /*----------------------------------------------------------------------------
  * nextRand: get next random number 
  *   Based on https://en.wikipedia.org/wiki/Linear_congruential_generator
  * --------------------------------------------------------------------------- */
uint32_t seed = 0x12345678 ;

// Returns a 32 bit number which is too long for us
uint32_t nextRand(void) {
  seed = (1103515245 * seed + 12345) ; 
  return seed ; 
}

// Generate random count in range 5-15 sec 
//    - take top 10 bits - max is 1023
//    - add 477 so max is 1500 (15 sec), min is 477  (4.8 sec)
uint32_t randCount() {
  uint32_t r1023 = (nextRand() & 0xFFC00000) >> 22 ; // top 10 bits
  return r1023 + 477 ;
}

/*----------------------------------------------------------------------------
   Task: randomPressTask

   Generate a signal at random intervals between 5 and 15 sec
   DO NOT CHANGE
 *----------------------------------------------------------------------------*/
#define UP 0
#define DOWN 1
#define DOWNCOUNT 20

int stateRPress ;       // state of the task
uint32_t countRPress ;  // count for timing

// initialise state
void initRandomPressTask() {
  countRPress =randCount() ;
  stateRPress = UP ;
}

bool signalR = false ; // signal to other task

void randomPressTask() {
    if (countRPress > 0) countRPress-- ;
    switch (stateRPress) {
        case UP:
            if (countRPress == 0){
                signalR = true ;
                stateRPress = DOWN ;
                countRPress = DOWNCOUNT ;
            }
            break ;
        case DOWN:
            if (countRPress == 0){
                stateRPress = UP ;
                countRPress = randCount();
            }
            break ;
    }
}

/* -------------------------------------
    Programmable Interrupt Timer (PIT) interrupt handler

      Check each channel to see if caused interrupt
      Write 1 to TIF to reset interrupt flag

    Change sthe LED brightness. KEEP THIS ISR but changes are needed
   ------------------------------------- */

// PIT load values
// The larger the count, the less frequency the interrupts
const uint32_t pitSlowCount = PITCLOCK * 10 / 32 ; // all 32 levels in 10 s
const uint32_t pitFastCount = PITCLOCK * 2 / 32 ; // all 32 levels in 2 s

// Brightness level
unsigned int bright = 0 ;  // the current brightness

void PIT_IRQHandler(void) {
    // clear pending interrupts
    NVIC_ClearPendingIRQ(PIT_IRQn);

    if (PIT->CHANNEL[0].TFLG & PIT_TFLG_TIF_MASK) {
        // clear TIF
        PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF_MASK ;
        
        // add code here for channel 0 interrupt
        
        // -- start of demo code    
        bright = (bright + 1) % (MAXBRIGHTNESS + 1);        
        setLEDBrightness(Green, bright) ;
        // -- end of demo code
    }

    if (PIT->CHANNEL[1].TFLG & PIT_TFLG_TIF_MASK) {
        // clear TIF
        PIT->CHANNEL[1].TFLG = PIT_TFLG_TIF_MASK ;

        // add code here for channel 1 interrupt
        // -- end of demo code
    }
}

/*----------------------------------------------------------------------------
   Task: toggleRate

   Toggle the rate of upadtes to the LEDs on every signal

   KEEP THIS TASK, but changes may be needed
*----------------------------------------------------------------------------*/

#define FAST 0
#define SLOW 1

int rateState ;  // this variable holds the current state

// initial state of task
void initToggleRateTask() {
    setTimer(0, pitSlowCount) ;
    rateState = SLOW ;
}

void toggleRateTask() {
    switch (rateState) {
        case FAST:  
            if (signalR) {                   // signal received
                signalR = false ;            // acknowledge
                setTimer(0, pitSlowCount) ;  // update PIT
                rateState = SLOW ;           // ... the new state
            }
            break ;
            
        case SLOW:
            if (signalR) {                   // signal received
                signalR = false ;            // acknowledge
                setTimer(0, pitFastCount) ;  // update PIT
                rateState = FAST ;           // ... the new state
            }
            break ;
  }
}



/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void) {
    configureLEDforPWM() ;   // Configure LEDs for PWM control
    configurePIT(0) ;        // configure PIT channel 0
    configureTPMClock() ;    // clocks to all TPM modules
    configureTPM0forPWM() ;  // configure PWM on TPM0 (blue LED)
    configureTPM2forPWM() ;  // configure PWM on TPM2 (red, green LEDs)
   
    Init_SysTick(1000) ;  // initialse SysTick every 1 ms

    // start everything
    setLEDBrightness(Red, 0) ;
    setLEDBrightness(Green, 0) ;
    setLEDBrightness(Blue, 0) ;

    initRandomPressTask() ;  // initialise task state
    initToggleRateTask() ;   // initialise task state
    startTimer(0) ;
    waitSysTickCounter(10) ;  // initialise delay counter
    while (1) {      // this runs forever
        randomPressTask() ;  // Generate signals for a simulated button
        toggleRateTask();    // Toggle LED update rate on every press signal
        // delay
        waitSysTickCounter(10) ;  // cycle every 10 ms 
    }
}
