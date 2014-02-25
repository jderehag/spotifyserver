/*
 * clock.h
 *
 *  Created on: 24 Feb 2014
 *      Author: Jesse
 */

#ifndef CLOCK_H_
#define CLOCK_H_

#include <time.h>

#ifdef __cplusplus
extern "C"
{
#endif

void clockInit();
void clockTick();
time_t getTime();
void setTime( time_t newTime );

#ifdef __cplusplus
}
#endif

#endif /* CLOCK_H_ */
