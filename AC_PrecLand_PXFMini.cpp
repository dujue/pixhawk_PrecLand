/*
 * AC_PrecLand_PXFMini.cpp
 *
 *  Created on: Jan 19, 2017
 *      Author: liuxibo
 */

#include <AP_HAL/AP_HAL.h>
#include "AC_PrecLand_PXFMini.h"

extern const AP_HAL::HAL& hal;

#if CONFIG_HAL_BOARD == HAL_BOARD_SITL || CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_PXFMINI || CONFIG_HAL_BOARD == HAL_BOARD_PX4

//#include <stdio.h> 
//#include <unistd.h> 
//#include <iostream> --canot use in pixhawk

// Constructor
AC_PrecLand_PXFMini::AC_PrecLand_PXFMini(const AC_PrecLand& frontend, AC_PrecLand::precland_state& state)
    : AC_PrecLand_Backend(frontend, state),
    _have_los_meas(false),
    _los_meas_time_ms(0),
    _real_height(0.0f),
    longifly_prec()
{
}

// init - perform initialisation of this backend
void AC_PrecLand_PXFMini::init()
{
    longifly_prec.init();

}

// update - give chance to driver to get updates from sensor
void AC_PrecLand_PXFMini::update()
{
    //hal.console->printf("AC_PrecLand_SITL::update()\n");
    const uint32_t now = AP_HAL::millis();
    if (_los_meas_time_ms + 10 > now) { // 100Hz update
        return;
    }

    bool new_data=longifly_prec.update();
   
    if(new_data)
    {
        _state.timestamp_index = longifly_prec.get_timestamp_index();
        //_state.healthy = true;
        // hal.console->printf("sitl recv data:%lu,%u,%f,%f,%f\n",
        // longifly_prec.position_pack.timestamp,longifly_prec.position_pack.targets_tag,
        // longifly_prec.position_pack.pos_x,longifly_prec.position_pack.pos_y,
        //     longifly_prec.position_pack.pos_z);
    }   
    else
    {
        //_state.timestamp_index = 0 ;
        //hal.console->printf("sitl:bad vision position\n");
        //_state.healthy = false;
        //return;
    }

    // hal.console->printf("longifly_prec.last_update_ms:%u,%u\n",longifly_prec.last_update_ms(),
    // _los_meas_time_ms);
    
    if (new_data && longifly_prec.last_update_ms() != _los_meas_time_ms) {
            

            longifly_prec.get_unit_vector_body(_los_meas_body);
            longifly_prec.get_real_height(_real_height);
            /*********for test*****
            // get new sensor data; we always point home
            Vector3f home;
            if (! _frontend._ahrs.get_relative_position_NED(home)) {
                _state.healthy = false;
                return;
            }
            if (home.length() > 10.0f) { // we can see the target out to 10 metres
                //return;
            }
            _state.healthy = true;

            //liuxibo
            home.x=-300;
            home.y=-400;

            hal.console->printf("set home....\n");
            const Matrix3f &body_to_ned = _frontend._ahrs.get_rotation_body_to_ned();
            
            _los_meas_body =  body_to_ned.mul_transpose(-home);
            _los_meas_body.x = -3.0f;
            _los_meas_body.y = -4.0f;
            _los_meas_body.z = 1.0f;
            _los_meas_body /= _los_meas_body.length();
            *********for test end***/

            _have_los_meas = true;
            _los_meas_time_ms = longifly_prec.last_update_ms();
        }
    
     //un update more than 3s
     bool delay_tag = AP_HAL::millis()-_los_meas_time_ms <= 3000 ;
     _have_los_meas = _have_los_meas && delay_tag ;
     if(delay_tag)
     {
        _state.healthy = true;
     }
     else
     {
        _state.healthy = false;
     }


   
}

bool AC_PrecLand_PXFMini::have_los_meas() {
    return _have_los_meas;
}


// provides a unit vector towards the target in body frame
//  returns same as have_los_meas()
bool AC_PrecLand_PXFMini::get_los_body(Vector3f& ret) {
    if (!_have_los_meas) {
        // no measurement for a full second; no vector available
        return false;
    }
    ret = _los_meas_body;
    //hal.console->printf("_los_meas_body:%f,%f,%f\n",_los_meas_body.x,_los_meas_body.y,_los_meas_body.z);

    return true;
}

bool AC_PrecLand_PXFMini::get_real_height(float& real_height){
    if (!_have_los_meas) {
        // no measurement for a full second; no vector available
        return false;
    }
    real_height = _real_height;
    return true;
}

#endif