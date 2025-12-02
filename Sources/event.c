#include <msp430.h>
#include "event.h"

LOCAL volatile struct {
   TEvent nxt;
   TEvent act;
   TEvent err;
} evn;

#pragma FUNC_ALWAYS_INLINE(Event_init)
GLOBAL Void Event_init(Void) {
   CLRBIT(evn.nxt, ALL_EVENTS);
   CLRBIT(evn.act, ALL_EVENTS);
   CLRBIT(evn.err, ALL_EVENTS);
}

#pragma FUNC_ALWAYS_INLINE(Event_wait)
GLOBAL Void Event_wait(Void) {
   _disable_interrupt();
   SETBIT(evn.err, evn.act BAND evn.nxt);
   SETBIT(evn.act, evn.nxt);
   CLRBIT(evn.nxt, ALL_EVENTS);
   if (evn.act EQ NO_EVENTS) {
      CLRBIT(P3OUT, BIT7);
      _low_power_mode_3();
      SETBIT(P3OUT, BIT7);
   }
   _enable_interrupt();
}

#pragma FUNC_ALWAYS_INLINE(Event_set)
GLOBAL Void Event_set(TEvent arg) {
   SETBIT(evn.nxt, arg);
}

#pragma FUNC_ALWAYS_INLINE(Event_clr)
GLOBAL Void Event_clr(TEvent arg) {
   CLRBIT(evn.act, arg);
}

#pragma FUNC_ALWAYS_INLINE(Event_tst)
GLOBAL Bool Event_tst(TEvent arg) {
   return TSTBIT(evn.act, arg);
}

#pragma FUNC_ALWAYS_INLINE(Event_err)
GLOBAL Bool Event_err(Void) {
   return (evn.err NE NO_EVENTS);
}
