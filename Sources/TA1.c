#include <msp430.h>
#include "..\base.h"
#include "TA1.h"
#include "event.h"
#include "TA0.h"

#define CNTMAX 1

typedef enum { S0, S1 } TState;

LOCAL UChar index;


typedef struct {
    Int cnt;
    TState state;
} TBtnState;

typedef struct {
    const UChar *port;
    UChar mask;
    TEvent msg;
} TBtnInfo;

// Button-Metadaten
LOCAL const TBtnInfo buttons[6] = {
    { (UChar*)&P1IN, BIT0, EVENT_BTN1 },
    { (UChar*)&P1IN, BIT1, EVENT_BTN2 },
    { (UChar*)&P3IN, BIT0, EVENT_BTN3 },
    { (UChar*)&P3IN, BIT1, EVENT_BTN4 },
    { (UChar*)&P3IN, BIT2, EVENT_BTN5 },
    { (UChar*)&P3IN, BIT3, EVENT_BTN6 },
};

// Button-Zustände
LOCAL TBtnState btnStates[6];

// ----------------------------------------
// Initialisierung
#pragma FUNC_ALWAYS_INLINE(TA1_init)
GLOBAL Void TA1_init(Void) {
    index = 0,
    btnStates[0] = (TBtnState){0, S0};
    btnStates[1] = (TBtnState){0, S0};
    btnStates[2] = (TBtnState){0, S0};
    btnStates[3] = (TBtnState){0, S0};
    btnStates[4] = (TBtnState){0, S0};
    btnStates[5] = (TBtnState){0, S0};

    TA1CTL   = 0;
    TA1CCTL0 = 0;
    TA1CCR0  = 2025 - 1;
    TA1EX0   = TAIDEX_0;
    TA1CTL   = TASSEL__ACLK | MC__UP | ID__1 | TACLR | TAIE;
}

// ----------------------------------------
// ISR mit manuellem "Loop Unrolling"
#pragma vector = TIMER1_A1_VECTOR
__interrupt Void TIMER1_A1_ISR(Void) {
    const TBtnInfo *btn = &buttons[index];
        TBtnState *state = &btnStates[index];

        Bool pressed = !TSTBIT(*btn->port, btn->mask);  // gedrückt = LOW angenommen

        if (pressed) {
            if (state->cnt < CNTMAX) {
                state->cnt++;
                if (state->cnt >= CNTMAX && state->state == S0) {
                    state->state = S1;
                    Event_set(btn->msg);
                    __low_power_mode_off_on_exit();
                }
            }
        } else {
            if (state->cnt > 0)
                state->cnt--;
            if (state->cnt == 0)
                state->state = S0;
        }

        // Index rotieren (0–5)
        index++;
        if (index >= 6) {
            index = 0;
        }

        // Interrupt-Flag löschen
        CLRBIT(TA1CTL, TAIFG);
}
