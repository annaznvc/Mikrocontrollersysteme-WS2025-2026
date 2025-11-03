#include <msp430.h>
#include "..\base.h"
#include "TA1.h"
#include "event.h"

// Timer-Clock = 613.75 kHz, ID = /1, IDEX = /1
// Zeitspanne = 3.3 ms
// CCR0 = 2025

#define CNTMAX 6  // Max. Wert für die Hysterese

typedef enum {S0, S1} TState;

// Zustandsvariablen für BTN1
LOCAL struct {
   Int cnt;
   TState state;
} var1;

// Zustandsvariablen für BTN2
LOCAL struct {
   Int cnt;
   TState state;
} var2;

// Konfiguration für BTN1
LOCAL const struct {
   const UChar * const port;
   const UChar mask;
   const TEvent msg;
} btn1 = {(UChar *)(&P1IN), BIT0, EVENT_BTN1};

// Konfiguration für BTN2
LOCAL const struct {
   const UChar * const port;
   const UChar mask;
   const TEvent msg;
} btn2 = {(UChar *)(&P1IN), BIT1, EVENT_BTN2};

// Flag für abwechselnde Abfrage
LOCAL Bool check_btn1;


#pragma FUNC_ALWAYS_INLINE(TA1_init)
GLOBAL Void TA1_init(Void) {
   var1.cnt = 0;
   var1.state = S0;
   var2.cnt = 0;
   var2.state = S0;

   check_btn1 = TRUE;

   TA1CTL = 0;
   TA1CCTL0 = 0;
   TA1CCR0 = 2025-1;
   TA1EX0 = TAIDEX_0;
   TA1CTL = TASSEL__ACLK
          | MC__UP
          | ID__1
          | TACLR
          | TAIE
          | TAIFG;
}


#pragma vector = TIMER1_A1_VECTOR
__interrupt Void TIMER1_A1_ISR(Void) {
   Int *Cnt;
   TState *State;
   UChar Taste;
   TEvent msg;

   CLRBIT(TA1CTL, TAIFG);

   // Wähle aktuellen Button
   if (check_btn1) {
      Cnt = &var1.cnt;
      State = &var1.state;
      Taste = TSTBIT(P1IN, BIT0);
      msg = EVENT_BTN1;
      check_btn1 = FALSE;
   } else {
      Cnt = &var2.cnt;
      State = &var2.state;
      Taste = TSTBIT(P1IN, BIT1);
      msg = EVENT_BTN2;
      check_btn1 = TRUE;
   }

   // Entprellungslogik wie in der Vorlesung
   // Ein nicht gedrückter Button ist mit 1 aktiv
   if (Taste) {
      if (--(*Cnt) LT 0) {
         *Cnt = 0;
         *State = S0;
      }
      return;
   }

   if (++(*Cnt) GT CNTMAX-1) {
      *Cnt = CNTMAX-1;
      if (*State EQ S0) {
         *State = S1;
         Event_set(msg);
         __low_power_mode_off_on_exit();
      }
   }
}
