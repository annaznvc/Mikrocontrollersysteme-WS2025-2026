#include <msp430.h>
#include "event.h"
#include "UCA1.h"

typedef Int (* StateFunc)(Void);

// SPI4x7SEG Data Frame struct
// D15 D14 D13 D12 D11  D10  D9  D8   D7 D6 D5 D4 D3 D2 D1 D0
// X   X    X   X  R/W    Reg.-Adr.   MSB <--- Data ----> LSB
// -----------------------------------------------------------

// R/W = 0: Leseoperation
// R/W = 1: Schreiboperation

// Register Address Map
// Register       HEX Code              Reg-Adr
//                              D15:D12 D10 D9 D8
// Digit 0            0xX0         X     0   0  0
// Digit 1            0xX1         X     0   0  1
// Digit 2            0xX2         X     0   1  0
// Digit 3            0xX3         X     0   1  1
// Display Control    0xX4         X     1   0  0
//                    0xX5         X     1   0  1
//                    0xX6         X     1   1  0
//                    0xX7         X     1   1  1

// SLK ist Low im nicht aktiven Zustand
// Datenausgabe im SPI-Master erfolgt mit der steigender SCK-Flanke
// SEL ist High im nicht aktiven Zustand

LOCAL Int State0(Void);
LOCAL Int State1(Void);
LOCAL Int State2(Void);
LOCAL Int State3(Void);
LOCAL Int State4(Void);
LOCAL Int State5(Void);

LOCAL struct {
   UInt adr;
   UInt val;
   UInt ret;
   UInt msg;
   Bool busy;
   StateFunc state;
} uca1;

#define SCKFRQ    (60.00e3)
#define ACLKFRQ  (613.75e3)
#define BWR      ((UInt)(ACLKFRQ / SCKFRQ))

#define TICK(t)  ((UInt)((ACLKFRQ * (t)) / 1.0) - 1)
#define DLY1     (TICK(50.0e-6))
#define DLY2     (TICK(10.0e-6))
#define DLY3     (TICK(30.0e-6))

#pragma FUNC_ALWAYS_INLINE(TB0_init)
LOCAL inline Void TB0_init(Void) {
   TB0CTL   = 0;    // stop mode, disable and clear flags
   TB0CCTL0 = 0;    // no capture mode, compare mode
                    // clear and disable interrupt flag
   TB0CCR0  = DLY1;      // set up Compare Register
   TB0EX0 = TBIDEX_0;    // set up expansion register /1
   TB0CTL = TBSSEL__ACLK
          | MC__STOP     // stop mode
          | ID__1        // input divider /1
          | TBCLR        // clear and start timer
          | 0            // interrupt disabled
          | 0;           // TAIFG clear
}

#pragma FUNC_ALWAYS_INLINE(UCA1_init)
GLOBAL inline Void UCA1_init(Void) {
   uca1.state = State0;
   uca1.busy  = FALSE;
   TB0_init();
   // set up Universal Serial Communication Interface A
   SETBIT(UCA1CTLW0, UCSWRST);  // UCA1 software reset
   UCA1BRW = BWR;               // prescaler

   // in Übereinstimmung mit dem SPI-Timing-Diagramm vom MSP430G2302
   UCA1CTLW0 = 0                // 15: clock phase select: Data is changed on the first UCLK edge
                                // and captured on the following edge
             | 0                // 14: clock polarity: inactive low
             | UCMSB            // 13: MSB first
             | 0                // 12: 8-bit data
             | UCMST            // 11: SPI master mode
             | UCMODE_0         // 10-9: mode select: 3-pin SPI
             | UCSYNC           // 8:  synchronous mode enable
             | UCSSEL__ACLK     // 7-6: clock source select
             | 0;               // 0: release the UCA0 for operation
   UCA1IE = UCRXIE;             // RX interrupt enable, TX interrupt disable
}


#pragma FUNC_ALWAYS_INLINE(UCA1_getvalue)
GLOBAL UChar UCA1_getvalue(Void) {
   return uca1.ret;
}

GLOBAL Int UCA1_emit(const UInt adr, const UInt val, const UInt msg) {
   if (uca1.busy)
      return -1;
   uca1.busy  = TRUE;
   uca1.adr   = adr;
   uca1.val   = val;
   uca1.msg   = msg;
   SETBIT(UCA1IFG, UCRXIFG); //löst sofort interrupt aus und ruft state0 auf
   return 0;
}

LOCAL Int State0(Void) {
   CLRBIT(P2OUT, BIT3);   // Select aktivieren
   TB0CCR0 = DLY1;        // set up Compare Register
   SETBIT(TB0CTL, MC__UP | TBCLR | TBIE);
   uca1.state = State1;
   return 0;
}

LOCAL Int State1(Void) {
   UCA1TXBUF  = uca1.adr;
   uca1.state = State2;
   return 0;
}

LOCAL Int State2(Void) {
   TB0CCR0 = DLY2;        // set up Compare Register
   SETBIT(TB0CTL, MC__UP | TBCLR | TBIE);
   uca1.state = State3;
   return 0;
}

LOCAL Int State3(Void) {
   UCA1TXBUF  = uca1.val;
   uca1.state = State4;
   return 0;
}

LOCAL Int State4(Void) {
   TB0CCR0 = DLY3;        // set up Compare Register
   SETBIT(TB0CTL, MC__UP | TBCLR | TBIE);
   uca1.state = State5;
   return 0;
}

LOCAL Int State5(Void) {
   SETBIT(P2OUT, BIT3);   // Select deaktivieren
   Event_set(uca1.msg);
   uca1.state = State0;
   uca1.busy  = FALSE;
   return 1;
}

#pragma vector = USCI_A1_VECTOR
__interrupt Void USCI_A1(Void) {
   // Reading UCxRXBUF resets the receive error bits and UCRXIFG.
   uca1.ret = UCA1RXBUF;
   (*uca1.state)();
}

#pragma vector = TIMER0_B1_VECTOR
__interrupt Void TIMER0_B1(Void) {
   // clear interrupt flag and stop mode
   CLRBIT(TB0CTL, MC1 | MC0 | TBIFG | TBIE);
   if ((*uca1.state)()) {
      __low_power_mode_off_on_exit();
   }
}

