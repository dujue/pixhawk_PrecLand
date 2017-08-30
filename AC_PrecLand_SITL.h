#pragma once

#if CONFIG_HAL_BOARD == HAL_BOARD_SITL || CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_PXFMINI 
#include <AP_Common/AP_Common.h>
#include <AP_Math/AP_Math.h>
#include <AC_PrecLand/AC_PrecLand_Backend.h>

#include "AP_Longifly_PXFMini_VO.h"
/*
 * AC_PrecLand_SITL - supplies vectors to a fake landing target
 */

class AC_PrecLand_SITL : public AC_PrecLand_Backend
{
public:

    // Constructor
    AC_PrecLand_SITL(const AC_PrecLand& frontend, AC_PrecLand::precland_state& state);

    // perform any required initialisation of backend
    void init();

    // retrieve updates from sensor
    void update();

    // provides a unit vector towards the target in body frame
    //  returns same as have_los_meas()
    bool get_los_body(Vector3f& ret);

    bool get_real_height(float& real_height){return false;}
    
    // returns system time in milliseconds of last los measurement
    uint32_t los_meas_time_ms() { return _los_meas_time_ms; }

    // return true if there is a valid los measurement available
    bool have_los_meas();

    // parses a mavlink message from the companion computer
    void handle_msg(mavlink_message_t* msg) {};
private:

    Vector3f            _los_meas_body;         // unit vector in body frame pointing towards target
    uint32_t            _los_meas_time_ms;      // system time in milliseconds when los was measured
    bool                _have_los_meas;         // true if there is a valid measurement from the camera
    
    AP_Longifly_PXFMini_VO longifly_prec;
};

#endif