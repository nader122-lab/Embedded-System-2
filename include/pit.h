#ifndef PIT_H
#define PIT_H
#include <MKL25Z4.h>

// The PIT clock is half the CPU clock
#define PITCLOCK (10485760)

// prototypes
void configurePIT(int channel) ;
void startTimer(int channel) ;
void stopTimer(int channel) ;
void setTimer(int channel, uint32_t timeout) ;

#endif