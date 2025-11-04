#include <msp430.h>
#include "..\base.h"
#include "TA1.h"
#include "event.h"

// Timer-Clock = 613.75 kHz
// Zeitspanne = 3.3 ms
// CCR0 = 2025

#define CNTMAX 5
#define NUM_BUTTONS 2

typedef enum {S0, S1} TState;


typedef struct {
   Int cnt;
   TState state;
} TButtonState;


typedef struct {
   volatile const UChar * const port;
   const UChar mask;
   const TEvent msg;
} TButtonConfig;


LOCAL TButtonState button_states[NUM_BUTTONS] = {
   {0, S0},
   {0, S0}
};

LOCAL const TButtonConfig button_configs[NUM_BUTTONS] = {
   {&P1IN, BIT0, EVENT_BTN1},
   {&P1IN, BIT1, EVENT_BTN2}
};

// Pointer auf aktuellen Button
LOCAL const TButtonConfig *current_config;
LOCAL TButtonState *current_state;

#pragma FUNC_ALWAYS_INLINE(TA1_init)
GLOBAL Void TA1_init(Void) {
   //Pointer zeigen ersten Eintrag je array
   current_config = button_configs;
   current_state = button_states;

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
   CLRBIT(TA1CTL, TAIFG);

   //invertierte logik, also 0 ist gedrückt und 1 losgelassen
   Bool pressed = !TSTBIT(*(current_config->port), current_config->mask);

   if (pressed) {
      current_state->cnt++;
      if (current_state->cnt GT CNTMAX-1) {
         current_state->cnt = CNTMAX-1;
         if (current_state->state EQ S0) {
            current_state->state = S1;
            Event_set(current_config->msg);
            __low_power_mode_off_on_exit();
         }
      }
   } else {
      current_state->cnt--; //kein Tastendruck erkannt
      if (current_state->cnt LE 0) {
         current_state->cnt = 0;
         current_state->state = S0;
      }
   }

  //nächster button
   current_config++;
   current_state++;


   if (current_config GE button_configs + NUM_BUTTONS) {
      current_config = button_configs;  // Zurück zum anfang
      current_state = button_states;
   }
}
