/* source/widgets/sgl_scope.c
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

#include <stdio.h>
#include <sgl_theme.h>
#include "sgl_scope.h"


// Draw a dashed line using Bresenham's algorithm with dash pattern
static void draw_dashed_line(sgl_surf_t *surf, sgl_area_t *area, sgl_draw_line_t *line)
{
    int16_t x0 = line->start.x;
    int16_t y0 = line->start.y;
    int16_t x1 = line->end.x;
    int16_t y1 = line->end.y;
    
    int16_t dx = sgl_abs(x1 - x0);
    int16_t dy = sgl_abs(y1 - y0);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;
    int16_t e2;

    int16_t dash_len = 0;
    const int16_t dash_pattern = 5; // Length of each dash segment

    sgl_area_t clip_area = {
        .x1 = surf->x1,
        .y1 = surf->y1,
        .x2 = surf->x2,
        .y2 = surf->y2
    };

    sgl_area_selfclip(&clip_area, area);

    while (1) {
        // Draw dash segment
        if (dash_len < dash_pattern) {
            // Check if point is within clipping area
            if (x0 >= clip_area.x1 && x0 <= clip_area.x2 && y0 >= clip_area.y1 && y0 <= clip_area.y2) {
                sgl_color_t *buf = sgl_surf_get_buf(surf, x0 - surf->x1, y0 - surf->y1);
                *buf = line->color;
            }
            dash_len++;
        } else if (dash_len < 2 * dash_pattern) {
            // Skip drawing (gap segment)
            dash_len++;
        } else {
            // Reset dash counter
            dash_len = 0;
        }
        
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}


// Custom line drawing function supporting variable line width
static void custom_draw_line(sgl_surf_t *surf, sgl_area_t *area, sgl_pos_t start, sgl_pos_t end, sgl_color_t color, int16_t width)
{
    int16_t x0 = start.x;
    int16_t y0 = start.y;
    int16_t x1 = end.x;
    int16_t y1 = end.y;
    
    // Handle invalid line width (zero or negative)
    if (width <= 0) return;
    
    // For line width = 1, use standard Bresenham algorithm
    if (width == 1) {
        int16_t dx = sgl_abs(x1 - x0);
        int16_t dy = sgl_abs(y1 - y0);
        int16_t sx = (x0 < x1) ? 1 : -1;
        int16_t sy = (y0 < y1) ? 1 : -1;
        int16_t err = dx - dy;
        int16_t e2;
        
        sgl_area_t clip_area = {
            .x1 = surf->x1,
            .y1 = surf->y1,
            .x2 = surf->x2,
            .y2 = surf->y2
        };
        
        while (1) {
            // Check if point is within clipping area
            if (x0 >= clip_area.x1 && x0 <= clip_area.x2 && y0 >= clip_area.y1 && y0 <= clip_area.y2) {
                sgl_color_t *buf = sgl_surf_get_buf(surf, x0 - surf->x1, y0 - surf->y1);
                *buf = color;
            }
            
            if (x0 == x1 && y0 == y1) break;
            e2 = 2 * err;
            if (e2 > -dy) {
                err -= dy;
                x0 += sx;
            }
            if (e2 < dx) {
                err += dx;
                y0 += sy;
            }
        }
        return;
    }
    
    // For line width > 1, draw main line plus perpendicular offsets to simulate thickness
    int16_t dx = sgl_abs(x1 - x0);
    int16_t dy = sgl_abs(y1 - y0);
    int16_t sx = (x0 < x1) ? 1 : -1;
    int16_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx - dy;
    int16_t e2;
    
    sgl_area_t clip_area = {
        .x1 = surf->x1,
        .y1 = surf->y1,
        .x2 = surf->x2,
        .y2 = surf->y2
    };

    sgl_area_selfclip(&clip_area, area);
    
    // Compute half-width for symmetric thickening
    int16_t half_width = width / 2;
    
    while (1) {
        // Draw current pixel and its perpendicular neighbors to form line thickness
        for (int16_t w = -half_width; w <= half_width; w++) {
            int16_t px, py;
            
            // Determine offset direction based on dominant axis
            if (dx > dy) {  // Dominant X-axis direction
                px = x0;
                py = y0 + w;
            } else {  // Dominant Y-axis direction
                px = x0 + w;
                py = y0;
            }
            
            // Check if point is within clipping area
            if (px >= clip_area.x1 && px <= clip_area.x2 && py >= clip_area.y1 && py <= clip_area.y2) {
                sgl_color_t *buf = sgl_surf_get_buf(surf, px - surf->x1, py - surf->y1);
                *buf = color;
            }
        }

        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

// Oscilloscope drawing callback function
static void scope_construct_cb(sgl_surf_t *surf, sgl_obj_t* obj, sgl_event_t *evt)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;

    if(evt->type == SGL_EVENT_DRAW_MAIN) {
        // Skip drawing if object is completely outside screen bounds
        if (obj->area.x2 < surf->x1 || obj->area.x1 >= surf->x2 ||
            obj->area.y2 < surf->y1 || obj->area.y1 >= surf->y2) {
            return; // Object is fully off-screen; no need to draw
        }

        // Draw background
        sgl_draw_rect_t bg_rect = {
            .color = scope->bg_color,
            .alpha = scope->alpha,
            .radius = 0,
            .border = scope->border_width,
        };

        sgl_draw_rect(surf, &obj->area, &obj->coords, &bg_rect);

        // Compute waveform display parameters
        uint16_t display_min = scope->min_value;
        uint16_t display_max = scope->max_value;
        
        // If auto-scaling is enabled, use runtime min/max values
        if (scope->auto_scale) {
            display_min = scope->running_min;
            display_max = scope->running_max;
            
            // Add margin to prevent waveform from touching borders
            uint16_t margin = (display_max - display_min) / 10;
            if (margin == 0) margin = 1;
            
            display_min = (display_min > margin) ? display_min - margin : 0;
            display_max = (display_max + margin < 0xFFFF) ? display_max + margin : 0xFFFF;
        }
        
        // Avoid division by zero if min equals max
        if (display_min == display_max) {
            if (display_max < 0xFFFF) {
                display_max++;
            } else {
                display_min--;
            }
        }
        
        // Draw grid lines
        sgl_draw_line_t grid_line = {
            .color = scope->grid_color,
            .width = 1,
            .alpha = scope->alpha,
        };
        
        int16_t width = obj->coords.x2 - obj->coords.x1;
        int16_t height = obj->coords.y2 - obj->coords.y1;
        int16_t x_center = (obj->coords.x1 + obj->coords.x2) / 2;
        int16_t y_center = obj->coords.y1 + (int32_t)(height * (display_max - (display_min + display_max) / 2)) / (display_max - display_min);
        
        // Draw horizontal center line (midpoint of display range)
        grid_line.color = scope->grid_color;
        grid_line.start.x = obj->coords.x1;
        grid_line.start.y = y_center;
        grid_line.end.x = obj->coords.x2;
        grid_line.end.y = y_center;

        if (scope->grid_style == 1) {
            // Draw dashed line
            draw_dashed_line(surf, &obj->area, &grid_line);
        } else {
            // Draw solid line
            sgl_draw_line(surf, &obj->area, &grid_line);
        }

        // Draw vertical center line
        grid_line.start.x = x_center;
        grid_line.start.y = obj->coords.y1;
        grid_line.end.x = x_center;
        grid_line.end.y = obj->coords.y2;
        
        if (scope->grid_style == 1) {
            // Draw dashed line
            draw_dashed_line(surf, &obj->area, &grid_line);
        } else {
            // Draw solid line
            sgl_draw_line(surf, &obj->area, &grid_line);
        }
        
        // Draw vertical grid lines (10 divisions)
        grid_line.color = scope->grid_color;
        for (int i = 1; i < 10; i++) {
            int16_t x_pos = obj->coords.x1 + (width * i / 10);
            grid_line.start.x = x_pos;
            grid_line.start.y = obj->coords.y1;
            grid_line.end.x = x_pos;
            grid_line.end.y = obj->coords.y2;
            
            if (scope->grid_style == 1) {
                // Draw dashed line
                draw_dashed_line(surf, &obj->area, &grid_line);
            } else {
                // Draw solid line
                sgl_draw_line(surf, &obj->area, &grid_line);
            }
        }
        
        // Draw horizontal grid lines (10 divisions)
        for (int i = 1; i < 10; i++) {
            int16_t y_pos = obj->coords.y1 + (height * i / 10);
            grid_line.start.x = obj->coords.x1;
            grid_line.start.y = y_pos;
            grid_line.end.x = obj->coords.x2;
            grid_line.end.y = y_pos;
            
            if (scope->grid_style == 1) {
                // Draw dashed line
                draw_dashed_line(surf, &obj->area, &grid_line);
            } else {
                // Draw solid line
                sgl_draw_line(surf, &obj->area, &grid_line);
            }
        }

        // Draw waveform data
        if (scope->display_count > 1) {
            sgl_pos_t start, end;
            
            // Determine number of points to display
            uint32_t display_points = scope->max_display_points > 0 ? scope->max_display_points : scope->data_len;
            if (display_points > scope->data_len) display_points = scope->data_len;
            
            // Number of actual data points to render
            uint32_t data_points = scope->display_count < display_points ? scope->display_count : display_points;
            
            // Compute index of the most recent data point (rightmost on screen)
            uint32_t last_index = (scope->current_index == 0) ? scope->data_len - 1 : scope->current_index - 1;
            int16_t last_value = scope->data_buffer[last_index];
            
            // Clamp value to display range
            if (last_value < display_min) last_value = display_min;
            if (last_value > display_max) last_value = display_max;
            
            start.x = obj->coords.x2;  // Rightmost X position
            start.y = obj->coords.y2 - ((int32_t)(last_value - display_min) * height) / (display_max - display_min);
            
            // Draw waveform from right to left
            for (uint32_t i = 1; i < data_points; i++) {
                //int index = (scope->current_index >= i) ? scope->current_index - i : scope->data_len - (i - scope->current_index);
                uint32_t prev_index = (scope->current_index >= i + 1) ? scope->current_index - (i + 1) : scope->data_len - (i + 1 - scope->current_index);
                
                int16_t current_value = scope->data_buffer[prev_index];

                // Clamp value to display range
                current_value = sgl_clamp(current_value, display_min, display_max);

                end.x = obj->coords.x2 - (i * width / (data_points - 1));  // Move leftward
                end.y = obj->coords.y2 - ((int32_t)(current_value - display_min) * height) / (display_max - display_min);

                custom_draw_line(surf, &obj->area, start, end, scope->waveform_color, scope->line_width);

                start = end;
            }
        }

        // Draw Y-axis labels if enabled and font is set
        if (scope->show_y_labels && scope->y_label_font) {
            char label_text[16];
            sgl_area_t text_area = {
                .x1 = obj->coords.x1 + 2,
                .y1 = obj->coords.y1,
                .x2 = obj->coords.x1 + 50,
                .y2 = obj->coords.y2
            };

            sgl_area_selfclip(&text_area, &obj->area);
            
            // Display maximum value
            sprintf(label_text, "%u", display_max);
            sgl_draw_string(surf, &text_area, obj->coords.x1 + 2, obj->coords.y1 + 2, 
                           label_text, scope->y_label_color, scope->alpha, scope->y_label_font);
            
            // Display minimum value
            sprintf(label_text, "%u", display_min);
            sgl_draw_string(surf, &text_area, obj->coords.x1 + 2, obj->coords.y2 - scope->y_label_font->font_height - 2, 
                           label_text, scope->y_label_color, scope->alpha, scope->y_label_font);
            
            // Display mid-range value
            uint16_t mid_value = (display_max + display_min) / 2;
            sprintf(label_text, "%u", mid_value);
            sgl_draw_string(surf, &text_area, obj->coords.x1 + 2, y_center - scope->y_label_font->font_height/2, 
                           label_text, scope->y_label_color, scope->alpha, scope->y_label_font);
        }
    }
}


// Create an oscilloscope object
sgl_obj_t* sgl_scope_create(sgl_obj_t* parent)
{
    sgl_scope_t *scope = sgl_malloc(sizeof(sgl_scope_t));
    if(scope == NULL) {
        return NULL;
    }
    
    memset(scope, 0, sizeof(sgl_scope_t));
    
    sgl_obj_t *obj = &scope->obj;
    sgl_obj_init(obj, parent);
    obj->construct_fn = scope_construct_cb;
    sgl_obj_set_border_width(obj, SGL_THEME_BORDER_WIDTH);
    
    // Initialize default parameters
    scope->waveform_color = sgl_rgb(0, 255, 0);   // Green waveform
    scope->bg_color = sgl_rgb(0, 0, 0);           // Black background
    scope->grid_color = sgl_rgb(50, 50, 50);      // Gray grid lines
    scope->border_width = 0;                      // border width is 0
    scope->border_color = sgl_rgb(150, 150, 150); // Light gray outer border
    scope->min_value = 0;
    scope->max_value = 0xFFFF;
    scope->running_min = 0xFFFF;  // Initialize runtime minimum
    scope->running_max = 0;       // Initialize runtime maximum
    scope->auto_scale = 1;        // Enable auto-scaling by default
    scope->line_width = 2;        // Default line thickness
    scope->max_display_points = 0; // Display all points by default
    scope->show_y_labels = 0;      // Hide Y-axis labels by default
    scope->alpha = SGL_ALPHA_MAX;  // Fully opaque by default
    scope->grid_style = 0;         // Solid grid lines by default
    scope->y_label_font = NULL;    // No font by default
    scope->y_label_color = sgl_rgb(255, 255, 255); // White label color
    scope->display_count = 0;         // No data initially

    return obj;
}

/**
 * @brief Append a new data point to the oscilloscope
 * @param obj The oscilloscope object
 * @param value The new data point
 * @note This function appends a new data point to the oscilloscope. 
 *       If the oscilloscope is configured to auto-scale, the function updates the running minimum and maximum values. 
 *       The function also updates the display count and marks the oscilloscope object as dirty.
 */
void sgl_scope_append_data(sgl_obj_t* obj, int16_t value)
{
    sgl_scope_t *scope = (sgl_scope_t*)obj;
    
    // update the auto scale only if the auto_scale is enabled
    if (scope->auto_scale) {
        if (value < scope->running_min) scope->running_min = value;
        if (value > scope->running_max) scope->running_max = value;
    }

    scope->data_buffer[scope->current_index] = value;

    if (sgl_is_pow2(scope->data_len)) {
        scope->current_index = (scope->current_index + 1) & (scope->data_len - 1);
    } else {
        scope->current_index = (scope->current_index + 1) % scope->data_len;
    }

    // update display count
    if (scope->display_count < scope->data_len) {
        scope->display_count++;
    }

    sgl_obj_set_dirty(obj);
}
