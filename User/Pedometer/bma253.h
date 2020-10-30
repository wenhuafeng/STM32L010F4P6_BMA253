/**************************************************************************
* Copytight 2017 Mandridy Tech.Co., Ltd. All rights reserved              *
*                                                                         *
* Filename : BMA253.h                                                     *
* Author : Wen Shifang                                                    *
* Version : 1.0                                                           *
*                                                                         *
* Decription : BMA GSensor Init                                           *
*               header file for BMA253.c                                  *
* Function-list:                                                          *
* History:                                                                *
*       <author>     <time>     <version>  <desc>                         *
*       Wen Shifang  2017-4.6   1.0        build this moudle              *
*                                                                         *
*                                                                         *
* Created : 2017-4-6                                                      *
* Last modified : 2017.4.6                                                *
**************************************************************************/

#ifndef __BMA253_H__
#define __BMA253_H__

#include "bma2x2.h"
#include "TypeDefine.h"
#include "main.h"

#define FIFO_DEPTH         16

#if ( (FIFO_DEPTH != 20) && ( FIFO_DEPTH != 16 ) && (FIFO_DEPTH != 31))
#error "FIFO_DEPTH Error"
#endif

extern ErrorStatus bma253_init(void);
extern void bma2x2_standy(void);
extern void make_bma250_to_deep_sleep(FlagStatus enable);

#endif
