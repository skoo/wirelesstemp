#include "tc77.h"
#include "hw.h"

void tc77_mode(unsigned char shutdown)
{
    TC77_CS = 0;

    tc77_spi_write(0);
    tc77_spi_write(0);

    /* write 16 bits of '1' for shutdown mode, '0' for cont conv mode */
    TC77_SDI_TRIS = 0;
    TC77_SDI_PORT = shutdown;

    tc77_spi_write(0);
    tc77_spi_write(0);

    TC77_CS = 1;
    TC77_SDI_TRIS = 1;
}

/*
 * Return temperature in 0.0625 celsius degrees,
 * multiply returned value by 0.0625 to get the temperature in c deg.
 */
int tc77_read_temp(void)
{
    unsigned int t;
    unsigned char l;

    TC77_CS = 0;

    tc77_spi_write(0);
    t = tc77_spi_buf;

    tc77_spi_write(0);
    l = tc77_spi_buf;

    if (!(l & 4)) {
        /* conversion not done after shutdown -> cont conv mode change */
        TC77_CS = 1;
        return TC77_NOT_READY;
    }

    t <<= 5;
    t |= (l >> 3);

    if (t & 0x1000)
        t |= 0xe000;

    TC77_CS = 1;

    return (int)t;
}
