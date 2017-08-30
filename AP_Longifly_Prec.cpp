/*
 * AP_Longifly_Prec.cpp
 *
 *  Created on: Jan 19, 2017
 *      Author: liuxibo
 */

#include "AP_Longifly_Prec.h"

// default constructor
AP_Longifly_Prec::AP_Longifly_Prec() :
    _last_update_ms(0),
    _last_timestamp(0)
{
    position_pack.timestamp = 0;
    position_pack.targets_tag = 0;
    position_pack.pos_x = 0.0f;
    position_pack.pos_y = 0.0f;
    position_pack.pos_z = 0.0f;

    // will be adjusted when init is called
    _flags.healthy = false;
}

AP_Longifly_Prec::~AP_Longifly_Prec() {}

// retrieve body frame x and y angles (in radians) to target
// returns true if data is available
bool AP_Longifly_Prec::get_angle_to_target_rad(float &x_angle_rad, float &y_angle_rad) const
{
    // return false if we have no target
    if (position_pack.targets_tag == 0) {
        return false;
    }

    // use data from first (largest) object
    x_angle_rad = atanf(position_pack.pos_x);
    y_angle_rad = atanf(position_pack.pos_y);
    return true;
}


// retrieve body frame unit vector in direction of target
// returns true if data is available
bool AP_Longifly_Prec::get_unit_vector_body(Vector3f& ret) const
{
    // return false if we have no target
    if (position_pack.targets_tag == 0) {
        return false;
    }

    if(position_pack.pos_z == 0){
        return false;
    }

    // use data from first (largest) object
    //ret.x = -position_pack.pos_y;//why????? liuxibo
    //ret.y = position_pack.pos_x;
    ret.x = -position_pack.pos_y;//cm
    ret.y = position_pack.pos_x;//cm
    ret.z = position_pack.pos_z;
    // ret.x = -position_pack.pos_y/210*21;//cm
    // ret.y = position_pack.pos_x/210*21;//cm
    // ret.z = 100.0f;
    ret /= ret.length();
    return true;
}

// retrieve body frame unit vector in direction of target
// returns true if data is available
bool AP_Longifly_Prec::get_real_height(float& real_height) const
{
    // return false if we have no target
    if (position_pack.targets_tag == 0) {
        return false;
    }

    if(position_pack.pos_z == 0){
        return false;
    }

    real_height = position_pack.pos_z;
    if(position_pack.pos_z >10*100.0)
    {
        real_height = 5*100.0;
    }
    // use data from first (largest) object
    //ret.x = -position_pack.pos_y;//why????? liuxibo
    //ret.y = position_pack.pos_x;
    
    return true;
}
