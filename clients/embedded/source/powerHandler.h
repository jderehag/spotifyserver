
#ifndef POWERHANDLER_H_
#define POWERHANDLER_H_

#ifdef __cplusplus
extern "C"
{
#endif

void pwrInit();
void pwrKeepAlive();
int pwrIsAlive();
void pwrCanPowerOff( int enabled );

#ifdef __cplusplus
}
#endif

#endif /* POWERHANDLER_H_ */
