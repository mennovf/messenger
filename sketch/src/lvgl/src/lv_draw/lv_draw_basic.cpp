/**
 * @file lv_draw_basic.c
 *
 */

#include "lv_draw_basic.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "../lv_font/lv_font.h"

#include <stddef.h>
#include <avr/pgmspace.h>

/*********************
 *      INCLUDES
 *********************/

/*********************
 *      DEFINES
 *********************/


#ifndef LV_ATTRIBUTE_MEM_ALIGN
#define LV_ATTRIBUTE_MEM_ALIGN
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/**
 * Draw a letter in the Virtual Display Buffer
 * @param pos_p left-top coordinate of the latter
 * @param mask_p the letter will be drawn only on this area  (truncated to VDB area)
 * @param font_p pointer to font
 * @param letter a letter to draw
 * @param the next letter to be drawn
 * @param color color of letter
 * @param opa opacity of letter (0..255)
 */
 

static uint16_t color_blend(uint16_t cl, uint16_t cr, float p) {
  uint16_t Rl = (cl >> 11), Rr = (cr >> 11);
  uint16_t Gl = (cl >> 5) & 63, Gr = (cr >> 5) & 63;
  uint16_t Bl = cl & 31, Br = cr & 31;
  uint16_t R = Rl*(1 - p) + Rr*p;
  uint16_t G = Gl*(1 - p) + Gr*p;
  uint16_t B = Bl*(1 - p) + Br*p;
  return (R << 11) | G << 5 | B;
}
 
uint16_t lv_draw_letter(DrawPixel draw_pixel, uint16_t const pos_px, uint16_t const pos_py, const lv_font_t * font_p, uint32_t letter, uint32_t next_letter, uint16_t color, uint16_t bg) {
    #define C(a) (color_blend(bg, color, a / 255.f))
    const uint16_t bpp1_opa_table[2]  = {C(0), C(255)};          /*Opacity mapping with bpp = 1 (Just for compatibility)*/
    const uint16_t bpp2_opa_table[4]  = {C(0), C(1*255/3), C(2*255/3), C(3*255/3)}; /*Opacity mapping with bpp = 2*/
    const uint16_t bpp4_opa_table[16] = {C(0),  C(17), C(34),  C(51),  /*Opacity mapping with bpp = 4*/
                                        C(68), C(85), C(102), C(119), C(136), C(153), C(170), C(187), C(204), C(221), C(238), C(255)};


    lv_font_glyph_dsc_t g;
    bool g_ret = lv_font_get_glyph_dsc(font_p, &g, letter, next_letter);
    if(g_ret == false) return 0;

    lv_coord_t pos_x = pos_px + g.ofs_x;
    lv_coord_t pos_y = pos_py + (font_p->line_height - font_p->base_line) - g.box_h - g.ofs_y;

    const uint16_t * bpp_opa_table;
    uint8_t bitmask_init;
    uint8_t bitmask;

    switch(g.bpp) {
        case 1:
            bpp_opa_table = bpp1_opa_table;
            bitmask_init  = 0x80;
            break;
        case 2:
            bpp_opa_table = bpp2_opa_table;
            bitmask_init  = 0xC0;
            break;
        case 4:
            bpp_opa_table = bpp4_opa_table;
            bitmask_init  = 0xF0;
            break;
        case 8:
            bpp_opa_table = NULL;
            bitmask_init  = 0xFF;
            break;       /*No opa table, pixel value will be used directly*/
        default: return 0; /*Invalid bpp. Can't render the letter*/
    }

    const uint8_t * map_p = lv_font_get_glyph_bitmap(font_p, letter);

    if(map_p == NULL) return 0;

    /*If the letter is completely out of mask don't draw it */
    //if(pos_x + g.box_w < mask_p->x1 || pos_x > mask_p->x2 || pos_y + g.box_h < mask_p->y1 || pos_y > mask_p->y2) return;

    lv_coord_t col, row;

    uint8_t width_byte_scr = g.box_w >> 3; /*Width in bytes (on the screen finally) (e.g. w = 11 -> 2 bytes wide)*/
    if(g.box_w & 0x7) width_byte_scr++;
    uint16_t width_bit = g.box_w * g.bpp; /*Letter width in bits*/

    /* Calculate the col/row start/end on the map*/
    lv_coord_t col_start = 0;
    lv_coord_t col_end   = g.box_w;
    lv_coord_t row_start = 0;
    lv_coord_t row_end   = g.box_h;

    /*Move on the map too*/
    uint32_t bit_ofs = (row_start * width_bit) + (col_start * g.bpp);
    map_p += bit_ofs >> 3;

    uint8_t letter_px;
    uint16_t col_bit;
    col_bit = bit_ofs & 0x7; /* "& 0x7" equals to "% 8" just faster */

    for(row = row_start; row < row_end; row++) {
        bitmask = bitmask_init >> col_bit;
        for(col = col_start; col < col_end; col++) {
            letter_px = (pgm_read_byte(map_p) & bitmask) >> (8 - col_bit - g.bpp);
            if(letter_px != 0) {
                uint16_t const c = bpp_opa_table[letter_px];

		draw_pixel(col + pos_x, row + pos_y, c);
            }

            if(col_bit < 8 - g.bpp) {
                col_bit += g.bpp;
                bitmask = bitmask >> g.bpp;
            } else {
                col_bit = 0;
                bitmask = bitmask_init;
                map_p++;
            }
        }
        col_bit += ((g.box_w - col_end) + col_start) * g.bpp;

        map_p += (col_bit >> 3);
        col_bit = col_bit & 0x7;
    }
    return g.adv_w;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
