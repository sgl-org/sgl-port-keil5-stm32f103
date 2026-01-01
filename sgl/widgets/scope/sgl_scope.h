/* source/widgets/sgl_scope.h
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

#ifndef __SGL_SCOPE_H__
#define __SGL_SCOPE_H__

#include <sgl_core.h>
#include <sgl_draw.h>
#include <sgl_math.h>
#include <sgl_log.h>
#include <sgl_mm.h>
#include <sgl_cfgfix.h>
#include <string.h>


#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    sgl_obj_t obj;
    int16_t  *data_buffer;             // channel data buffer
    uint32_t  data_len;                // data length
    sgl_color_t waveform_color;        // waveform color
    sgl_color_t bg_color;              // background color
    sgl_color_t grid_color;            // grid line color
    sgl_color_t border_color;          // border color
    int16_t min_value;                 // min value of data
    int16_t max_value;                 // max value of data
    int16_t running_min;               // min value of runtime
    int16_t running_max;               // max value of runtime
    uint8_t auto_scale : 1;            // whether to automatically scale
    uint8_t show_y_labels : 1;         // whether to show Y axis labels
    uint8_t border_width;              // outer border width
    uint8_t line_width;                // width of waveform line
    uint8_t display_count;             // data count that has been displayed
    uint32_t max_display_points;       // max display points
    uint8_t alpha;                     // aplha of waveform
    uint8_t grid_style;                // grid line style（0-solid line，1-dashed line）
    const sgl_font_t *y_label_font;    // font of Y axis labels
    sgl_color_t y_label_color;         // color of Y axis labels
    uint32_t current_index;            // current data index
} sgl_scope_t;


/**
 * @brief create scope object
 * @param parent parent object
 * @return scope object
 */
sgl_obj_t* sgl_scope_create(sgl_obj_t* parent);

/**
 * @brief set scope data buffer
 * @param obj scope object
 * @param data_buffer data buffer
 * @param data_len data length
 * @return none
 */
static inline void sgl_scope_set_data_buffer(sgl_obj_t* obj, int16_t *data_buffer, uint32_t data_len)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->data_buffer = data_buffer;
    scope->data_len = data_len;
}

/**
 * @brief Append a new data point to the oscilloscope
 * @param obj The oscilloscope object
 * @param value The new data point
 * @note This function appends a new data point to the oscilloscope. 
 *       If the oscilloscope is configured to auto-scale, the function updates the running minimum and maximum values. 
 *       The function also updates the display count and marks the oscilloscope object as dirty.
 */
void sgl_scope_append_data(sgl_obj_t* obj, int16_t value);

/**
 * @brief get scope data
 * @param obj scope object
 * @param index data index
 * @return data value
 */
static inline int16_t sgl_scope_get_data(sgl_obj_t* obj, uint32_t index)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    if (index >= scope->data_len) 
        return 0;

    return scope->data_buffer[index];
}

/**
 * @brief set scope max display points
 * @param obj scope object
 * @param max_points max display points
 * @return none
 */
static inline void sgl_scope_set_max_display_points(sgl_obj_t* obj, uint8_t max_points)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->max_display_points = max_points;
    sgl_obj_set_dirty(obj);
}

/**
 * @brief set scope waveform color
 * @param obj scope object
 * @param color waveform color
 * @return none
 */
static inline void sgl_scope_set_waveform_color(sgl_obj_t* obj, sgl_color_t color)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->waveform_color = color;
    sgl_obj_set_dirty(obj);
}

/**
 * @brief set scope background color
 * @param obj scope object
 * @param color background color
 * @return none
 */
static inline void sgl_scope_set_bg_color(sgl_obj_t* obj, sgl_color_t color)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->bg_color = color;
    sgl_obj_set_dirty(obj);
}

/**
 * @brief set scope grid line color
 * @param obj scope object
 * @param color grid line color
 * @return none
 */
static inline void sgl_scope_set_grid_color(sgl_obj_t* obj, sgl_color_t color)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->grid_color = color;
    sgl_obj_set_dirty(obj);
}

/**
 * @brief set scope range
 * @param obj scope object
 * @param min_value minimum value
 * @param max_value maximum value
 * @return none
 */
static inline void sgl_scope_set_range(sgl_obj_t* obj, uint16_t min_value, uint16_t max_value)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->min_value = min_value;
    scope->max_value = max_value;
    scope->auto_scale = 0;  // disable auto scale
    sgl_obj_set_dirty(obj);
}

/**
 * @brief set scope line width
 * @param obj scope object
 * @param width line width
 * @return none
 */
static inline void sgl_scope_set_line_width(sgl_obj_t* obj, uint8_t width)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->line_width = width;
    sgl_obj_set_dirty(obj);
}

/**
 * @brief enable/disable auto scale
 * @param obj scope object
 * @param enable enable/disable
 * @return none
 */
static inline void sgl_scope_enable_auto_scale(sgl_obj_t* obj, bool enable)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->auto_scale = (uint8_t)enable;
    sgl_obj_set_dirty(obj);
}

/**
 * @brief set scope alpha
 * @param obj scope object
 * @param alpha alpha
 * @return none
 */
static inline void sgl_scope_set_alpha(sgl_obj_t* obj, uint8_t alpha)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->alpha = alpha;
    sgl_obj_set_dirty(obj);
}

/**
 * @brief show/hide Y axis labels
 * @param obj scope object
 * @param show show/hide
 * @return none
 */
static inline void sgl_scope_show_y_labels(sgl_obj_t* obj, bool show)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->show_y_labels = (uint8_t)show;
    sgl_obj_set_dirty(obj);
}

/**
 * @brief set scope Y axis labels font
 * @param obj scope object
 * @param font font
 * @return none
 */
static inline void sgl_scope_set_y_label_font(sgl_obj_t* obj, const sgl_font_t *font)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->y_label_font = font;
    sgl_obj_set_dirty(obj);
}

/**
 * @brief set scope Y axis labels color
 * @param obj scope object
 * @param color color
 * @return none
 */
static inline void sgl_scope_set_y_label_color(sgl_obj_t* obj, sgl_color_t color)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->y_label_color = color;
    sgl_obj_set_dirty(obj);
}

/**
 * @brief set scope border color
 * @param obj scope object
 * @param color border color
 * @return none
 */
static inline void sgl_scope_set_border_color(sgl_obj_t* obj, sgl_color_t color)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->border_color = color;
    sgl_obj_set_dirty(obj);
}

/**
 * @brief set scope border width
 * @param obj scope object
 * @param width border width
 * @return none
 */
static inline void sgl_scope_set_border_width(sgl_obj_t* obj, uint8_t width)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->border_width = width;
    sgl_obj_set_border_width(obj, width);
    sgl_obj_set_dirty(obj);
}

/**
 * @brief set scope grid style
 * @param obj scope object
 * @param style grid style, 0-solid line，1-dashed line
 * @return none
 */
static inline void sgl_scope_set_grid_style(sgl_obj_t* obj, uint8_t style)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    scope->grid_style = style;
    sgl_obj_set_dirty(obj);
}


#ifdef __cplusplus
}
#endif

#endif // SGL_SCOPE_H
