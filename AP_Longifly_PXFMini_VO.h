/*
 * AP_Longifly_PXFMini_VO.h
 *
 *  Created on: Jan 19, 2017
 *      Author: liuxibo
 */
#pragma once

#include "AP_Longifly_Prec.h"
#include <AP_HAL/utility/Socket.h>
#include <stdio.h>

#if CONFIG_HAL_BOARD == HAL_BOARD_SITL
#define TEST_LONGIFLY_VO_LOG_PATH "vo_data.txt"
#endif

#if CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_PXFMINI
#define TEST_LONGIFLY_VO_LOG_PATH "/root/vo_data.txt"
#endif


class AC_PrecLand_SITL;
class AC_PrecLand_PXFMini;

class AP_Longifly_PXFMini_VO : public AP_Longifly_Prec
{

friend class AC_PrecLand_SITL;
friend class AC_PrecLand_PXFMini;

public:
    AP_Longifly_PXFMini_VO();

    // init - initialize sensor library
    virtual void init();

    // retrieve latest sensor data - returns true if new data is available
    virtual bool update();

private:
    uint16_t     _port;
    SocketAPM _sock;
    bool _fileLog;
    FILE *_logf;
};
