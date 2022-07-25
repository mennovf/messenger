/**
 * @file lv_draw_basic.h
 *
 */

#ifndef LV_DRAW_BASIC_H
#define LV_DRAW_BASIC_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#ifdef LV_CONF_INCLUDE_SIMPLE
#include "lv_conf.h"
#else
#include "../../lv_conf.h"
#endif

#include "../lv_font/lv_font.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Draw a letter in the Virtual Display Buffer
 * @param pos_p left-top coordinate of the latter
 * @param mask_p the letter will be drawn only on this area
 * @param font_p pointer to font
 * @param letter a letter to draw
 * @param color color of letter
 * @param opa opacity of letter (0..255)
 */
typedef void (*DrawPixel)(uint16_t left, uint16_t top, uint16_t color);
uint16_t lv_draw_letter(DrawPixel draw_pixel, uint16_t const pos_px, uint16_t const pos_py, const lv_font_t * font_p, uint32_t letter, uint32_t next_letter, uint16_t color, uint16_t bg); 
/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_DRAW_BASIC_H*/
