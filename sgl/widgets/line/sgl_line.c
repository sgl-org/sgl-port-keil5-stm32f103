/* source/widgets/sgl_line.c
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
#include "sgl_line.h"


static void sgl_line_construct_cb(sgl_surf_t *surf, sgl_obj_t* obj, sgl_event_t *evt)
{
    sgl_line_t *line = (sgl_line_t*)obj;

    if(evt->type == SGL_EVENT_DRAW_MAIN) {
        sgl_draw_line(surf, &obj->area, &line->desc);
    }
}


/**
 * @brief create a line object
 * @param parent parent of the line
 * @return line object
 */
sgl_obj_t* sgl_line_create(sgl_obj_t* parent)
{
    sgl_line_t *line = sgl_malloc(sizeof(sgl_line_t));
    if(line == NULL) {
        SGL_LOG_ERROR("sgl_line_create: malloc failed");
        return NULL;
    }

    /* set object all member to zero */
    memset(line, 0, sizeof(sgl_line_t));

    sgl_obj_t *obj = &line->obj;
    sgl_obj_init(&line->obj, parent);
    obj->construct_fn = sgl_line_construct_cb;

    line->desc.color = SGL_THEME_BG_COLOR;
    line->desc.alpha = SGL_ALPHA_MAX;
    line->desc.width = 1;

    return obj;
}


/**
 * @brief set line start position
 * @param obj line object
 * @param x start x position
 * @param y start y position
 * @return none
 */
void sgl_line_set_pos(sgl_obj_t *obj, int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
	SGL_ASSERT(obj != NULL);
    int16_t _x1, _y1, _x2, _y2;
	sgl_line_t *line = (sgl_line_t*)obj;

	line->desc.start.x = obj->parent->coords.x1 + x1;
	line->desc.start.y = obj->parent->coords.y1 + y1;
	line->desc.end.x = obj->parent->coords.x1 + x2;
	line->desc.end.y = obj->parent->coords.y1 + y2;

    _x1 = line->desc.start.x;
    _x2 = line->desc.end.x;
    _y1 = line->desc.start.y;
    _y2 = line->desc.end.y;

	if (_x1 > _x2) {
		sgl_swap(&_x1, &_x2);
	}
	if (_y1 > _y2) {
		sgl_swap(&_y1, &_y2);
	}

	/* default thinckness is 1 */
	obj->coords.x1 = _x1 - 1;
	obj->coords.y1 = _y1 - 1;
	obj->coords.x2 = _x2 + 1;
	obj->coords.y2 = _y2 + 1;
	line->desc.width = 2;

	sgl_obj_set_dirty(obj);
}
