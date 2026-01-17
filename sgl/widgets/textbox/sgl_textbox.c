/* source/widgets/sgl_textbox.c
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
#include <sgl_cfgfix.h>
#include <sgl_theme.h>
#include "sgl_textbox.h"


#define  SGL_TEXTBOX_SCROLL_WIDTH                  (4)

static int16_t textbox_scroll_get_pos(sgl_obj_t* obj, int16_t scroll_h)
{
    sgl_textbox_t *textbox = (sgl_textbox_t*)obj;
    int16_t height = obj->coords.y2 - obj->coords.y1 - 2 * textbox->bg.radius;
    return (-textbox->y_offset) * (height + scroll_h) / (textbox->text_height - height);
}


static void sgl_textbox_construct_cb(sgl_surf_t *surf, sgl_obj_t* obj, sgl_event_t *evt)
{
    sgl_textbox_t *textbox = (sgl_textbox_t*)obj;
    int16_t height = obj->coords.y2 - obj->coords.y1 - 2 * textbox->bg.radius;
    int16_t width = obj->coords.x2 - obj->coords.x1 - 2 * textbox->bg.radius;
    int16_t scroll_height = sgl_max(height / 8, SGL_TEXTBOX_SCROLL_WIDTH);
    sgl_rect_t area;

    SGL_ASSERT(textbox->font != NULL);

    if(evt->type == SGL_EVENT_DRAW_MAIN) {
        area.x1 = obj->coords.x1 + textbox->bg.radius;
        area.y1 = obj->coords.y1 + textbox->bg.radius;
        area.x2 = obj->coords.x2 - textbox->bg.radius;
        area.y2 = obj->coords.y2 - textbox->bg.radius;

        sgl_draw_rect(surf, &obj->area, &obj->coords, &textbox->bg);
        sgl_draw_string_mult_line(surf, &area, area.x1, 
                                 area.y1 + textbox->y_offset, 
                                 textbox->text, textbox->text_color, textbox->bg.alpha, textbox->font, textbox->line_margin
                                 );

        if(textbox->scroll_enable) {
            area.x1 = obj->coords.x2 - SGL_TEXTBOX_SCROLL_WIDTH - textbox->bg.radius;
            area.y1 = obj->coords.y1;
            area.x2 = obj->coords.x2 - textbox->bg.radius;
            area.y2 = obj->coords.y2;

            area.y1 = textbox_scroll_get_pos(obj, scroll_height) + obj->coords.y1;
            area.y2 = area.y1 + scroll_height;

            sgl_draw_fill_rect(surf, &obj->area, &area, SGL_TEXTBOX_SCROLL_WIDTH / 2, textbox->text_color, 128);
        }
    }
    else if(evt->type == SGL_EVENT_MOVE_UP) {
        textbox->text_height = sgl_font_get_string_height(width, textbox->text, textbox->font, textbox->line_margin);
        textbox->scroll_enable = 1;
        if((textbox->text_height + textbox->y_offset) > height ) {
           textbox->y_offset -= evt->distance;
        }
        sgl_obj_set_dirty(obj);
    }
    else if(evt->type == SGL_EVENT_MOVE_DOWN) {
        textbox->text_height = sgl_font_get_string_height(width, textbox->text, textbox->font, textbox->line_margin);
        textbox->scroll_enable = 1;
        if(textbox->y_offset < 0) {
            textbox->y_offset += evt->distance;
        }
        sgl_obj_set_dirty(obj);
    }
    else if (evt->type == SGL_EVENT_PRESSED) {
        textbox->scroll_enable = 1;
        sgl_obj_set_dirty(obj);
    }
    else if (evt->type == SGL_EVENT_RELEASED) {
        textbox->scroll_enable = 0;
        sgl_obj_set_dirty(obj);
    }
    else if (evt->type == SGL_EVENT_FOCUSED) {
        textbox->bg.border ++;
    }
    else if (evt->type == SGL_EVENT_UNFOCUSED) {
        textbox->bg.border --;
    }
}


/**
 * @brief create a textbox object
 * @param parent parent of the textbox
 * @return pointer to the textbox object
 */
sgl_obj_t* sgl_textbox_create(sgl_obj_t* parent)
{
    sgl_textbox_t *textbox = sgl_malloc(sizeof(sgl_textbox_t));
    if(textbox == NULL) {
        SGL_LOG_ERROR("sgl_textbox_create: malloc failed");
        return NULL;
    }

    /* set object all member to zero */
    memset(textbox, 0, sizeof(sgl_textbox_t));

    sgl_obj_t *obj = &textbox->obj;
    sgl_obj_init(&textbox->obj, parent);
    obj->construct_fn = sgl_textbox_construct_cb;
    sgl_obj_set_border_width(obj, SGL_THEME_BORDER_WIDTH);
    obj->focus = 1;

    sgl_obj_set_clickable(obj);
    sgl_obj_set_movable(obj);

    textbox->bg.alpha = SGL_THEME_ALPHA;
    textbox->bg.color = SGL_THEME_COLOR;
    textbox->bg.radius = 10;
    textbox->bg.border = 1;
    textbox->bg.border_color = SGL_THEME_BORDER_COLOR;

    textbox->scroll.alpha = SGL_THEME_ALPHA;
    textbox->scroll.color = SGL_THEME_SCROLL_FG_COLOR;

    textbox->text_color = SGL_THEME_TEXT_COLOR;
    textbox->line_margin = 1;
    textbox->text = "textbox";

    textbox->y_offset = 0;
    textbox->scroll_enable = 0;

    return obj;
}
