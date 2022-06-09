# KitchenTimer

Kitchedn timer to build with arduino

## Instructions

turn the rotary encoder clockwise to increase the timer (display shows minutes). 
Turn it counter clockwise to decrease it.
Once the timer reaches zero a not so loud but vaguely annoying beeep will turn on. 
Move the rotary encoder in either direction to shut down the beep.
After 30 seconds of inactivity (doesn't count when the timer is running) the display will turn off to save battery.

## Schematics

 ![Schematics](/schematics.png)
**Note:** resistor on led matrix ground is probably not needed.

 ![Schematics with standalone atmega](/schematics_stand_alone.png)
