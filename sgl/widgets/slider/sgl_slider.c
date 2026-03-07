/* source/widgets/sgl_slider.c
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
#include "sgl_slider.h"


static void sgl_slider_construct_cb(sgl_surf_t *surf, sgl_obj_t* obj, sgl_event_t *evt)
{
    sgl_slider_t *slider = sgl_container_of(obj, sgl_slider_t, obj);
    int16_t w = obj->coords.x2 - obj->coords.x1 + 1;
    int16_t h = obj->coords.y2 - obj->coords.y1 + 1;
    int16_t knob_r, fill_pos, thickness, radius;
    sgl_rect_t bar;

    if(evt->type == SGL_EVENT_DRAW_MAIN) {
        if(slider->direct == SGL_DIRECT_HORIZONTAL) {
            knob_r = h / 2 - 1;
            thickness = sgl_min(slider->thickness, knob_r);
            bar.x1 = obj->coords.x1 + knob_r;
            bar.x2 = obj->coords.x2 - knob_r;
            bar.y1 = obj->coords.y1 + (h - thickness) / 2;
            bar.y2 = bar.y1 + thickness - 1;
            fill_pos = obj->coords.x1 + (obj->coords.x2 - obj->coords.x1) * slider->value / 100 - obj->border;
    
            radius = sgl_min(thickness / 2, obj->radius);
            sgl_draw_fill_rect(surf, &obj->area, &bar, radius, slider->track_color, SGL_ALPHA_MAX);
            fill_pos = sgl_clamp(fill_pos, bar.x1, bar.x2);
            bar.x2 = fill_pos;
            sgl_draw_fill_rect(surf, &obj->area, &bar, radius, slider->fill_color, SGL_ALPHA_MAX);
            sgl_draw_fill_circle(surf, &obj->area, fill_pos, sgl_mid(bar.y1, bar.y2), knob_r, slider->knob_color, SGL_ALPHA_MAX);
        }
        else {
            knob_r = w / 2 - 1;
            thickness = sgl_min(slider->thickness, knob_r);
            bar.y1 = obj->coords.y1 + knob_r;
            bar.y2 = obj->coords.y2 - knob_r;
            bar.x2 = obj->coords.x2 - (w - thickness) / 2;
            bar.x1 = bar.x2 - thickness + 1;
            fill_pos = obj->coords.y2 - (obj->coords.y2 - obj->coords.y1) * slider->value / 100 + obj->border;

            radius = sgl_min(thickness / 2, obj->radius);
            sgl_draw_fill_rect(surf, &obj->area, &bar, radius, slider->track_color, SGL_ALPHA_MAX);
            fill_pos = sgl_clamp(fill_pos, bar.y1, bar.y2);
            bar.y1 = fill_pos;
            sgl_draw_fill_rect(surf, &obj->area, &bar, radius, slider->fill_color, SGL_ALPHA_MAX);
            sgl_draw_fill_circle(surf, &obj->area, sgl_mid(bar.x1, bar.x2), fill_pos, knob_r, slider->knob_color, SGL_ALPHA_MAX);
        }
    }
    else if(evt->type == SGL_EVENT_PRESSED ||
        evt->type == SGL_EVENT_MOVE_DOWN || evt->type == SGL_EVENT_MOVE_UP || evt->type == SGL_EVENT_MOVE_LEFT || evt->type == SGL_EVENT_MOVE_RIGHT
    ) {
        if(slider->direct == SGL_DIRECT_HORIZONTAL) {
            slider->value = (evt->pos.x - obj->coords.x1) * 100 / (obj->coords.x2 - obj->coords.x1);
        }
        else {
            slider->value = (obj->coords.y2 - evt->pos.y) * 100 / (obj->coords.y2 - obj->coords.y1);
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
 * @brief create a slider object
 * @param parent parent object of the slider
 * @return slider object
 */
sgl_obj_t* sgl_slider_create(sgl_obj_t* parent)
{
    sgl_slider_t *slider = sgl_malloc(sizeof(sgl_slider_t));
    if(slider == NULL) {
        SGL_LOG_ERROR("sgl_slider_create: malloc failed");
        return NULL;
    }

    /* set object all member to zero */
    memset(slider, 0, sizeof(sgl_slider_t));

    sgl_obj_t *obj = &slider->obj;
    sgl_obj_init(&slider->obj, parent);
    sgl_obj_set_clickable(obj);
    sgl_obj_set_movable(obj);
    obj->construct_fn = sgl_slider_construct_cb;
    sgl_obj_set_border_width(obj, SGL_THEME_BORDER_WIDTH);

    slider->direct = SGL_DIRECT_HORIZONTAL;
    slider->track_color = sgl_color_mixer(SGL_THEME_COLOR, SGL_THEME_BG_COLOR, 128);
    slider->knob_color = SGL_THEME_BG_COLOR;
    slider->fill_color = SGL_THEME_BG_COLOR;
    slider->thickness = 255;
    return obj;
}
