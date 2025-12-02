#ifndef UCA1_H_
#define UCA1_H_

#include "..\base.h"

#define DRVWREN   (0x0008) // write enable
#define DRVRDEN   (0x0000) // read enable
#define DRVENABLE (0x0002) // display enable

// register address map
#define DRVDIG_0  (0x0000) // Digit 0 register
#define DRVDIG_1  (0x0001) // Digit 1 register
#define DRVDIG_2  (0x0002) // Digit 2 register
#define DRVDIG_3  (0x0003) // Digit 3 register
#define DRVCTL    (0x0004) // control register

EXTERN inline Void UCA1_init(Void);
EXTERN Int UCA1_emit(const UInt adr, const UInt val, const UInt msg);

#endif /* UCA1_H_ */
