/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * Development of the code in this file was sponsored by Microbric Pty Ltd
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Paul Sokolovsky
 * Copyright (c) 2016 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <math.h>

#if !MICROPY_ESP_IDF_4
#include "rom/gpio.h"
#endif
#include "esp_log.h"
#include "esp_spi_flash.h"

#include "py/runtime.h"
#include "py/mperrno.h"
#include "py/mphal.h"
#include "drivers/dht/dht.h"
#include "modesp.h"

STATIC mp_obj_t esp_osdebug(size_t n_args, const mp_obj_t *args) {
    esp_log_level_t level = LOG_LOCAL_LEVEL;
    if (n_args == 2) {
        level = mp_obj_get_int(args[1]);
    }
    if (args[0] == mp_const_none) {
        // Disable logging
        esp_log_level_set("*", ESP_LOG_ERROR);
    } else {
        // Enable logging at the given level
        // TODO args[0] should set the UART to which debug is sent
        esp_log_level_set("*", level);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(esp_osdebug_obj, 1, 2, esp_osdebug);

STATIC mp_obj_t esp_flash_read(mp_obj_t offset_in, mp_obj_t buf_in) {
    mp_int_t offset = mp_obj_get_int(offset_in);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf_in, &bufinfo, MP_BUFFER_WRITE);
    esp_err_t res = spi_flash_read(offset, bufinfo.buf, bufinfo.len);
    if (res != ESP_OK) {
        mp_raise_OSError(MP_EIO);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(esp_flash_read_obj, esp_flash_read);

STATIC mp_obj_t esp_flash_write(mp_obj_t offset_in, mp_obj_t buf_in) {
    mp_int_t offset = mp_obj_get_int(offset_in);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf_in, &bufinfo, MP_BUFFER_READ);
    esp_err_t res = spi_flash_write(offset, bufinfo.buf, bufinfo.len);
    if (res != ESP_OK) {
        mp_raise_OSError(MP_EIO);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(esp_flash_write_obj, esp_flash_write);

STATIC mp_obj_t esp_flash_erase(mp_obj_t sector_in) {
    mp_int_t sector = mp_obj_get_int(sector_in);
    esp_err_t res = spi_flash_erase_sector(sector);
    if (res != ESP_OK) {
        mp_raise_OSError(MP_EIO);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(esp_flash_erase_obj, esp_flash_erase);

STATIC mp_obj_t esp_flash_size(void) {
    return mp_obj_new_int_from_uint(spi_flash_get_chip_size());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(esp_flash_size_obj, esp_flash_size);

STATIC mp_obj_t esp_flash_user_start(void) {
    return MP_OBJ_NEW_SMALL_INT(0x200000);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(esp_flash_user_start_obj, esp_flash_user_start);

STATIC mp_obj_t esp_gpio_matrix_in(mp_obj_t pin, mp_obj_t sig, mp_obj_t inv) {
    gpio_matrix_in(mp_obj_get_int(pin), mp_obj_get_int(sig), mp_obj_get_int(inv));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(esp_gpio_matrix_in_obj, esp_gpio_matrix_in);

STATIC mp_obj_t esp_gpio_matrix_out(size_t n_args, const mp_obj_t *args) {
    (void)n_args;
    gpio_matrix_out(mp_obj_get_int(args[0]), mp_obj_get_int(args[1]), mp_obj_get_int(args[2]), mp_obj_get_int(args[3]));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(esp_gpio_matrix_out_obj, 4, 4, esp_gpio_matrix_out);


float IRAM_ATTR many_hypots() {
    float x = 1.1;
    float y = 1.0001;
    float h;
    for (int i = 0; i < 1000; i++) {
        h = hypot(x,y);
        h = hypot(h,y);
        h = hypot(h,y);
        h = hypot(h,y);
        h = hypot(h,y);
        h = hypot(h,y);
        h = hypot(h,y);
        h = hypot(h,y);
        h = hypot(h,y);
        x = hypot(h,y);
    }
    return h;
}
STATIC mp_obj_t many_hypots_() {
    return mp_obj_new_float(many_hypots());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(many_hypots_obj, many_hypots_);

STATIC mp_obj_t esp_led_on_() {
    GPIO_REG_WRITE(GPIO_OUT_W1TS_REG, 4);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(esp_led_on_obj, esp_led_on_);

STATIC mp_obj_t esp_led_off_() {
    for (int i = 0; i < 10000; i++) {
        GPIO_REG_WRITE(GPIO_OUT_W1TC_REG, 4);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(esp_led_off_obj, esp_led_off_);

STATIC mp_obj_t esp_pin_set_(mp_obj_t pin_mask) {
    esp_pin_set(mp_obj_get_int(pin_mask));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(esp_pin_set_obj, esp_pin_set_);


STATIC mp_obj_t esp_pin_clear_(mp_obj_t pin_mask) {
    esp_pin_clear(mp_obj_get_int(pin_mask));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(esp_pin_clear_obj, esp_pin_clear_);

STATIC mp_obj_t esp_neopixel_write_(mp_obj_t pin, mp_obj_t buf, mp_obj_t timing) {
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf, &bufinfo, MP_BUFFER_READ);
    return mp_obj_new_int_from_uint(esp_neopixel_write(mp_hal_get_pin_obj(pin),
        (uint8_t*)bufinfo.buf, bufinfo.len, mp_obj_get_int(timing)));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(esp_neopixel_write_obj, esp_neopixel_write_);

STATIC mp_obj_t esp_neopixel_write_timings_(size_t n_args, const mp_obj_t *args) {
    (void)n_args;
    mp_obj_t pin = args[0], buf = args[1], t0high = args[2], t1high = args[3], t0low = args[4], t1low = args[5], t_latch = args[6];
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf, &bufinfo, MP_BUFFER_READ);
    return mp_obj_new_int_from_uint(
        esp_neopixel_write_timings(
            mp_hal_get_pin_obj(pin),
            (uint8_t*)bufinfo.buf, 
            bufinfo.len, 
            mp_obj_get_int(t0high), 
            mp_obj_get_int(t1high), 
            mp_obj_get_int(t0low), 
            mp_obj_get_int(t1low), 
            mp_obj_get_int(t_latch)
        )
    );
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(esp_neopixel_write_timings_obj, 7, 7, esp_neopixel_write_timings_);

STATIC const mp_rom_map_elem_t esp_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_esp) },

    { MP_ROM_QSTR(MP_QSTR_osdebug), MP_ROM_PTR(&esp_osdebug_obj) },

    { MP_ROM_QSTR(MP_QSTR_flash_read), MP_ROM_PTR(&esp_flash_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_flash_write), MP_ROM_PTR(&esp_flash_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_flash_erase), MP_ROM_PTR(&esp_flash_erase_obj) },
    { MP_ROM_QSTR(MP_QSTR_flash_size), MP_ROM_PTR(&esp_flash_size_obj) },
    { MP_ROM_QSTR(MP_QSTR_flash_user_start), MP_ROM_PTR(&esp_flash_user_start_obj) },

    { MP_ROM_QSTR(MP_QSTR_gpio_matrix_in), MP_ROM_PTR(&esp_gpio_matrix_in_obj) },
    { MP_ROM_QSTR(MP_QSTR_gpio_matrix_out), MP_ROM_PTR(&esp_gpio_matrix_out_obj) },

    { MP_ROM_QSTR(MP_QSTR_many_hypots), MP_ROM_PTR(&many_hypots_obj) },
    // { MP_ROM_QSTR(MP_QSTR_led_onh), MP_ROM_PTR(&many_hypots_obj) },
    { MP_ROM_QSTR(MP_QSTR_led_on), MP_ROM_PTR(&esp_led_on_obj) },
    { MP_ROM_QSTR(MP_QSTR_led_off), MP_ROM_PTR(&esp_led_off_obj) },
    { MP_ROM_QSTR(MP_QSTR_esp_pin_set), MP_ROM_PTR(&esp_pin_set_obj) },
    { MP_ROM_QSTR(MP_QSTR_esp_pin_clear), MP_ROM_PTR(&esp_pin_clear_obj) },

    { MP_ROM_QSTR(MP_QSTR_neopixel_write), MP_ROM_PTR(&esp_neopixel_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_neopixel_write_timings), MP_ROM_PTR(&esp_neopixel_write_timings_obj) },
    { MP_ROM_QSTR(MP_QSTR_dht_readinto), MP_ROM_PTR(&dht_readinto_obj) },

    // Constants for second arg of osdebug()
    { MP_ROM_QSTR(MP_QSTR_LOG_NONE), MP_ROM_INT((mp_uint_t)ESP_LOG_NONE)},
    { MP_ROM_QSTR(MP_QSTR_LOG_ERROR), MP_ROM_INT((mp_uint_t)ESP_LOG_ERROR)},
    { MP_ROM_QSTR(MP_QSTR_LOG_WARNING), MP_ROM_INT((mp_uint_t)ESP_LOG_WARN)},
    { MP_ROM_QSTR(MP_QSTR_LOG_INFO), MP_ROM_INT((mp_uint_t)ESP_LOG_INFO)},
    { MP_ROM_QSTR(MP_QSTR_LOG_DEBUG), MP_ROM_INT((mp_uint_t)ESP_LOG_DEBUG)},
    { MP_ROM_QSTR(MP_QSTR_LOG_VERBOSE), MP_ROM_INT((mp_uint_t)ESP_LOG_VERBOSE)},
};

STATIC MP_DEFINE_CONST_DICT(esp_module_globals, esp_module_globals_table);

const mp_obj_module_t esp_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&esp_module_globals,
};

