#include <msp430.h>
#include "Handler.h"
#include "event.h"
#include "UCA1.h"

// *** Speicheroptimiert: unsigned char statt UInt ***
static UChar digi[DIGISIZE];
static UChar index;
static Bool is_decrement;


// ----------------------------------------------------------------------------
// Statt struct: Einzelne Variablen zur Speicherersparnis
typedef enum { STATE_IDLE = 0, STATE_SEND } TState;
TState display_state;
static UChar display_pos;
static UChar display_cnt;

// ----------------------------------------------------------------------------

void Button_Handler(void) {

    if (Event_tst(EVENT_BTN3)) {
        Event_clr(EVENT_BTN3);
        index = 0;
        Event_set(EVENT_UDIG);
    }
    if (Event_tst(EVENT_BTN4)) {
        Event_clr(EVENT_BTN4);
        index = 1;
        Event_set(EVENT_UDIG);
    }
    if (Event_tst(EVENT_BTN5)) {
        Event_clr(EVENT_BTN5);
        index = 2;
        Event_set(EVENT_UDIG);
    }
    if (Event_tst(EVENT_BTN6)) {
        Event_clr(EVENT_BTN6);
        index = 3;
        Event_set(EVENT_UDIG);
    }

    if(TSTBIT(P2OUT, BIT7)) {
        is_decrement = FALSE;
    } else {
        is_decrement = TRUE;
    }
}

// *** Rekursion entfernt – effizienter für Stack ***
/*void change_digit(unsigned char pos) {
    while (pos < DIGISIZE) {
        if (is_decrement) {
            if (digi[pos] == 0) {
                digi[pos] = NUMBASE - 1;
                pos++;
            } else {
                digi[pos]--;
                break;
            }
        } else {
            digi[pos]++;
            if (digi[pos] >= NUMBASE) {
                digi[pos] = 0;
                pos++;
            } else {
                break;
            }
        }
    }
}*/

// ----------------------------------------------------------------------------

void Number_Handler(void) {
    if (Event_tst(EVENT_UDIG)){
        Event_clr(EVENT_UDIG);


        UInt idx = index;
        if(is_decrement) {
            while (idx < DIGISIZE){
                digi[idx]++;
                if (digi[idx] < NUMBASE){
                    break;
                }
                digi[idx] = 0;
                idx++;
            }
        }else {
            while (idx < DIGISIZE) {
                if (digi[idx] > 0) {
                    digi[idx]--;
                    break;
                }else {
                    digi[idx] = NUMBASE-1;
                    idx++;
                }
            }
        }
        Event_set(EVENT_7SEG);
    }
}

// ----------------------------------------------------------------------------

void Display_Handler(void) {

    if(Event_tst(EVENT_7SEG)){
        Event_clr(EVENT_7SEG);
        display_pos = 0;
        Event_set(EVENT_SPIRDY);
    }
    if (Event_tst(EVENT_SPIRDY)) {
        Event_clr(EVENT_SPIRDY);
        if (display_pos < DIGISIZE){
            UCA1_emit(DRVWREN + display_pos, digi[display_pos], EVENT_SPIRDY);
            display_pos++;
        }
    }

}

// ----------------------------------------------------------------------------

void Handler_init(void) {
    unsigned char i = 0;
    while (i < DIGISIZE) {
        digi[i] = 0;
        i++;
    }
    is_decrement = FALSE;
    display_state = STATE_IDLE;
    display_pos = 0;
    display_cnt = 0;
    CLRBIT(P1OUT, BIT0); // LED AUS (INKREMENT-Modus)
}
