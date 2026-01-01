/* source/widgets/sgl_scroll.c
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

#include <sgl_core.h>
#include <sgl_draw.h>
#include <sgl_math.h>
#include <sgl_log.h>
#include <sgl_mm.h>
#include <sgl_theme.h>
#include <sgl_cfgfix.h>
#include <string.h>
#include "sgl_scroll.h"

#define  SGL_SCROLL_DEFAULT_WIDTH              (10)

/**
 * @brief scroll construct callback
 * @param surf pointer to surface
 * @param obj pointer to object
 * @param evt pointer to event
 * @return none
 */
static void sgl_scroll_construct_cb(sgl_surf_t *surf, sgl_obj_t* obj, sgl_event_t *evt)
{
    sgl_scroll_t *scroll = (sgl_scroll_t*)obj;
    sgl_rect_t fill = {0};
    sgl_color_t tmp = scroll->desc.color;
    uint16_t len = 0, pos = 0;
    scroll->desc.radius = sgl_min(scroll->desc.radius, scroll->width / 2);

    if(evt->type == SGL_EVENT_DRAW_MAIN) {
        if (scroll->bind && (scroll->hidden != 1)) {

            if (scroll->direct == SGL_DIRECT_VERTICAL) {
                obj->coords.x2 = scroll->bind->coords.x2;
                obj->coords.y2 = scroll->bind->coords.y2;
                obj->coords.y1 = scroll->bind->coords.y1;
                obj->coords.x1 = obj->coords.x2 - scroll->width;

                len = sgl_max((obj->coords.y2 - obj->coords.y1) / 8, scroll->desc.radius * 2 + 1);
                pos = scroll->value * (obj->coords.y2 - obj->coords.y1 - len) / 100;

                fill.x1 = obj->coords.x1 + scroll->desc.border;
                fill.x2 = obj->coords.x2 - scroll->desc.border;
                //fill.y1 = pos + scroll->desc.border;
                //fill.y2 = pos + len - scroll->desc.border;
                fill.y1 = obj->coords.y1+pos + scroll->desc.border;
                fill.y2 = obj->coords.y1+pos + len - scroll->desc.border;
            }
            else if (scroll->direct == SGL_DIRECT_HORIZONTAL) {
                obj->coords.x1 = scroll->bind->coords.x1;
                obj->coords.y2 = scroll->bind->coords.y2;
                obj->coords.x2 = scroll->bind->coords.x2;
                obj->coords.y1 = obj->coords.y2 - scroll->width;

                len = sgl_max((obj->coords.x2 - obj->coords.x1) / 8, scroll->desc.radius * 2 + 1);
                pos = scroll->value * (obj->coords.x2 - obj->coords.x1 - len) / 100;

                fill.y1 = obj->coords.y1 + scroll->desc.border;
                fill.y2 = obj->coords.y2 - scroll->desc.border;
                //fill.x1 = pos + scroll->desc.border;
                //fill.x2 = pos + len - scroll->desc.border;
                fill.x1 =obj->coords.x1+ pos + scroll->desc.border;
                fill.x2 =obj->coords.x2+ pos + len - scroll->desc.border;
            }

            sgl_draw_rect(surf, &obj->coords, &obj->coords, &scroll->desc);
            tmp = sgl_color_mixer(tmp, SGL_THEME_BG_COLOR, 128);
            sgl_draw_fill_rect(surf, &fill, &fill, scroll->desc.radius - scroll->desc.border, tmp, scroll->desc.alpha);
        }
    }
    else if (evt->type == SGL_EVENT_MOVE_DOWN) {
        if (scroll->direct == SGL_DIRECT_VERTICAL && scroll->value < 100) {
            scroll->value ++;
            sgl_obj_move_child_pos_y(scroll->bind, evt->distance);
        }
    }
    else if(evt->type == SGL_EVENT_MOVE_UP) {
        if (scroll->direct == SGL_DIRECT_VERTICAL && scroll->value > 0) {
            scroll->value --;
            sgl_obj_move_child_pos_y(scroll->bind, -evt->distance);
        }
    }
    else if(evt->type == SGL_EVENT_MOVE_LEFT) {
        if (scroll->direct == SGL_DIRECT_HORIZONTAL && scroll->value > 0) {
            scroll->value --;
            sgl_obj_move_child_pos_x(scroll->bind, -evt->distance);
        }
    }
    else if(evt->type == SGL_EVENT_MOVE_RIGHT) {
        if (scroll->direct == SGL_DIRECT_HORIZONTAL && scroll->value < 100) {
            scroll->value ++;
            sgl_obj_move_child_pos_x(scroll->bind, evt->distance);
        }
    }
    else if(evt->type == SGL_EVENT_PRESSED) {
        if(obj->event_fn) {
            obj->event_fn(evt);
        }
    }
    else if(evt->type == SGL_EVENT_RELEASED) {
        if(obj->event_fn) {
            obj->event_fn(evt);
        }
    }
}


/**
 * @brief create scroll object
 * @param parent pointer to parent object
 * @return pointer to scroll object
 */
sgl_obj_t* sgl_scroll_create(sgl_obj_t* parent)
{
    sgl_scroll_t *scroll = sgl_malloc(sizeof(sgl_scroll_t));
    if(scroll == NULL) {
        SGL_LOG_ERROR("sgl_scroll_create: malloc failed");
        return NULL;
    }

    /* set object all member to zero */
    memset(scroll, 0, sizeof(sgl_scroll_t));

    sgl_obj_t *obj = &scroll->obj;
    sgl_obj_init(&scroll->obj, parent);
    obj->construct_fn = sgl_scroll_construct_cb;
    sgl_obj_set_border_width(obj, SGL_THEME_BORDER_WIDTH);
    obj->movable = 1;
    obj->clickable = 1;

    scroll->bind = NULL;
    scroll->direct = SGL_DIRECT_VERTICAL;
    scroll->hidden = false;
    scroll->value = 0;
    scroll->width = SGL_SCROLL_DEFAULT_WIDTH;
    scroll->desc.radius = SGL_THEME_RADIUS;
    scroll->desc.color = SGL_THEME_COLOR;
    scroll->desc.alpha = SGL_THEME_ALPHA;
    scroll->desc.border_color = SGL_THEME_BORDER_COLOR;
    scroll->desc.border = 2;
    scroll->desc.pixmap = NULL;

    return obj;
}
