#include <msp430.h>
#include "Handler.h"
#include "event.h"
#include "UCA1.h"

// *** Speicheroptimiert: unsigned char statt UInt ***
static UChar digi[DIGISIZE];
static UChar index;
static Bool is_decrement;

// ----------------------------------------------------------------------------
// State Machine f�r Display
typedef enum { STATE_IDLE = 0, STATE_SEND } TState;
TState display_state;
static UChar display_pos;
static UChar display_cnt;

// ----------------------------------------------------------------------------
// State Machine f�r Number Processing (ersetzt while-Schleifen)
typedef enum { 
    STATE_NUM_IDLE = 0, 
    STATE_NUM_PROCESSING 
} TNumState;
static TNumState num_state;
static UChar num_pos;  // Aktuelle Position beim Verarbeiten

// ----------------------------------------------------------------------------
// Hilfsfunktion zur Behandlung eines Button-Events (reduziert Code-Duplikation)
static void handle_digit_button(UChar event, UChar digit_index) {
    if (Event_tst(event)) {
        Event_clr(event);
        index = digit_index;
        Event_set(EVENT_UDIG);
    }
}

// ----------------------------------------------------------------------------

void Button_Handler(void) {
    // Digit-Buttons: BTN3=0, BTN4=1, BTN5=2, BTN6=3
    handle_digit_button(EVENT_BTN3, 0);
    handle_digit_button(EVENT_BTN4, 1);
    handle_digit_button(EVENT_BTN5, 2);
    handle_digit_button(EVENT_BTN6, 3);

    // Modus-Toggle: BTN1 steuert Increment/Decrement (wird in main.c behandelt)
    // Hier nur den aktuellen Zustand auslesen
    if(TSTBIT(P2OUT, BIT7)) {
        is_decrement = FALSE;
    } else {
        is_decrement = TRUE;
    }
}

// ----------------------------------------------------------------------------
// State Machine basierte Implementierung - keine while-Schleifen, keine Rekursion

void Number_Handler(void) {
    // Start der Verarbeitung (nur wenn im IDLE-Zustand)
    if (Event_tst(EVENT_UDIG)) {
        Event_clr(EVENT_UDIG);
        if (num_state == STATE_NUM_IDLE) {
            num_state = STATE_NUM_PROCESSING;
            num_pos = index;
        }
    }
    
    // State Machine: Schrittweise Verarbeitung (ein Schritt pro Aufruf)
    if (num_state == STATE_NUM_PROCESSING) {
        if (num_pos < DIGISIZE) {
            if (is_decrement) {
                // Decrement: Ziffer verringern
                if (digi[num_pos] > 0) {
                    digi[num_pos]--;
                    // Kein Unterlauf -> fertig
                    num_state = STATE_NUM_IDLE;
                    Event_set(EVENT_7SEG);
                } else {
                    // Unterlauf -> n�chste Position
                    digi[num_pos] = NUMBASE - 1;
                    num_pos++;
                    // Event setzen f�r Weiterverarbeitung
                    Event_set(EVENT_UDIG);
                }
            } else {
                // Increment: Ziffer erh�hen
                digi[num_pos]++;
                if (digi[num_pos] < NUMBASE) {
                    // Kein �berlauf -> fertig
                    num_state = STATE_NUM_IDLE;
                    Event_set(EVENT_7SEG);
                } else {
                    // �berlauf -> n�chste Position
                    digi[num_pos] = 0;
                    num_pos++;
                    // Event setzen f�r Weiterverarbeitung
                    Event_set(EVENT_UDIG);
                }
            }
        } else {
            // Alle Positionen verarbeitet
            num_state = STATE_NUM_IDLE;
            Event_set(EVENT_7SEG);
        }
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
    num_state = STATE_NUM_IDLE;
    num_pos = 0;
    CLRBIT(P1OUT, BIT0); // LED AUS (INKREMENT-Modus)
}
