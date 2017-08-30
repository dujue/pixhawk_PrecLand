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

#if  CONFIG_HAL_BOARD == HAL_BOARD_PX4
#include "AP_Longifly_PX4_VO.h"

#include <fcntl.h>
#include <unistd.h>

extern const AP_HAL::HAL& hal;

AP_Longifly_PX4_VO::AP_Longifly_PX4_VO() :
    _fileLog(false),
    //_logf(NULL),
    _port(NULL),
    pwm_pkt({0})
{}


ssize_t total_size;
ssize_t need_size;
bool _search_for_head;
bool _search_for_end;
//struct position_packet pwm_pkt;

void AP_Longifly_PX4_VO::init()
{
    total_size = 32;
    need_size = total_size;
    _search_for_head = 1;
    _search_for_end = 0;
    //init hal.uartE
    _port = hal.uartE;
    _port->printf("liuxibo:init %s success\n","uartE");

    hal.console->printf("AP_Longifly_PX4_VO::init\n");

    _flags.healthy = true;

    //init file log
    // _fileLog = true;
    // if(_fileLog) {
    //     if (_logf == NULL) {
    //         _logf = fopen(TEST_LONGIFLY_VO_LOG_PATH, "a+");
    //         char init_to_write[256]={0};
    //         sprintf(init_to_write,"------init a new fly------\n");
    //         fwrite(init_to_write, sizeof(char), strlen(init_to_write), _logf);
    //         fflush(_logf);
    //     }
    // }

    //fclose (_logf);
}

// retrieve latest sensor data - returns true if new data is available
bool AP_Longifly_PX4_VO::update()
{
    // return immediately if not healthy
    if (!_flags.healthy) {
        return false;
    }

    //_port->printf("liuxibo:update %s success\n","uartE");
    //hal.console->printf("liuxibo:update %s success\n","uartE");

    bool new_data = false;
    // read position of all objects
    

    int16_t numc;
    numc = _port->available();
    

    while(numc > 0)
    {
        //search first char 0xff 0xff 0xff 0xff
        if(_search_for_head)
        {
            char data = _port->read();
            numc--;
            if(data == 0xff)
            {
                _search_for_head = 0;
                need_size = total_size ;
                pwm_pkt.timestamp = 0;
                pwm_pkt.targets_tag = 0;
                pwm_pkt.pos_x = 0.0f;
                pwm_pkt.pos_y = 0.0f;
                pwm_pkt.pos_z = 0.0f;
                pwm_pkt.size_width = 0.0f;
                pwm_pkt.size_height = 0.0f;
            }
        }

        if(_search_for_end)
        {
            char data = _port->read();
            numc--;
            _search_for_head = 1;
            _search_for_end = 0;   
            if(data == 0x00)
            {

                _port->printf("numc:%d\n",numc);
                // hal.console->printf("udp port recv data:%llu,%u,%f,%f,%f,%f,%f\n",
                //             pwm_pkt.timestamp,
                //             pwm_pkt.targets_tag,
                //             pwm_pkt.size_width,
                //             pwm_pkt.size_height,
                //             pwm_pkt.pos_x,
                //             pwm_pkt.pos_y,
                //             pwm_pkt.pos_z
                //             );

                 _port->printf("uartE recv data:%llu,%u,%f,%f,%f,%f,%f\n",
                            pwm_pkt.timestamp,
                            pwm_pkt.targets_tag,
                            pwm_pkt.size_width,
                            pwm_pkt.size_height,
                            pwm_pkt.pos_x,
                            pwm_pkt.pos_y,
                            pwm_pkt.pos_z
                            );

                if(pwm_pkt.targets_tag != 0 && pwm_pkt.timestamp!= _last_timestamp)
                {
                    new_data = true;
                    _last_timestamp = pwm_pkt.timestamp;
                }
                else
                {
                    pwm_pkt.targets_tag = 0;
                }

                if(pwm_pkt.targets_tag != 0)
                {
                    /*
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
                    */
                }

                position_pack.pos_x = pwm_pkt.pos_x;
                position_pack.pos_y = -pwm_pkt.pos_y;
                position_pack.pos_z = pwm_pkt.pos_z;
                position_pack.timestamp = pwm_pkt.timestamp;
                position_pack.targets_tag = pwm_pkt.targets_tag;
                _last_update_ms = AP_HAL::millis();

                return new_data;

            }
       
        }

        if((!_search_for_head) && (!_search_for_end))
        {
            while(need_size > 0 && numc > 0)
            {
                //size = read(port->_get_fd() , (char*)&pwm_pkt + (total_size - need_size), need_size );
                char data = _port->read();
                //_port->printf("data:%d\n",data);
                memcpy((char*)&pwm_pkt + (total_size - need_size), &data, 1);
                need_size = need_size - 1;
                numc--;
            }

            if(need_size == 0)
            {
                //search for end
                _search_for_end = 1;

            }


        }
        
        

    }
        
        
        
        


    // return true if new data found
    return new_data;
}

#endif //CONFIG_HAL_BOARD == HAL_BOARD_PX4
