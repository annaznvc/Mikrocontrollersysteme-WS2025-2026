#include <msp430.h>
#include "..\base.h"
#include "TA0.h"

#define ACKFRQ 30.6875  // kHz (nach Teilung)
#define HIGH 0x8000
#define LOW  0x0000

// Makro zur Berechnung der Timer-Ticks
#define TICK(t) ((UInt)(ACKFRQ * (t)) - 1)

// Blinkmuster-Definitionen als ROM-Tabellen
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

// Lookup-Tabelle mit Pointern auf Muster
LOCAL const UInt * const blinkmuster[] = {
   muster1, muster2, muster3, muster4, muster5, muster6
};

// Globale Variablen für ISR - minimal!
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

   TA0CTL   = 0; // stop mode, disable and clear flags
   TA0CCTL0 = 0; // no capture mode, compare mode
                 // clear and disable interrupt flag
   TA0CCR0  = 0xFFFF;       // set up Compare Register
   TA0EX0   = TAIDEX_0;     // set up expansion register
   TA0CTL   = TASSEL__ACLK  // 613.75 kHz
            | MC__UP        // Up Mode
            | ID__1         // input divider
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

   // Prüfe Ende des Musters
   if (*st.ptr EQ 0) {
      // Periode zu Ende
      if (st.change_pending) {
         st.start = st.next_start;
         st.change_pending = FALSE;
      }
      st.ptr = st.start;
   }

   // Lade nächste Phase
   cnt = *st.ptr++;

   // Setze LED entsprechend HIGH/LOW
   if (TSTBIT(cnt, HIGH)) {
      SETBIT(P1OUT, BIT2);
      cnt = cnt BAND (BNOT HIGH);  // HIGH-Bit maskieren
   } else {
      CLRBIT(P1OUT, BIT2);
   }

   TA0CCR0 = cnt;
}
