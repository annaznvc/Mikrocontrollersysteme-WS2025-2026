#include <msp430.h>
#include "..\base.h"
#include "TA0.h"

LOCAL const UInt muster[Tabsize] = {
    HIGH | TICK(2000), LOW | TICK(500), 0,
    HIGH | TICK(750), LOW | TICK(750), 0,
    HIGH | TICK(250), LOW | TICK(250), 0,
    LOW | TICK(500), HIGH | TICK(500), LOW | TICK(1500), 0,
    LOW | TICK(500), HIGH | TICK(500), LOW | TICK(500), HIGH | TICK(500), LOW | TICK(1500), 0,
    LOW | TICK(500), HIGH | TICK(500), LOW | TICK(500), HIGH | TICK(500), LOW | TICK(500), HIGH | TICK(500), LOW | TICK(1500), 0,
 };

LOCAL const UInt blinkmuster[6] = {0,3,6,9,13,19};

LOCAL const UInt *ptr;

LOCAL const UInt *ctr;


GLOBAL Void set_blink_muster(UInt arg) {
    ptr = &muster[blinkmuster[arg]];
    ctr = &muster[blinkmuster[arg]];
}

#pragma FUNC_ALWAYS_INLINE(TA0_init)
GLOBAL Void TA0_init(Void) {
   //ptr = muster1; //neu index statt 0
   //ctr = ptr;
   ptr = &muster[2];
   ctr = &muster[0];

   TA0CTL   = 0; // stop mode, disable and clear flags
   TA0CCTL0 = 0; // no capture mode, compare mode
                 // clear and disable interrupt flag
   TA0CCR0  = 30688 - 1 ;       // set up Compare Register
   TA0EX0   = TAIDEX_5;     // set up expansion register
   TA0CTL   = TASSEL__ACLK  // 613.75 kHz
            | MC__UP        // Up Mode
            | ID__8         // input divider
            | TACLR         // clear and start Timer
            | TAIE          // enable interrupt
            | TAIFG;        // set interrupt flag
}


#pragma vector = TIMER0_A1_VECTOR
__interrupt Void TIMER0_A1_ISR(Void) {

    CLRBIT(TA0CTL,TAIFG);

    if(*ptr EQ 0){
        ptr = ctr;
    }
    /*if(*ptr == 0){
        ptr = ctr;
    }*/


    UInt cnt = *ptr++;//++ neu
    //ptr++;

    if (TSTBIT(cnt, HIGH)) {
        SETBIT(P1OUT, BIT2);  // LED an
    } else {
        CLRBIT(P1OUT, BIT2);  // LED aus
    }

    TA0CCR0 = ~HIGH & cnt;          // Neue Dauer setzen

}

