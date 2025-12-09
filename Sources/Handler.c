#include <msp430.h>
#include "Handler.h"
#include "event.h"
#include "UCA1.h"

static UChar digi[DIGISIZE]; //Einer, Zehner, Hunderter, Tausender
static UChar index;
static Bool is_decrement;

// State Machine fï¿½r Display
typedef enum { STATE_IDLE, STATE_SEND } TState; //idle = display bleibt unverï¿½ndert, send = zahl auf display aktualisiert sich
static TState display_state;
static UChar display_pos;

// State Machine fï¿½r Number Processing (ersetzt Schleifen)
typedef enum { STATE_NUM_IDLE, STATE_NUM_PROCESSING } TNumState;
static TNumState num_state;
static UChar num_pos;

// ----------------------------------------------------------------------------
// Hilfsfunktion zur Behandlung eines Button-Events (reduziert Code-Duplikation)
static void handle_digit_button(UChar event, UChar digit_index) {
    if (Event_tst(event)) {
        Event_clr(event);
        index = digit_index; //Welche Stelle soll geï¿½ndert werden?
        Event_set(EVENT_UDIG);
    }
}
// sammelt Button Events und merkt sich, welche stelle verändert werden soll und ob dekrementiert wird
GLOBAL Void Button_Handler(Void) {
    //Fï¿½r jeden Button prï¿½fen, ob er gedrï¿½ckt wurde
    handle_digit_button(EVENT_BTN3, 0);
    handle_digit_button(EVENT_BTN4, 1);
    handle_digit_button(EVENT_BTN5, 2);
    handle_digit_button(EVENT_BTN6, 3);

    //BTN1 steuert Increment/Decrement (wird in main.c behandelt), hier nur den aktuellen Zustand auslesen
    if(TSTBIT(P2OUT, BIT7)) {
        is_decrement = TRUE;
    } else {
        is_decrement = FALSE;
    }
}

// ----------------------------------------------------------------------------
// Führt Inkrementier und Dekrementier Logik aus mit Carry, Borrows

GLOBAL Void Number_Handler(Void) {
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
                if (digi[num_pos] > 0) {
                    digi[num_pos]--;
                    num_state = STATE_NUM_IDLE;
                    Event_set(EVENT_7SEG);
                } else {
                    // Unterlauf, nï¿½chste Position
                    digi[num_pos] = NUMBASE - 1; //10-1 = 9
                    num_pos++; //eine stelle Weiter +1
                    Event_set(EVENT_UDIG); //neue UDIG Aufrufe solange ï¿½berschlag gibt, bis man durch ist (Schleife umgangen)
                }
            } else {
                digi[num_pos]++; //inkrementieren nicht an bedingung gebunden anders als bei dekrementierung wiel bei 0-- falsch 255 rauskommt (UChar)
                if (digi[num_pos] < NUMBASE) {
                    num_state = STATE_NUM_IDLE;
                    Event_set(EVENT_7SEG);
                } else {
                    digi[num_pos] = 0;
                    num_pos++;
                    Event_set(EVENT_UDIG);
                }
            }
        } else {
            // Alle Positionen verarbeitet 0 bis 3, kein increment oder decrement der idle setzen kï¿½nnte, muss selbst gemacht werden am ende
            num_state = STATE_NUM_IDLE;
            Event_set(EVENT_7SEG);
        }
    }
}

// ----------------------------------------------------------------------------
//sendet die 4 stellen nacheinander per spi ans display
GLOBAL Void Display_Handler(Void) {

    if(Event_tst(EVENT_7SEG)){ //display muss aktualisiert werden
        Event_clr(EVENT_7SEG);
        display_pos = 0; //beim ersten digit starten
        Event_set(EVENT_SPIRDY); //SPI darf erste ziffer senden
    }
    if (Event_tst(EVENT_SPIRDY)) {
        Event_clr(EVENT_SPIRDY);
        if (display_pos < DIGISIZE){ //haben wir noch ziffern ï¿½brig?
            UCA1_emit(DRVWREN + display_pos, digi[display_pos], EVENT_SPIRDY); //versand an display: aktuelle position + wert der ziffer + event spirdy erneut setzen
            display_pos++; //gehe zur nï¿½chsten stelle im display
        }
    }

}

// ----------------------------------------------------------------------------

#pragma FUNC_ALWAYS_INLINE(Handler_init)
GLOBAL inline Void Handler_init(Void) {
    unsigned char i = 0;
    while (i < DIGISIZE) {
        digi[i] = 0; //setze alle ziffern auf 0
        i++;
    }
    is_decrement = FALSE;
    display_state = STATE_IDLE;
    display_pos = 0;
    num_state = STATE_NUM_IDLE;
    num_pos = 0;
    CLRBIT(P1OUT, BIT0); // LED AUS (INKREMENT-Modus)
}
