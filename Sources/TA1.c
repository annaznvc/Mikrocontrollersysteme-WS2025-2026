#include <msp430.h>
#include "..\base.h"
#include "TA1.h"
#include "event.h"

// Timer-Clock = 613.75 kHz
// Zeitspanne = 3.3 ms
// CCR0 = 2025

#define CNTMAX       5
#define NUM_BUTTONS  2

typedef enum { S0, S1 } TState;

typedef struct {
   Int     cnt;
   TState  state;
} TButtonState;

typedef struct {
   volatile const UChar * const port;
   const UChar  mask;
   const TEvent msg;
} TButtonConfig;

// Pointer auf aktuellen + Grenzen (für pointerbasiertes Wrapping)
LOCAL const TButtonConfig *first_config;
LOCAL const TButtonConfig *last_config;
LOCAL const TButtonConfig *current_config;

LOCAL TButtonState *first_state;
LOCAL TButtonState *current_state;

#pragma FUNC_ALWAYS_INLINE(TA1_init)
GLOBAL Void TA1_init(Void) {
   // --- Zustände im RAM (ändern sich zur Laufzeit), ohne Index initialisieren
   static TButtonState button_states[NUM_BUTTONS];
   TButtonState *s = button_states;
   s->cnt = 0;  s->state = S0;  s++;         // BTN1
   s->cnt = 0;  s->state = S0;               // BTN2

   // --- Fixe Button-Konfigurationen im Flash (spart RAM)
   static const TButtonConfig config_btn1 = { &P1IN, BIT0, EVENT_BTN1 };
   static const TButtonConfig config_btn2 = { &P1IN, BIT1, EVENT_BTN2 };

   // --- Pointer-Setup (keine Indizierung)
   first_config  = &config_btn1;
   last_config   = &config_btn2;
   current_config = first_config;

   first_state   = button_states;
   current_state = first_state;

   // --- Timer-Einstellungen (ACLK=613.75 kHz, Up-Mode, ID=/1, IDEX=/1)
   TA1CTL   = 0;          // stop, Flags clear
   TA1CCTL0 = 0;          // Compare mode, IFG clear/disable
   TA1CCR0  = 2025 - 1;   // 3.3 ms
   TA1EX0   = TAIDEX_0;   // IDEX = /1
   TA1CTL   = TASSEL__ACLK   // ACLK
            | MC__UP         // Up Mode
            | ID__1          // ID = /1
            | TACLR          // Counter clear/start
            | TAIE           // TA1 overflow interrupt enable
            | TAIFG;         // trigger first ISR immediately (optional)
}

#pragma vector = TIMER1_A1_VECTOR
__interrupt Void TIMER1_A1_ISR(Void) {
   // TA1 overflow IFG manuell löschen
   CLRBIT(TA1CTL, TAIFG);

   // invertierte Logik: 0 = gedrückt, 1 = losgelassen
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
   } else { // kein Tastendruck erkannt
      current_state->cnt--;
      if (current_state->cnt LE 0) {
         current_state->cnt = 0;
         current_state->state = S0;
      }
   }

   // --- Nächster Button (pointerbasiert, ohne Indizierung)
   current_config++;
   current_state++;

   if (current_config GT last_config) {
      current_config = first_config;   // zurück zum Anfang
      current_state  = first_state;
   }
}
