#include <AP_HAL/AP_HAL.h>
#include "AC_PrecLand_SITL.h"

extern const AP_HAL::HAL& hal;

#if CONFIG_HAL_BOARD == HAL_BOARD_SITL || CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_PXFMINI 

#include <stdio.h>
#include <unistd.h>
#include <iostream>

// Constructor
AC_PrecLand_SITL::AC_PrecLand_SITL(const AC_PrecLand& frontend, AC_PrecLand::precland_state& state)
    : AC_PrecLand_Backend(frontend, state),
    longifly_prec(),
    _have_los_meas(false)
{
}

// init - perform initialisation of this backend
void AC_PrecLand_SITL::init()
{
    longifly_prec.init();

}

// update - give chance to driver to get updates from sensor
void AC_PrecLand_SITL::update()
{
    //hal.console->printf("AC_PrecLand_SITL::update()\n");
    const uint32_t now = AP_HAL::millis();
    if (_los_meas_time_ms + 10 > now) { // 100Hz update
        return;
    }

    /*********for test*****/
    // get new sensor data; we always point home
    Vector3f home;
    if (! _frontend._ahrs.get_relative_position_NED(home)) {
        _state.healthy = false;
        return;
    }
    if (home.length() > 10.0f) { // we can see the target out to 10 metres
        return;
    }
    _state.healthy = true;

    //liuxibo
    //home.x=-300;
    //home.y=-400;

    //hal.console->printf("set home....\n");
    const Matrix3f &body_to_ned = _frontend._ahrs.get_rotation_body_to_ned();

    _los_meas_body =  body_to_ned.mul_transpose(-home);
    //_los_meas_body.x = -3.0f;
    //_los_meas_body.y = -4.0f;
    //_los_meas_body.z = 1.0f;
    _los_meas_body /= _los_meas_body.length();
    /*********for test end***/

    _have_los_meas = true;
    _los_meas_time_ms = now;
    

     _have_los_meas = _have_los_meas && AP_HAL::millis()-_los_meas_time_ms <= 1000;

   
}

bool AC_PrecLand_SITL::have_los_meas() {
    return _have_los_meas;
}


// provides a unit vector towards the target in body frame
//  returns same as have_los_meas()
bool AC_PrecLand_SITL::get_los_body(Vector3f& ret) {
    if (!_have_los_meas) {
        // no measurement for a full second; no vector available
        return false;
    }
    ret = _los_meas_body;
    //hal.console->printf("_los_meas_body:%f,%f,%f\n",_los_meas_body.x,_los_meas_body.y,_los_meas_body.z);

    return true;
}

#endif