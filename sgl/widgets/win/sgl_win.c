/* source/widgets/sgl_win.c
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

#include <sgl_theme.h>
#include "sgl_win.h"

static void sgl_win_construct_cb(sgl_surf_t *surf, sgl_obj_t* obj, sgl_event_t *evt)
{
    sgl_win_t *win = sgl_container_of(obj, sgl_win_t, obj);
    sgl_rect_t bg = obj->coords;
    sgl_rect_t title_bg = obj->coords;
    sgl_pos_t align_pos;
    int16_t close_cx, close_cy, close_r, title_h;

    title_h = sgl_max3(obj->radius, win->title_h, sgl_font_get_height(win->title_font));
    bg.y1 -= title_h;
    title_bg.y1 = bg.y1;
    title_bg.y2 = title_bg.y1 + title_h;
    close_r  = title_h / 3;
    close_cx = obj->coords.x2 - close_r - obj->radius / 2;
    close_cy = title_bg.y1 + title_h / 2 + obj->border / 2;

    if (obj->area.y1 == obj->coords.y1) {
        obj->area.y1 -= title_h;
    }

    if (evt->type == SGL_EVENT_DRAW_MAIN) {
        sgl_draw_rect(surf, &obj->area, &bg, &win->bg);
        sgl_draw_fill_rect_with_border(surf, &title_bg, &bg, obj->radius, win->title_bg_color, 
                                             win->bg.border_color, win->bg.border, win->bg.alpha
                                      );

        align_pos = sgl_get_text_pos(&title_bg, win->title_font, win->title_text, 0, (sgl_align_type_t)win->title_align);
        if (win->title_align == SGL_ALIGN_LEFT_MID) {
            align_pos.x += obj->radius;
        }
        sgl_draw_string(surf, &obj->area, align_pos.x, align_pos.y + obj->border, win->title_text, 
                          win->title_text_color, win->bg.alpha, win->title_font
                        );
        
        sgl_draw_fill_circle(surf, &title_bg, close_cx, close_cy, close_r, win->close_color, win->bg.alpha);
    }
    else if (evt->type == SGL_EVENT_PRESSED || evt->type == SGL_EVENT_CLICKED) {
        if (evt->pos.x >= (close_cx - close_r) && evt->pos.x <= (close_cx + close_r) 
             && evt->pos.y >= (close_cy - close_r) && evt->pos.y <= (close_cy + close_r)) {
            sgl_obj_set_destroyed(obj);
        }
    }
}

/**
 * @brief create a window object
 * @param parent parent of the window
 * @return window object
 */
sgl_obj_t* sgl_win_create(sgl_obj_t* parent)
{
    sgl_win_t *win = sgl_malloc(sizeof(sgl_win_t));
    if(win == NULL) {
        SGL_LOG_ERROR("sgl_win_create: malloc failed");
        return NULL;
    }

    /* set object all member to zero */
    memset(win, 0, sizeof(sgl_win_t));

    sgl_obj_t *obj = &win->obj;
    sgl_obj_init(&win->obj, parent);
    obj->construct_fn = sgl_win_construct_cb;
    sgl_obj_set_clickable(obj);

    win->bg.alpha = SGL_THEME_ALPHA;
    win->bg.color = SGL_THEME_COLOR;
    win->bg.radius = SGL_THEME_RADIUS;
    win->bg.border = 0;
    win->bg.border_color = SGL_THEME_BORDER_COLOR;

    win->title_align = SGL_ALIGN_LEFT_MID;
    win->title_bg_color = SGL_THEME_COLOR;
    win->title_font = sgl_get_system_font();
    win->close_color = sgl_rgb(255, 90, 80);

    return obj;
}
