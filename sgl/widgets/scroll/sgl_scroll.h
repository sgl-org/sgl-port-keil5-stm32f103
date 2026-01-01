/* source/widgets/sgl_scroll.h
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

#ifndef __SGL_SCROLL_H__
#define __SGL_SCROLL_H__

#include <sgl_core.h>
#include <sgl_draw.h>
#include <sgl_math.h>
#include <sgl_log.h>
#include <sgl_mm.h>
#include <sgl_cfgfix.h>
#include <string.h>


typedef struct sgl_scroll {
    sgl_obj_t               obj;
    sgl_obj_t               *bind;
    sgl_draw_rect_t         desc;
    uint8_t                 width;
    uint8_t                 value;
    uint8_t                 direct;
    uint8_t                 hidden;
} sgl_scroll_t;


/**
 * @brief create scroll object
 * @param parent pointer to parent object
 * @return pointer to scroll object
 */
sgl_obj_t* sgl_scroll_create(sgl_obj_t* parent);


static inline void sgl_scroll_bind_obj(sgl_obj_t *obj, sgl_obj_t *bind)
{
    SGL_ASSERT(obj != NULL);
    sgl_scroll_t *scroll = (sgl_scroll_t*)obj;
    scroll->bind = bind;
    sgl_obj_set_dirty(obj);
}


static inline void sgl_scroll_set_color(sgl_obj_t *obj, sgl_color_t color)
{
    SGL_ASSERT(obj != NULL);
    sgl_scroll_t *scroll = (sgl_scroll_t*)obj;
    scroll->desc.color = color;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_scroll_set_alpha(sgl_obj_t *obj, uint8_t alpha)
{
    SGL_ASSERT(obj != NULL);
    sgl_scroll_t *scroll = (sgl_scroll_t*)obj;
    scroll->desc.alpha = alpha;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_scroll_set_radius(sgl_obj_t *obj, uint16_t radius)
{
    SGL_ASSERT(obj != NULL);
    sgl_scroll_t *scroll = (sgl_scroll_t*)obj;
    scroll->desc.radius = radius;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_scroll_set_border_color(sgl_obj_t *obj, sgl_color_t color)
{
    SGL_ASSERT(obj != NULL);
    sgl_scroll_t *scroll = (sgl_scroll_t*)obj;
    scroll->desc.border_color = color;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_scroll_set_border_width(sgl_obj_t *obj, uint8_t width)
{
    SGL_ASSERT(obj != NULL);
    sgl_scroll_t *scroll = (sgl_scroll_t*)obj;
    scroll->desc.border = width;
    sgl_obj_set_border_width(obj, width);
    sgl_obj_set_dirty(obj);
}



static inline void sgl_scroll_set_width(sgl_obj_t *obj, uint8_t width)
{
    SGL_ASSERT(obj != NULL);
    sgl_scroll_t *scroll = (sgl_scroll_t*)obj;
    scroll->width = width;
    sgl_obj_set_dirty(obj);
}


static inline void sgl_scroll_set_direct(sgl_obj_t *obj, uint8_t direct)
{
    SGL_ASSERT(obj != NULL);
    sgl_scroll_t *scroll = (sgl_scroll_t*)obj;
    scroll->direct = direct;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_scroll_set_hidden(sgl_obj_t *obj, uint8_t hidden)
{
    SGL_ASSERT(obj != NULL);
    sgl_scroll_t *scroll = (sgl_scroll_t*)obj;
    scroll->hidden = hidden;
    sgl_obj_set_dirty(obj);
}

static inline void sgl_scroll_set_value(sgl_obj_t *obj, uint8_t value)
{
    SGL_ASSERT(obj != NULL);
    sgl_scroll_t *scroll = (sgl_scroll_t*)obj;
    scroll->value = value;
    sgl_obj_set_dirty(obj);
}


static inline uint8_t sgl_scroll_get_value(sgl_obj_t *obj)
{
    SGL_ASSERT(obj != NULL);
    sgl_scroll_t *scroll = (sgl_scroll_t*)obj;
    return scroll->value;
}

#endif // !__SGL_SCROLL_H__
