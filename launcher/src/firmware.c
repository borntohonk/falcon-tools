/*
* Copyright (c) 2022 EliseZeroTwo <mail@elise.moe>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms and conditions of the GNU General Public License,
* version 2, as published by the Free Software Foundation.
*
* This program is distributed in the hope it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "fs.h"
#include "firmware.h"
#include "hwinit.h"
#include "postrun.h"

gfx_ctxt_t gfx_ctxt;
gfx_con_t gfx_con;

void firmware() {
    display_init();
    gfx_init_ctxt(display_init_framebuffer(), 720, 1280, 720);
    gfx_clear_color(BLACK);
    gfx_con_init();
    gfx_con_setcol(DEFAULT_TEXT_COL, 0, 0);
    display_backlight_pwm_init();
    display_backlight_brightness(100, 5000);
    clock_enable_uart(UART_B);
    uart_init(UART_B, BAUD_115200);

    print("UwU!\nPress a button to go\n");
    
    btn_wait();

    tsec_res_t *run = tsec_run(TSEC_ENTRYPOINT);

    print("Done\n");

    postrun(run);
    
    print("Press any button to power off\n");

    usleep(12e+5);
    btn_wait();
    i2c_send_byte(I2C_5, MAX77620_I2C_ADDR, MAX77620_REG_ONOFFCNFG1, MAX77620_ONOFFCNFG1_PWR_OFF);
}
