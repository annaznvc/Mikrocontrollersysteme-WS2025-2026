#include <msp430.h>
#include "..\base.h"
#include "TA1.h"
#include "event.h"


#pragma FUNC_ALWAYS_INLINE(TA1_init)
GLOBAL Void TA1_init(Void) {

   TA1CTL   = 0; // stop mode, disable and clear flags
   TA1CCTL0 = 0; // no capture mode, compare mode
                 // clear and disable interrupt flag
   TA1CCR0  = 0xFFFF;          // set up Compare Register
   TA1EX0   = TAIDEX_0;        // set up expansion register
   TA1CTL   = TASSEL__ACLK     // 613.75 kHz
            | MC__UP           // Up Mode
            | ID__1            // input divider
            | TACLR            // clear and start Timer
            | TAIE;            // enable interrupt
}



#pragma vector = TIMER1_A1_VECTOR
__interrupt Void TIMER1_A1_ISR(Void) {

   /*
    * Der Inhalt der ISR ist zu implementieren
    */
    
}
