#ifndef _HW_H_
#define _HW_H_

#include <p18f13k22.h>
#include <delays.h>

/* TC77 HW config ----------------------------------------------------------- */

#define TC77_CS     PORTCbits.RC3

#define tc77_spi_write(x) \
    SSPBUF = x; \
    while (!SSPSTATbits.BF);

#define tc77_spi_buf    SSPBUF
#define TC77_SDI_TRIS   TRISBbits.TRISB4
#define TC77_SDI_PORT   PORTBbits.RB4

/* BK2421 HW config ----------------------------------------------------------*/

#define BK2421_CS   PORTCbits.RC5
#define BK2421_CE   PORTCbits.RC4
#define BK2421_IRQ  PORTAbits.RA2

#define bk2421_spi_write(x) \
    SSPBUF = x; \
    while (!SSPSTATbits.BF);

#define bk2421_spi_buf    SSPBUF

/* for delays used in bk2421 init */
#define bk2421_delay_ms(x) Delay1KTCYx(x); /* @ 1MHz */
#define bk2421_delay_us(x) Delay10TCYx((x+9)/10); /* @ 1MHz */

#endif /* _HW_H_ */
