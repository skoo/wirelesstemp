#include "hw.h"
#include <bk2421.h>
#include "tc77.h"

#define _id 0xc3

//#define PULSE_COUNTER

static volatile unsigned short _pulse_count = 0;
static volatile unsigned char _timer_int;

static const unsigned char _tx_addr[5] = {'s', 'k', '-', 'w', 0x00};
static const unsigned char _rx_addr[5] = {'s', 'k', '-', 'w', _id};

/* 18f+tc77+step-up pull 89uA @ 1.5V when in sleep mode
 * 
 */

static void main_delay(void);
static void timer_delay(void);
static void rf_send_temp(int t);

void high_isr(void);

#pragma code high_vector=0x08
void interrupt_at_high_vector(void)
{
  _asm GOTO high_isr _endasm
}

#pragma code

void main(void)
{
    int t;

    ANSEL = 0;  /* all ports digital */
    ANSELH = 0;

    /* set unused pins as outputs */
    TRISA = 0b00000100; // RA2: RF2400 IRQ
    TRISB = 0b00010000; // R45: SDI
    TRISC = 0b00000000;

#ifdef PULSE_COUNTER
    TRISBbits.TRISB5 = 1;
    IOCBbits.IOCB5 = 1;
    INTCONbits.RABIE = 1;
#endif

    TC77_CS = 1;
    BK2421_CS = 1;

    SSPSTATbits.SMP = 1;
    SSPSTATbits.CKE = 1;
    SSPCON1bits.CKP = 0;
    /* TC77 has max SPI clock of 7MHz, RF has max 8MHz clock. */
    /* Using FOSC / 4 = 16/4 = 4MHz */
    SSPCON1bits.SSPM1 = 0;
    SSPCON1bits.WCOL = 0;
    SSPCON1bits.SSPEN = 1;

    WDTCONbits.SWDTEN = 1; // watchdog is set to ~65.5s with post scaler

    T1CON = 0b10110000; // 1:8 prescaler

    PIE1bits.TMR1IE = 1;
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;

    bk2421_init(_rx_addr, _tx_addr);

    tc77_mode(TC77_CONTCONV);
    Delay1KTCYx(250);
    Delay1KTCYx(150);

    while (1) {
        int timeout = 100;
        do {
             Delay1KTCYx(1);
            t = tc77_read_temp();
        } while (t == TC77_NOT_READY && timeout-- > 0);

        bk2421_rf_power_mode(1);
        rf_send_temp(t);
        bk2421_rf_power_mode(0);

        tc77_mode(TC77_SHUTDOWN); /* 0.1uA when in shutdown mode */

        main_delay();

        /* Enable tc77 temperature conversion,
         * It takes 300.400ms for first conversion.
         * tc77 consumes 250uA in continuous conversion mode */
        tc77_mode(TC77_CONTCONV); 

        /* Set timer for ~524ms (1:8 tmr1 prescaler) */
        TMR1H = 0xc0;
        TMR1L = 0x00;
        T1CONbits.TMR1ON = 1;
        
        timer_delay();

        T1CONbits.TMR1ON = 0;
        PIR1bits.TMR1IF = 0;
    }
}

#ifdef PULSE_COUNTER

/*
 * System does not sleep when pulse counter is enabled as it must
 * keep counting pulses from RB5.
 */

void main_delay(void)
{
    unsigned char i;

    ClrWdt();

    /* set timer to 2048ms (1:8 tmr1 prescaler) */
    TMR1H = 0x00;
    TMR1L = 0x00;

    /* sleep for 30 seconds */
    for (i = 0; i < 15; i++) {
        _timer_int = 0;
        T1CONbits.TMR1ON = 1;
        while (_timer_int == 0);
        T1CONbits.TMR1ON = 0;
    }
}

void timer_delay(void)
{
    _timer_int = 0;
    while (_timer_int == 0);
}

#else
void main_delay(void)
{
    OSCCONbits.IDLEN = 0;
    Sleep(); /* WDT wakeup after ~65s */
}

void timer_delay(void)
{
    OSCCONbits.IDLEN = 1;
    Sleep(); /* Timer1 wakeup */
}
#endif

void rf_send_temp(int t)
{
    static unsigned char buf[5];

    buf[0] = t >> 8;
    buf[1] = t & 0xff;
    buf[2] = _id;

#ifdef PULSE_COUNTER
    INTCONbits.RABIE = 0;
    buf[3] = _pulse_count >> 8;
    buf[4] = _pulse_count & 0xff;
    INTCONbits.RABIE = 1;
    bk2421_send_packet(buf, 5);
#else
    bk2421_send_packet(buf, 3);
#endif
}

#pragma code
/*****************High priority ISR **************************/

#pragma interrupt high_isr
void high_isr (void)
{
    if (PIR1bits.TMR1IF) {
        _timer_int++;
        PIR1bits.TMR1IF = 0;
    }

    if (INTCONbits.RABIF) {
        if (PORTBbits.RB5 == 0) {
            _pulse_count++;
        }
        INTCONbits.RABIF = 0;
    }
}