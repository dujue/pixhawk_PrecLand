/// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
#include <AP_HAL/AP_HAL.h>
#include "AC_PrecLand.h"
#include "AC_PrecLand_Backend.h"
#include "AC_PrecLand_Companion.h"
#include "AC_PrecLand_IRLock.h"
#include "AC_PrecLand_SITL.h"
#include "AC_PrecLand_PXFMini.h"

extern const AP_HAL::HAL& hal;

const AP_Param::GroupInfo AC_PrecLand::var_info[] = {
    // @Param: ENABLED
    // @DisplayName: Precision Land enabled/disabled and behaviour
    // @Description: Precision Land enabled/disabled and behaviour
    // @Values: 0:Disabled, 1:Enabled Always Land, 2:Enabled Strict
    // @User: Advanced
    AP_GROUPINFO_FLAGS("ENABLED", 0, AC_PrecLand, _enabled, 0, AP_PARAM_FLAG_ENABLE),

    // @Param: TYPE
    // @DisplayName: Precision Land Type
    // @Description: Precision Land Type
    // @Values: 0:None, 1:CompanionComputer, 2:IRLock
    // @User: Advanced
    AP_GROUPINFO("TYPE",    1, AC_PrecLand, _type, 0),

    AP_GROUPEND
};

// Default constructor.
// Note that the Vector/Matrix constructors already implicitly zero
// their values.
//
AC_PrecLand::AC_PrecLand(const AP_AHRS& ahrs, const AP_InertialNav& inav) :
    _ahrs(ahrs),
    _inav(inav),
    _last_update_ms(0),
    _last_backend_los_meas_ms(0),
    _backend(NULL)
{
    // set parameters to defaults
    AP_Param::setup_object_defaults(this, var_info);

    // other initialisation
    _backend_state.healthy = false;
    _backend_state.timestamp_index = 0;
}


// init - perform any required initialisation of backends
void AC_PrecLand::init()
{
    // exit immediately if init has already been run
    if (_backend != NULL) {
        return;
    }

    // default health to false
    _backend = NULL;
    _backend_state.healthy = false;
    _backend_state.timestamp_index = 0;

    //_enabled = true;
    //_type = 6;

    // instantiate backend based on type parameter
    switch ((enum PrecLandType)(_type.get())) {
        // no type defined
        case PRECLAND_TYPE_NONE:
        default:
            return;
        // companion computer
        case PRECLAND_TYPE_COMPANION:
            _backend = new AC_PrecLand_Companion(*this, _backend_state);
            break;
        // IR Lock
#if CONFIG_HAL_BOARD == HAL_BOARD_PX4 || CONFIG_HAL_BOARD == HAL_BOARD_VRBRAIN
        case PRECLAND_TYPE_IRLOCK:
            _backend = new AC_PrecLand_IRLock(*this, _backend_state);
            break;
#endif
#if CONFIG_HAL_BOARD == HAL_BOARD_SITL
        case PRECLAND_TYPE_SITL_GAZEBO:
            //_backend = new AC_PrecLand_SITL_Gazebo(*this, _backend_state);
            break;
        case PRECLAND_TYPE_SITL:
            _backend = new AC_PrecLand_SITL(*this, _backend_state);
            break;
#endif

#if CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_PXFMINI || CONFIG_HAL_BOARD == HAL_BOARD_SITL || CONFIG_HAL_BOARD == HAL_BOARD_PX4 
        case PRECLAND_TYPE_RELATIVE_GPS:
            //_backend = new AC_PrecLand_SITL_Gazebo(*this, _backend_state);
            break;
        case PRECLAND_TYPE_VO:
            _backend = new AC_PrecLand_PXFMini(*this, _backend_state);
            break;
#endif
    }

    // init backend
    if (_backend != NULL) {
        _backend->init();
    }
}

// update - give chance to driver to get updates from sensor
void AC_PrecLand::update(float alt_above_terrain_cm)
{

    _attitude_history.push_back(_ahrs.get_rotation_body_to_ned());
    //hal.console->printf("AC_PrecLand::update:%f\n",alt_above_terrain_cm);
    
    // run backend update
    if (_backend != NULL && _enabled) {
        // read from sensor
        _backend->update();
        
        // Vector3f vehicleVelocityNED = _inav.get_velocity()*0.01f;
        // vehicleVelocityNED.z = -vehicleVelocityNED.z;


        if (_backend->have_los_meas() && _backend->los_meas_time_ms() != _last_backend_los_meas_ms) {
            // we have a new, unique los measurement
            _last_backend_los_meas_ms = _backend->los_meas_time_ms();

            Vector3f target_vec_unit_body;
            _backend->get_los_body(target_vec_unit_body);
            float real_height;
            _backend->get_real_height(real_height);
            //calc_angles_and_pos(target_vec_unit_body, alt_above_terrain_cm);
            calc_angles_and_pos(target_vec_unit_body, real_height);
        }
    }
    else
    {
        //hal.console->println("none _backend");
    }
}

bool AC_PrecLand::target_acquired()
{
    return (AP_HAL::millis()-_last_update_ms) < 3000;
}

bool AC_PrecLand::get_target_position(Vector3f& ret)
{
    if (!target_acquired()) {
        return false;
    }

    ret = _target_pos;
    return true;
}

bool AC_PrecLand::get_target_position_relative(Vector3f& ret)
{
    if (!target_acquired()) {
        return false;
    }

    ret = _target_pos_rel;
    return true;
}

bool AC_PrecLand::get_target_velocity_relative(Vector3f& ret)
{
    return false;
}

// converts sensor's body-frame angles to earth-frame angles and position estimate
//  raw sensor angles stored in _angle_to_target (might be in earth frame, or maybe body frame)
//  earth-frame angles stored in _ef_angle_to_target
//  position estimate is stored in _target_pos
void AC_PrecLand::calc_angles_and_pos(const Vector3f& target_vec_unit_body, float alt_above_terrain_cm)
{

    // Apply sensor yaw alignment rotation
    float sin_yaw_align = sinf(radians(0*0.01f));
    float cos_yaw_align = cosf(radians(0*0.01f));
    Matrix3f Rz = Matrix3f(
        cos_yaw_align, -sin_yaw_align, 0,
        sin_yaw_align, cos_yaw_align, 0,
        0, 0, 1
    );

    Vector3f target_vec_unit_ned = _attitude_history.front() * Rz * target_vec_unit_body;
    // rotate into NED frame
    //Vector3f target_vec_unit_ned = _ahrs.get_rotation_body_to_ned()*target_vec_unit_body;

    //hal.console->printf("target_vec_unit_ned:%f,%f,%f\n",target_vec_unit_ned.x,target_vec_unit_ned.y,target_vec_unit_ned.z);
    // extract the angles to target (logging only)
    _angle_to_target.x = atan2f(-target_vec_unit_body.y, target_vec_unit_body.z);
    _angle_to_target.y = atan2f( target_vec_unit_body.x, target_vec_unit_body.z);
    _ef_angle_to_target.x = atan2f(-target_vec_unit_ned.y, target_vec_unit_ned.z);
    _ef_angle_to_target.y = atan2f( target_vec_unit_ned.x, target_vec_unit_ned.z);

    if (target_vec_unit_ned.z > 0.0f) {

        // get current altitude (constrained to be positive)
        float alt = MAX(alt_above_terrain_cm, 0.0f);
        float dist = alt/target_vec_unit_ned.z;
        _target_pos_rel.x = target_vec_unit_ned.x*dist;
        _target_pos_rel.y = target_vec_unit_ned.y*dist;
        _target_pos_rel.z = alt;  // not used

        dist = alt/target_vec_unit_body.z;
        _target_pos_rel.x = target_vec_unit_body.x*dist;
        _target_pos_rel.y = target_vec_unit_body.y*dist;
        hal.console->printf("_target_pos_rel:%f,%f\n",_target_pos_rel.x,_target_pos_rel.y);
        hal.console->printf("target_vec_unit_ned:%f,%f,%f\n",target_vec_unit_ned.x,target_vec_unit_ned.y,target_vec_unit_ned.z);
        _target_pos = _inav.get_position()+_target_pos_rel;

        _last_update_ms = AP_HAL::millis();
    }
}

// handle_msg - Process a LANDING_TARGET mavlink message
void AC_PrecLand::handle_msg(mavlink_message_t* msg)
{
    // run backend update
    if (_backend != NULL) {
        _backend->handle_msg(msg);
    }
}
