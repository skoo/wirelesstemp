#include "hw.h"

#pragma config FOSC = IRC
#pragma config PLLEN = OFF
#pragma config PCLKEN = ON
#pragma config FCMEN = OFF
#pragma config IESO = OFF
#pragma config BOREN = OFF
#pragma config HFOFST = ON
#pragma config STVREN = ON
#pragma config WRTD = OFF
#pragma config LVP = OFF
#pragma config XINST = OFF
#pragma config WDTPS = 16384 /* 16384*4ms = 65.536s */
//#pragma config WDTPS = 4096 /* 16s */
//#pragma config WDTPS = 1024 /* 4s */
#pragma config WDTEN = ON
#pragma config MCLRE = OFF
