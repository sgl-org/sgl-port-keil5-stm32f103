/* source/widgets/sgl_win.h
 *
 * MIT License
 *
 * Copyright(c) 2023-present All contributors of SGL  
 * Document reference link: docs directory
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __SGL_WIN_H__
#define __SGL_WIN_H__

#include <sgl_core.h>
#include <sgl_draw.h>
#include <sgl_math.h>
#include <sgl_log.h>
#include <sgl_mm.h>
#include <sgl_cfgfix.h>
#include <string.h>

/**
 * @brief sgl window struct
 * @obj: sgl general object
 */
typedef struct sgl_win {
    sgl_obj_t       obj;
    const char     *title_text;
    const sgl_font_t *title_font;
    sgl_color_t     title_bg_color;
    sgl_color_t     title_text_color;
    sgl_draw_rect_t bg;
    uint8_t         title_h;
    uint8_t         title_align;
    sgl_color_t     close_color;
} sgl_win_t;

/**
 * @brief create a window object
 * @param parent parent of the window
 * @return window object
 */
sgl_obj_t* sgl_win_create(sgl_obj_t* parent);

static inline void sgl_win_set_color(sgl_obj_t *obj, sgl_color_t color)
{
    sgl_win_t *win = sgl_container_of(obj, sgl_win_t, obj);
    win->bg.color = color;
    sgl_obj_set_dirty(obj);
}

/**
 * @brief set window radius
 * @param obj window object
 * @param radius radius
 */
static inline void sgl_win_set_radius(sgl_obj_t *obj, uint8_t radius)
{
    sgl_win_t *win = sgl_container_of(obj, sgl_win_t, obj);
    win->bg.radius = radius;
    sgl_obj_set_radius(obj, radius);
    sgl_obj_set_dirty(obj);
}

/**
 * @brief set window alpha
 * @param obj window object
 * @param alpha alpha
 */
static inline void sgl_win_set_alpha(sgl_obj_t *obj, uint8_t alpha)
{
    sgl_win_t *win = sgl_container_of(obj, sgl_win_t, obj);
    win->bg.alpha = alpha;
    sgl_obj_set_dirty(obj);
}

/**
 * @brief set window border width
 * @param obj window object
 * @param width border width
 */
static inline void sgl_win_set_border_width(sgl_obj_t *obj, uint8_t width)
{
    sgl_win_t *win = sgl_container_of(obj, sgl_win_t, obj);
    win->bg.border = width;
    sgl_obj_set_border_width(obj, width);
    sgl_obj_set_dirty(obj);
}

/**
 * @brief set window border color
 * @param obj window object
 * @param color border color
 */
static inline void sgl_win_set_border_color(sgl_obj_t *obj, sgl_color_t color)
{
    sgl_win_t *win = sgl_container_of(obj, sgl_win_t, obj);
    win->bg.border_color = color;
    sgl_obj_set_dirty(obj);
}

/**
 * @brief set window pixmap
 * @param obj window object
 * @param pixmap pixmap
 */
static inline void sgl_win_set_pixmap(sgl_obj_t *obj, const sgl_pixmap_t *pixmap)
{
    sgl_win_t *win = sgl_container_of(obj, sgl_win_t, obj);
    win->bg.pixmap = pixmap;
    sgl_obj_set_dirty(obj);
}

/**
 * @brief set window title text
 * @param obj window object
 * @param text title text
 */
static inline void sgl_win_set_title_text(sgl_obj_t *obj, const char *text)
{
    sgl_area_t area = obj->area;
    sgl_win_t *win = sgl_container_of(obj, sgl_win_t, obj);
    win->title_text = text;
    area.y2 = area.y1 + win->title_h;
    sgl_obj_update_area(&area);
}

/**
 * @brief set window title text color
 * @param obj window object
 * @param color text color
 */
static inline void sgl_win_set_title_text_color(sgl_obj_t *obj, sgl_color_t color)
{
    sgl_win_t *win = sgl_container_of(obj, sgl_win_t, obj);
    win->title_text_color = color;
    sgl_obj_set_dirty(obj);
}

/**
 * @brief set window title font
 * @param obj window object
 * @param font title font
 */
static inline void sgl_win_set_title_font(sgl_obj_t *obj, const sgl_font_t *font)
{
    sgl_win_t *win = sgl_container_of(obj, sgl_win_t, obj);
    win->title_font = font;
    sgl_obj_set_dirty(obj);
}

/**
 * @brief set window title height
 * @param obj window object
 * @param height title height
 */
static inline void sgl_win_set_title_height(sgl_obj_t *obj, uint16_t height)
{
    sgl_win_t *win = sgl_container_of(obj, sgl_win_t, obj);
    win->title_h = height;
    sgl_obj_set_dirty(obj);
}

/**
 * @brief set window title text align
 * @param obj window object
 * @param align text align
 */
static inline void sgl_win_set_title_text_align(sgl_obj_t *obj, uint8_t align)
{
    sgl_win_t *win = sgl_container_of(obj, sgl_win_t, obj);
    win->title_align = align;
    sgl_obj_set_dirty(obj);
}

/**
 * @brief set window title background color
 * @param obj window object
 * @param color title background color
 */
static inline void sgl_win_set_title_bg_color(sgl_obj_t *obj, sgl_color_t color)
{
    sgl_win_t *win = sgl_container_of(obj, sgl_win_t, obj);
    win->title_bg_color = color;
    sgl_obj_set_dirty(obj);
}

/**
 * @brief set window close button color
 * @param obj window object
 * @param color close button color
 */
static inline void sgl_win_set_close_btn_color(sgl_obj_t *obj, sgl_color_t color)
{
    sgl_win_t *win = sgl_container_of(obj, sgl_win_t, obj);
    win->close_color = color;
    sgl_obj_set_dirty(obj);
}

#endif // !__SGL_LED_H__
