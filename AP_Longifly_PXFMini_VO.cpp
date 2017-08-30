/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * AP_Longifly_PXFMini_VO.cpp
 *
 *  Created on: Jan 19, 2017
 *      Author: liuxibo
 */
#include <AP_HAL/AP_HAL.h>

#if CONFIG_HAL_BOARD == HAL_BOARD_SITL || CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_PXFMINI
#include "AP_Longifly_PXFMini_VO.h"

#include <fcntl.h>
#include <unistd.h>

extern const AP_HAL::HAL& hal;

AP_Longifly_PXFMini_VO::AP_Longifly_PXFMini_VO() :
    _port(0),
    _sock(true),
    _fileLog(false),
    _logf(NULL)
{}


void AP_Longifly_PXFMini_VO::init()
{
     _port = 9999;
    if(!_sock.bind("0.0.0.0", _port)) {
        hal.console->printf("failed to bind UDP socket\n");
    }

    _sock.set_blocking(false);
    hal.console->printf("AP_Longifly_PXFMini_VO::bind udp port 9999\n");

    _flags.healthy = true;

    //init file log
    _fileLog = true;
    if(_fileLog) {
        if (_logf == NULL) {
            _logf = fopen(TEST_LONGIFLY_VO_LOG_PATH, "a+");
            char init_to_write[256]={0};
            sprintf(init_to_write,"------sitl init a new fly------\n");
            fwrite(init_to_write, sizeof(char), strlen(init_to_write), _logf);
            fflush(_logf);
        }
    }
    //fclose (_logf);
}

// retrieve latest sensor data - returns true if new data is available
bool AP_Longifly_PXFMini_VO::update()
{
    // return immediately if not healthy
    if (!_flags.healthy) {
        return false;
    }


    bool new_data = false;
    // read position of all objects
    ssize_t size;
    position_packet pwm_pkt;

    //linux is diff whit windows
    //in windows:if buffer size < data size ,return -1,and set error WSAEMSGSIZE
    //in linux:return buffer size
    size = _sock.recv(&pwm_pkt, sizeof(pwm_pkt), 0);

    if(size>0)
    {
        if(pwm_pkt.targets_tag != 0 && pwm_pkt.timestamp!= _last_timestamp)
        {
            new_data = true;
            _last_timestamp = pwm_pkt.timestamp;
            _last_timestamp = 0;
        }
        else
        {
            pwm_pkt.targets_tag = 0;
        }

        //for test
        //position_pack.targets_tag = 0;
        if(pwm_pkt.targets_tag != 0)
        {
            hal.console->printf("udp port recv size:%u\n",size);
            hal.console->printf("udp port recv data:%llu,%u,%f,%f,%f\n",pwm_pkt.timestamp,
                pwm_pkt.targets_tag,
                pwm_pkt.pos_x,
                pwm_pkt.pos_y,
            pwm_pkt.pos_z);

            if (_logf != NULL) {

                //char size_to_write[256]={0};
                //sprintf(size_to_write,"udp port recv size:%u\n",size);
                //::fwrite(size_to_write, sizeof(char), strlen(size_to_write), _logf);

                char data_to_write[256]={0};
                sprintf(data_to_write,"udp port recv data:%llu,%u,%f,%f,%f\n",
                    pwm_pkt.timestamp,
                    pwm_pkt.targets_tag,
                    pwm_pkt.pos_x,
                    pwm_pkt.pos_y,
                    pwm_pkt.pos_z);
                ::fwrite(data_to_write, sizeof(char), strlen(data_to_write), _logf);

                fflush(_logf);
            }
        }
        
        
        

        position_pack.pos_x = pwm_pkt.pos_x;
        position_pack.pos_y = -pwm_pkt.pos_y;
        position_pack.pos_z = pwm_pkt.pos_z;
        position_pack.timestamp = pwm_pkt.timestamp;
        position_pack.targets_tag = pwm_pkt.targets_tag;
        _last_update_ms = AP_HAL::millis();
    }

    // return true if new data found
    return new_data;
}

#endif //CONFIG_HAL_BOARD == HAL_BOARD_SITL || CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_PXFMINI
