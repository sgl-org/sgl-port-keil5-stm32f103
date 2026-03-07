/* source/widgets/sgl_bar.c
 *
 * MIT License
 *
 * Copyright(c) 2023-present All contributors of SGL  
 * Document reference link: https://sgl-docs.readthedocs.io
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
#include "sgl_bar.h"


static void sgl_bar_construct_cb(sgl_surf_t *surf, sgl_obj_t* obj, sgl_event_t *evt)
{
    sgl_bar_t *bar = sgl_container_of(obj, sgl_bar_t, obj);

    sgl_draw_rect_t desc = {
        .alpha = bar->alpha,
        .border = obj->border,
        .border_color = bar->border_color,
        .pixmap = bar->pixmap,
        .color = bar->track_color,
        .radius = obj->radius,
    };

    sgl_area_t knob = {
        .x1 = obj->coords.x1 + obj->border,
        .x2 = obj->coords.x2 - obj->border,
        .y1 = obj->coords.y1 + obj->border,
        .y2 = obj->coords.y2 - obj->border,
    };

    if(evt->type == SGL_EVENT_DRAW_MAIN) {
        if(bar->direct == SGL_DIRECT_HORIZONTAL) {
            knob.x2 = obj->coords.x1 + (obj->coords.x2 - obj->coords.x1) * bar->value / 100 - obj->border;
        }
        else {
            knob.y1 = obj->coords.y2 - (obj->coords.y2 - obj->coords.y1) * bar->value / 100 + obj->border;
        }

        /* set knob area */
        sgl_area_selfclip(&knob, &obj->area);

        sgl_draw_rect(surf, &obj->area, &obj->coords, &desc);
        sgl_draw_fill_rect_with_border(surf, &knob, &obj->coords, obj->radius, bar->fill_color, bar->border_color, obj->border, bar->alpha);
    }
    else if(evt->type == SGL_EVENT_PRESSED ||
        evt->type == SGL_EVENT_MOVE_DOWN || evt->type == SGL_EVENT_MOVE_UP || evt->type == SGL_EVENT_MOVE_LEFT || evt->type == SGL_EVENT_MOVE_RIGHT
    ) {
        if(bar->direct == SGL_DIRECT_HORIZONTAL) {
            bar->value = (evt->pos.x - obj->coords.x1) * 100 / (obj->coords.x2 - obj->coords.x1);
        }
        else {
            bar->value = (obj->coords.y2 - evt->pos.y) * 100 / (obj->coords.y2 - obj->coords.y1);
        }

        if(evt->type == SGL_EVENT_PRESSED) {
            sgl_obj_size_zoom(obj, 2);
        }
        sgl_obj_set_dirty(obj);
    }
    else if(evt->type == SGL_EVENT_RELEASED) {
        sgl_obj_size_zoom(obj, -2);
        sgl_obj_set_dirty(obj);
    }
}


/**
 * @brief create a bar object
 * @param parent parent object of the bar
 * @return bar object
 */
sgl_obj_t* sgl_bar_create(sgl_obj_t* parent)
{
    sgl_bar_t *bar = sgl_malloc(sizeof(sgl_bar_t));
    if(bar == NULL) {
        SGL_LOG_ERROR("sgl_bar_create: malloc failed");
        return NULL;
    }

    /* set object all member to zero */
    memset(bar, 0, sizeof(sgl_bar_t));

    sgl_obj_t *obj = &bar->obj;
    sgl_obj_init(&bar->obj, parent);
    sgl_obj_set_clickable(obj);
    sgl_obj_set_movable(obj);
    obj->construct_fn = sgl_bar_construct_cb;
    sgl_obj_set_border_width(obj, SGL_THEME_BORDER_WIDTH);

    bar->direct = SGL_DIRECT_HORIZONTAL;
    bar->alpha = SGL_THEME_ALPHA;
    bar->track_color = SGL_THEME_COLOR;
    bar->border_color = SGL_THEME_BORDER_COLOR;
    bar->fill_color = SGL_THEME_BG_COLOR;
    bar->alpha = SGL_THEME_ALPHA;

    return obj;
}
