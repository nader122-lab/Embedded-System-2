# Lab 3 Starter Code

Clone this project with the starter code for lab 3. First read it and then run it, before making any changes.

The code displays a sequence of colour whilst varying the brightness of the on-board LED's on the KL25Z development board. There are 32 brightness levels. Initially, the cycle for the sequence of colour takes 6 sec but, at random intervals, the system changes between this rate and a faster one where the cycle is covered in 2 sec. 

There are two tasks:
  1. The 'randomPress' tasks simulates a button press at random intervals.
  2. The 'toogleRate' task toggles the update rate on each button press.
  3. the 'colourChange' task simulates the change of colour in the sequence. 
and also a ISR (interrupt service routine) for the PIT (programmanble interrupt timer). This is where the brightness
changes are made.
