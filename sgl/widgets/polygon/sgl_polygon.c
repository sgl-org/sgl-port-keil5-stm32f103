/* source/widgets/polygon/sgl_polygon.c
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
#include "sgl_polygon.h"


// Polygon construction callback function
static void sgl_polygon_construct_cb(sgl_surf_t *surf, sgl_obj_t* obj, sgl_event_t *evt)
{
    sgl_polygon_t *polygon = (sgl_polygon_t*)obj;
    
    if (evt->type != SGL_EVENT_DRAW_MAIN) {
        return;
    }
    
    if (polygon->vertex_count < 3 || polygon->vertices == NULL) {
        return; // At least 3 vertices are required to form a polygon
    }
    
    // Draw fill
    if (polygon->fill_color.full != 0) {
        // Calculate bounding box of polygon (using actual coordinates)
        int16_t min_x = polygon->vertices[0].x, max_x = polygon->vertices[0].x;
        int16_t min_y = polygon->vertices[0].y, max_y = polygon->vertices[0].y;
        
        for (uint16_t i = 1; i < polygon->vertex_count; i++) {
            sgl_pos_t vertex = polygon->vertices[i];
            min_x = sgl_min(min_x, vertex.x);
            max_x = sgl_max(max_x, vertex.x);
            min_y = sgl_min(min_y, vertex.y);
            max_y = sgl_max(max_y, vertex.y);
        }

        // Adjust coordinates relative to parent object
        sgl_area_t polygon_area = {
            .x1 = min_x + obj->parent->coords.x1,  // Adjust to parent coordinates
            .x2 = max_x + obj->parent->coords.x1,
            .y1 = min_y + obj->parent->coords.y1,
            .y2 = max_y + obj->parent->coords.y1
        };
        
        sgl_area_t clip;
        if (sgl_surf_clip(surf, &polygon_area, &clip) && sgl_area_selfclip(&clip, &obj->area)) {
            // Calculate intersections of scan lines with polygon
            int intersections[64]; // Assume max 64 intersections per scan line
            
            for (int y = clip.y1; y <= clip.y2; y++) {
                uint8_t count = 0;
                
                // Calculate intersections of scan line with polygon
                for (uint16_t i = 0; i < polygon->vertex_count; i++) {
                    // Adjust vertex coordinates relative to parent
                    sgl_pos_t p1 = {polygon->vertices[i].x + obj->parent->coords.x1, 
                                    polygon->vertices[i].y + obj->parent->coords.y1};
                    sgl_pos_t p2 = {polygon->vertices[(i + 1) % polygon->vertex_count].x + obj->parent->coords.x1, 
                                    polygon->vertices[(i + 1) % polygon->vertex_count].y + obj->parent->coords.y1};
                    
                    // Calculate intersection of scan line with edge
                    if ((p1.y > y) != (p2.y > y)) {
                        intersections[count++] = p1.x + (y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y);
                    }
                }
                
                // Sort intersections (using optimized bubble sort)
                for (uint8_t i = 0; i < count - 1; i++) {
                    bool swapped = false;
                    for (uint8_t j = 0; j < count - i - 1; j++) {
                        if (intersections[j] > intersections[j + 1]) {
                            int temp = intersections[j];
                            intersections[j] = intersections[j + 1];
                            intersections[j + 1] = temp;
                            swapped = true;
                        }
                    }
                    if (!swapped) break; // If no swaps occurred, sorting is complete
                }
                
                // Fill scan line segments
                sgl_color_t *buf = sgl_surf_get_buf(surf, clip.x1 - surf->x1, y - surf->y1);
                for (uint8_t i = 0; i < count; i += 2) {
                    int start = sgl_max(intersections[i], clip.x1);
                    int end = sgl_min(intersections[i + 1], clip.x2);
                    
                    if (start <= end) {
                        if (polygon->alpha == SGL_ALPHA_MAX) {
                            for (int x = start; x <= end; x++) {
                                buf[x - clip.x1] = polygon->fill_color;
                            }
                        } else {
                            for (int x = start; x <= end; x++) {
                                int buf_index = x - clip.x1;
                                buf[buf_index] = sgl_color_mixer(polygon->fill_color, buf[buf_index], polygon->alpha);
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Draw border
    if (polygon->border_width > 0 && polygon->border_color.full != 0) {
        for (uint16_t i = 0; i < polygon->vertex_count; i++) {
            // Adjust vertex coordinates relative to parent
            sgl_pos_t start = {polygon->vertices[i].x + obj->parent->coords.x1, 
                               polygon->vertices[i].y + obj->parent->coords.y1};
            sgl_pos_t end = {polygon->vertices[(i + 1) % polygon->vertex_count].x + obj->parent->coords.x1, 
                             polygon->vertices[(i + 1) % polygon->vertex_count].y + obj->parent->coords.y1};

            draw_line_fill_slanted(surf, &obj->area, start.x, start.y, end.x, end.y, polygon->border_width, polygon->border_color, polygon->alpha);
        }
    }

    // Draw text
    if (polygon->text && polygon->font) {
        // Calculate center point of polygon
        int32_t center_x = 0, center_y = 0;
        for (uint16_t i = 0; i < polygon->vertex_count; i++) {
            center_x += polygon->vertices[i].x + obj->parent->coords.x1;  // Adjust to parent coordinates
            center_y += polygon->vertices[i].y + obj->parent->coords.y1;
        }
        center_x /= polygon->vertex_count;
        center_y /= polygon->vertex_count;
        
        // Calculate text dimensions
        int16_t text_width = sgl_font_get_string_width(polygon->text, polygon->font);
        int16_t text_height = sgl_font_get_height(polygon->font);
        
        // Text drawing position (centered)
        int16_t text_x = center_x - text_width / 2;
        int16_t text_y = center_y - text_height / 2;
        
        sgl_draw_string(surf, &obj->area, text_x, text_y, polygon->text, polygon->text_color, polygon->alpha, polygon->font);
    }
}
// Create polygon object
sgl_obj_t* sgl_polygon_create(sgl_obj_t* parent)
{
    sgl_polygon_t *polygon = (sgl_polygon_t*)sgl_malloc(sizeof(sgl_polygon_t));
    if (polygon == NULL) {
        return NULL;
    }
    
    memset(polygon, 0, sizeof(sgl_polygon_t));
    
    sgl_obj_t *obj = &polygon->obj;
    sgl_obj_init(obj, parent);
    obj->construct_fn = sgl_polygon_construct_cb;
    
    // Set default values
    polygon->vertex_count = 0;
    polygon->vertices = NULL;
    polygon->fill_color = sgl_rgb(127, 127, 127);
    polygon->border_color = sgl_rgb(0, 0, 0);
    polygon->border_width = 1;
    polygon->alpha = SGL_ALPHA_MAX;
    polygon->pixmap = NULL;
    polygon->text = NULL;
    polygon->font = NULL;
    polygon->text_color = sgl_rgb(0, 0, 0);
    
    return obj;
}

// Set polygon vertices
void sgl_polygon_set_vertices(sgl_obj_t* obj, sgl_pos_t* vertices, uint16_t count)
{
    sgl_polygon_t *polygon = (sgl_polygon_t *)obj;
    if (polygon == NULL || vertices == NULL || count < 3) {
        return;
    }
    
    // Free old vertex data (if any)
    if (polygon->vertices != NULL) {
        sgl_free(polygon->vertices);
    }
    
    // Allocate new vertex data
    polygon->vertices = (sgl_pos_t*)sgl_malloc(sizeof(sgl_pos_t) * count);
    if (polygon->vertices == NULL) {
        polygon->vertex_count = 0;
        return;
    }
    
    // Copy vertex data
    memcpy(polygon->vertices, vertices, sizeof(sgl_pos_t) * count);
    polygon->vertex_count = count;
    
    // Mark object as needing redraw
    sgl_obj_set_dirty(obj);
}

// Set polygon vertices by coordinate arrays
void sgl_polygon_set_vertex_coords(sgl_obj_t* obj, int16_t* x_coords, int16_t* y_coords, uint16_t count)
{
    sgl_polygon_t *polygon = (sgl_polygon_t *)obj;
    if (polygon == NULL || x_coords == NULL || y_coords == NULL || count < 3) {
        return;
    }
    
    // Free old vertex data (if any)
    if (polygon->vertices != NULL) {
        sgl_free(polygon->vertices);
    }
    
    // Allocate new vertex data
    polygon->vertices = (sgl_pos_t*)sgl_malloc(sizeof(sgl_pos_t) * count);
    if (polygon->vertices == NULL) {
        polygon->vertex_count = 0;
        return;
    }
    
    // Build vertices from coordinate arrays
    for (uint16_t i = 0; i < count; i++) {
        polygon->vertices[i].x = x_coords[i];
        polygon->vertices[i].y = y_coords[i];
    }
    polygon->vertex_count = count;
    
    // Mark object as needing redraw
    sgl_obj_set_dirty(obj);
}

// Set polygon vertices by 2D coordinate array
void sgl_polygon_set_vertex_array(sgl_obj_t* obj, int16_t (*coords)[2], uint16_t count)
{
    sgl_polygon_t *polygon = (sgl_polygon_t *)obj;
    if (polygon == NULL || coords == NULL || count < 3) {
        return;
    }
    
    // Free old vertex data (if any)
    if (polygon->vertices != NULL) {
        sgl_free(polygon->vertices);
    }
    
    // Allocate new vertex data
    polygon->vertices = (sgl_pos_t*)sgl_malloc(sizeof(sgl_pos_t) * count);
    if (polygon->vertices == NULL) {
        polygon->vertex_count = 0;
        return;
    }
    
    // Build vertices from 2D coordinate array
    for (uint16_t i = 0; i < count; i++) {
        polygon->vertices[i].x = coords[i][0];
        polygon->vertices[i].y = coords[i][1];
    }
    polygon->vertex_count = count;
    
    // Mark object as needing redraw
    sgl_obj_set_dirty(obj);
}

// Set fill color
void sgl_polygon_set_fill_color(sgl_obj_t* obj, sgl_color_t color)
{
    sgl_polygon_t *polygon = (sgl_polygon_t *)obj;
    if (polygon == NULL) {
        return;
    }
    
    polygon->fill_color = color;
    sgl_obj_set_dirty(obj);
}

// Set border color
void sgl_polygon_set_border_color(sgl_obj_t* obj, sgl_color_t color)
{
    sgl_polygon_t *polygon = (sgl_polygon_t *)obj;
    if (polygon == NULL) {
        return;
    }
    
    polygon->border_color = color;
    sgl_obj_set_dirty(obj);
}

// Set border width
void sgl_polygon_set_border_width(sgl_obj_t* obj, uint8_t width)
{
    sgl_polygon_t *polygon = (sgl_polygon_t *)obj;
    if (polygon == NULL) {
        return;
    }
    
    polygon->border_width = width;
    sgl_obj_set_dirty(obj);
}

// Set alpha value
void sgl_polygon_set_alpha(sgl_obj_t* obj, uint8_t alpha)
{
    sgl_polygon_t *polygon = (sgl_polygon_t *)obj;
    if (polygon == NULL) {
        return;
    }
    
    polygon->alpha = alpha;
    sgl_obj_set_dirty(obj);
}

// Set background image
void sgl_polygon_set_pixmap(sgl_obj_t* obj, const sgl_pixmap_t* pixmap)
{
    sgl_polygon_t *polygon = (sgl_polygon_t *)obj;
    if (polygon == NULL) {
        return;
    }
    
    polygon->pixmap = pixmap;
    sgl_obj_set_dirty(obj);
}

// Set text
void sgl_polygon_set_text(sgl_obj_t* obj, const char* text)
{
    sgl_polygon_t *polygon = (sgl_polygon_t *)obj;
    if (polygon == NULL) {
        return;
    }
    
    polygon->text = text;
    sgl_obj_set_dirty(obj);
}

// Set font
void sgl_polygon_set_font(sgl_obj_t* obj, const sgl_font_t* font)
{
    sgl_polygon_t *polygon = (sgl_polygon_t *)obj;
    if (polygon == NULL) {
        return;
    }
    
    polygon->font = font;
    sgl_obj_set_dirty(obj);
}

// Set text color
void sgl_polygon_set_text_color(sgl_obj_t* obj, sgl_color_t color)
{
    sgl_polygon_t *polygon = (sgl_polygon_t *)obj;
    if (polygon == NULL) {
        return;
    }
    
    polygon->text_color = color;
    sgl_obj_set_dirty(obj);
}










