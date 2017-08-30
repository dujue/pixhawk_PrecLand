/*
 * AC_PrecLand_PXFMini.cpp
 *
 *  Created on: Jan 19, 2017
 *      Author: liuxibo
 */

#pragma once

#if CONFIG_HAL_BOARD == HAL_BOARD_SITL || CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_PXFMINI || CONFIG_HAL_BOARD == HAL_BOARD_PX4
#include <AP_Common/AP_Common.h>
#include <AP_Math/AP_Math.h>
#include "AC_PrecLand_Backend.h"

#if CONFIG_HAL_BOARD == HAL_BOARD_SITL || CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_PXFMINI
#include "AP_Longifly_PXFMini_VO.h"
#endif

#if CONFIG_HAL_BOARD == HAL_BOARD_PX4
#include "AP_Longifly_PX4_VO.h"
#endif

/*
 * AC_PrecLand_PXFMini - supplies vectors to a fake landing target
 */

class AC_PrecLand_PXFMini : public AC_PrecLand_Backend
{
public:

    // Constructor
    AC_PrecLand_PXFMini(const AC_PrecLand& frontend, AC_PrecLand::precland_state& state);

    // perform any required initialisation of backend
    void init();

    // retrieve updates from sensor
    void update();

    // provides a unit vector towards the target in body frame
    //  returns same as have_los_meas()
    bool get_los_body(Vector3f& ret);

    bool get_real_height(float& real_height);
    // returns system time in milliseconds of last los measurement
    uint32_t los_meas_time_ms() { return _los_meas_time_ms; }

    // return true if there is a valid los measurement available
    bool have_los_meas();

    // parses a mavlink message from the companion computer
    void handle_msg(mavlink_message_t* msg) {};
private:

    
    bool                _have_los_meas;         // true if there is a valid measurement from the camera
    uint32_t            _los_meas_time_ms;      // system time in milliseconds when los was measured
    float               _real_height;

    #if CONFIG_HAL_BOARD == HAL_BOARD_SITL || CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_PXFMINI 
    AP_Longifly_PXFMini_VO longifly_prec;
    #endif

    #if  CONFIG_HAL_BOARD == HAL_BOARD_PX4
    AP_Longifly_PX4_VO longifly_prec;
    #endif
    Vector3f            _los_meas_body;         // unit vector in body frame pointing towards target
};

#endif