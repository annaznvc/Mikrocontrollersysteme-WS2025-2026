#include <msp430.h>
#include "..\base.h"
#include "TA0.h"

#define ACKFRQ 613.75
#define HIGH 0x8000
#define LOW  0x0000

// Makro zur Berechnung der Timer-Ticks (Ansatz aus der Folie mit ID und TAIDEX
#define TICK(t) ((UInt)(((ACKFRQ * t) / 8.0) / 5.0) - 1)

// Blinkmuster-Definitionen als Tabellen
LOCAL const UInt muster1[] = {
   HIGH | TICK(2000),   LOW | TICK(500),   0
};

LOCAL const UInt muster2[] = {
   HIGH | TICK(750),  LOW | TICK(750),  0
};

LOCAL const UInt muster3[] = {
   HIGH | TICK(250),   LOW | TICK(250),   0
};

LOCAL const UInt muster4[] = {
   LOW | TICK(500),   HIGH | TICK(500),
   LOW | TICK(1500), 0
};

LOCAL const UInt muster5[] = {
   LOW | TICK(500),   HIGH | TICK(500),
   LOW | TICK(500),   HIGH | TICK(500),
   LOW | TICK(1500), 0
};

LOCAL const UInt muster6[] = {
   LOW | TICK(500),   HIGH | TICK(500),
   LOW | TICK(500),   HIGH | TICK(500),
   LOW | TICK(500),   HIGH | TICK(500),
   LOW | TICK(1500), 0
};

//Tabelle mit Pointern auf Muster
LOCAL const UInt * const blinkmuster[] = {
   muster1, muster2, muster3, muster4, muster5, muster6
};

// Globale Variablen für ISR
LOCAL struct {
   UInt *ptr;                 // Pointer auf aktuelle Phase
   UInt *start;               // Pointer auf Musteranfang
   UInt *next_start;          // Nächstes Muster
   Bool change_pending;       // Musterwechsel angefordert
} st;


#pragma FUNC_ALWAYS_INLINE(TA0_init)
GLOBAL inline Void TA0_init(Void) {
   st.start = (UInt *)blinkmuster[MUSTER1];
   st.ptr = st.start;
   st.next_start = NULL;
   st.change_pending = FALSE;

   /*
    * Erster Ansatz:
    * Die längste High/Low Phase dauert 2 s
    * Timer Clock ist 613,75 kHz
    * Teilungsfaktor: 613750Hz * 2s = 1227500
    * Skal.faktor = 1227500 * 2^16 = 18,73 = 19
    * {/4}, {/5}
    * 4*5 = 20
    *
    *Zweiter Ansatz:
    *Die längste High/Low Phase dauert 2 s
    *Timer Clock ist 613,75 kHz
    *Teilungsfaktor: 613750Hz * 2s = 1227500
    *Skal.faktor = 1227500 * 2^15 = 37,46003 = 38
    *{/5 TAIDEX}, {/8 ID}
    * */
   TA0CTL   = 0; // stop mode, disable and clear flags
   TA0CCTL0 = 0; // no capture mode, compare mode
                 // clear and disable interrupt flag
   TA0CCR0  = 0xFFFF;       // set up Compare Register
   TA0EX0   = TAIDEX_4;     // set up expansion register (zu berechnen) -> 5
   TA0CTL   = TASSEL__ACLK  // 613.75 kHz
            | MC__UP        // Up Mode
            | ID_3       // input divider (zu berechnen) -> 4
            | TACLR         // clear and start Timer
            | TAIE          // enable interrupt
            | TAIFG;        // set interrupt flag
}



GLOBAL Void set_blink_muster(UInt muster_nr) {
    /*
     * Die Funktion muss so erweitert werden,
     * dass ein Blinkmuster selektiert wird.
     * Diese L�sung h�ngt stark von der gew�hlten
     * Datenstruktur ab.
     */
   const UInt * const *ptr_to_muster = blinkmuster;

   if (muster_nr LE MUSTER6) {
      ptr_to_muster += muster_nr;
      st.next_start = (UInt *)*ptr_to_muster;
      st.change_pending = TRUE;
   }
}


#pragma vector = TIMER0_A1_VECTOR
__interrupt Void TIMER0_A1_ISR(Void) {
   UInt cnt;

   CLRBIT(TA0CTL, TAIFG);

   //Prüfe Ende des Musters
   if (*st.ptr EQ 0) {
      //Periode zu Ende
      if (st.change_pending) {
         st.start = st.next_start;
         st.change_pending = FALSE; //Wir setzen change penging wieder auf false, weil es erst wieder true wird, wenn wir btn2 drücken und nen wechselwunsch ahben
      }
      st.ptr = st.start;
   }

   //Lade nächste Phase
   cnt = *st.ptr++;

   //Setze LED entsprechend HIGH/LOW
   if (TSTBIT(cnt, HIGH)) {
      SETBIT(P1OUT, BIT2); //Ausgangsport auf logisch 1
      cnt = cnt BAND (BNOT HIGH);  //HIGH-Bit maskieren
   } else {
      CLRBIT(P1OUT, BIT2); //Ausgangsport auf logisch 0
   }

   TA0CCR0 = cnt;
}
