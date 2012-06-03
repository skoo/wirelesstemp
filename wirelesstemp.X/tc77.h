#ifndef _TC77_H_
#define _TC77_H_

#define TC77_SHUTDOWN 1
#define TC77_CONTCONV 0

#define TC77_NOT_READY 0x8000

void tc77_mode(unsigned char shutdown);
int tc77_read_temp(void);

#endif /* _TC77_H_ */
