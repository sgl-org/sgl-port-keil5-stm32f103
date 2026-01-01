/* source/draw/sgl_draw_circle.c
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


/**
 * @brief Draw a circle
 * @param surf Surface
 * @param area Area of the circle
 * @param cx X coordinate of the center
 * @param cy Y coordinate of the center
 * @param radius Radius of the circle
 * @param color Color of the circle
 * @param alpha Alpha of the circle
 * @return none
 */
void sgl_draw_fill_circle(sgl_surf_t *surf, sgl_area_t *area, int16_t cx, int16_t cy, int16_t radius, sgl_color_t color, uint8_t alpha)
{
    sgl_area_t clip = SGL_AREA_MAX;
    sgl_color_t *buf = NULL, *blend = NULL;

    sgl_surf_clip_area_return(surf, area, &clip);

    sgl_area_t c_rect = {
        .x1 = cx - radius,
        .x2 = cx + radius,
        .y1 = cy - radius,
        .y2 = cy + radius
    };
    if (!sgl_area_selfclip(&clip, &c_rect)) {
        return;
    }

    int y2 = 0, real_r2 = 0, edge_alpha = 0;
    int r2 = sgl_pow2(radius);
    int r2_edge = sgl_pow2(radius + 1);

    buf = sgl_surf_get_buf(surf, clip.x1 - surf->x1, clip.y1 - surf->y1);
    for (int y = clip.y1; y <= clip.y2; y++) {
        blend = buf;
        y2 = sgl_pow2(y - cy);
        for (int x = clip.x1; x <= clip.x2; x++, blend++) {
            real_r2 = sgl_pow2(x - cx) + y2;

            if (real_r2 >= r2_edge) {
                if(x > cx)
                    break;
                continue;
            }
            else if (real_r2 >= r2) {
                edge_alpha = SGL_ALPHA_MAX - sgl_sqrt_error(real_r2);
                *blend = (alpha == SGL_ALPHA_MAX ? sgl_color_mixer(color, *blend, edge_alpha) : sgl_color_mixer(sgl_color_mixer(color, *blend, edge_alpha), *blend, alpha));
            }
            else {
                *blend = (alpha == SGL_ALPHA_MAX ? color : sgl_color_mixer(color, *blend, alpha));
            }
        }
        buf += surf->pitch;
    }
}


/**
 * @brief Draw a circle with pixmap and alpha
 * @param surf Surface
 * @param area Area of the circle
 * @param cx X coordinate of the center
 * @param cy Y coordinate of the center
 * @param radius Radius of the circle
 * @param pixmap Pixmap of image
 * @param alpha Alpha of the circle
 * @return none
 */
void sgl_draw_fill_circle_pixmap(sgl_surf_t *surf, sgl_area_t *area, int16_t cx, int16_t cy, int16_t radius, const sgl_pixmap_t *pixmap, uint8_t alpha)
{
    int y2 = 0, real_r2 = 0, s_x = cx - radius, s_y = cy - radius;
    int r2 = radius * radius;
    int r2_max = (radius + 1) * (radius + 1);
    sgl_color_t *buf = NULL, *pbuf = NULL, *blend = NULL;
    sgl_area_t clip = SGL_AREA_MAX;
    uint8_t edge_alpha = 0;

    sgl_surf_clip_area_return(surf, area, &clip);

    sgl_area_t c_rect = {
        .x1 = cx - radius,
        .x2 = cx + radius,
        .y1 = cy - radius,
        .y2 = cy + radius
    };
    if (!sgl_area_selfclip(&clip, &c_rect)) {
        return;
    }

    uint32_t scale_x = (pixmap->width << 10) / (radius * 2);
    uint32_t scale_y = (pixmap->height << 10) / (radius * 2);
    uint32_t step_x = 0, step_y = 0;

    buf = sgl_surf_get_buf(surf, clip.x1 - surf->x1, clip.y1 - surf->y1);
    for (int y = clip.y1; y <= clip.y2; y++) {
        blend = buf;
        y2 = sgl_pow2(y - cy);
        step_y = (scale_y * (y - s_y)) >> 10;

        for (int x = clip.x1; x <= clip.x2; x++, blend++) {
            real_r2 = sgl_pow2(x - cx) + y2;

            step_x = (scale_x * (x - s_x)) >> 10;
            pbuf = sgl_pixmap_get_buf(pixmap, step_x, step_y);

            if (real_r2 >= r2_max) {
                if(x > cx)
                    break;
                continue;
            }
            else if (real_r2 >= r2) {
                edge_alpha = SGL_ALPHA_MAX - sgl_sqrt_error(real_r2);
                *blend = (alpha == SGL_ALPHA_MAX ? sgl_color_mixer(*pbuf, *blend, edge_alpha) : sgl_color_mixer(sgl_color_mixer(*pbuf, *blend, edge_alpha), *blend, alpha));
            }
            else {
                *blend = (alpha == SGL_ALPHA_MAX ? *pbuf : sgl_color_mixer(*pbuf, *blend, alpha));
            }
        }
        buf += surf->pitch;
    }
}


/**
 * @brief Draw a circle with alpha and border
 * @param surf Surface
 * @param area Area of the circle
 * @param cx X coordinate of the center
 * @param cy Y coordinate of the center
 * @param radius Radius of the circle
 * @param color Color of the circle
 * @param border_color Color of the border
 * @param border_width Width of the border
 * @param alpha Alpha of the circle
 * @return none
 */
void sgl_draw_fill_circle_with_border(sgl_surf_t *surf, sgl_area_t *area, int16_t cx, int16_t cy, int16_t radius, sgl_color_t color, sgl_color_t border_color, int16_t border_width, uint8_t alpha)
{
    int radius_in = sgl_max(radius - border_width + 1, 0);
    int y2 = 0, real_r2 = 0;
    int in_r2 = sgl_pow2(radius_in);
    int out_r2 = sgl_pow2(radius);
    sgl_color_t *buf = NULL, *blend = NULL;

    int in_r2_max = sgl_pow2(radius_in - 1);
    int out_r2_max = sgl_pow2(radius + 1);
    uint8_t edge_alpha = 0;
    sgl_area_t clip = SGL_AREA_MAX;

    sgl_surf_clip_area_return(surf, area, &clip);

    buf = sgl_surf_get_buf(surf, clip.x1 - surf->x1, clip.y1 - surf->y1);
    for (int y = clip.y1; y <= clip.y2; y++) {
        blend = buf;
        y2 = sgl_pow2(y - cy);

        for (int x = clip.x1; x <= clip.x2; x++, blend++) {
            real_r2 = sgl_pow2(x - cx) + y2;

            if (real_r2 >= out_r2_max) {
                if (x > cx)
                    break;
                continue;
            }
            if (real_r2 < in_r2_max) {
                *blend = (alpha == SGL_ALPHA_MAX ? color : sgl_color_mixer(color, *blend, alpha));
                continue;
            }
            if (real_r2 < in_r2 ) {
                edge_alpha = sgl_sqrt_error(real_r2);
                *blend = (alpha == SGL_ALPHA_MAX ? sgl_color_mixer(border_color, color, sgl_sqrt_error(real_r2)) : sgl_color_mixer(sgl_color_mixer(border_color, color, edge_alpha), *blend, alpha));
            }
            else if (real_r2 > out_r2) {
                edge_alpha = SGL_ALPHA_MAX - sgl_sqrt_error(real_r2);
                *blend = (alpha == SGL_ALPHA_MAX ? sgl_color_mixer(border_color, *blend, edge_alpha) : sgl_color_mixer(sgl_color_mixer(border_color, *blend, edge_alpha), *blend, alpha));
            }
            else {
                *blend = (alpha == SGL_ALPHA_MAX ? border_color : sgl_color_mixer(border_color, *blend, alpha));
            }
        }
        buf += surf->pitch;
    }
}


/**
 * @brief draw task, the task contains the draw information and canvas
 * @param surf surface pointer
 * @param area the area of the task
 * @param desc the draw information
 * @return none
 */
void sgl_draw_circle(sgl_surf_t *surf, sgl_area_t *area, sgl_draw_circle_t *desc)
{
    sgl_color_t color = desc->color;
    int16_t radius = desc->radius;
    uint8_t alpha = desc->alpha;
    uint16_t border = desc->border;
    sgl_color_t border_color = desc->border_color;

    if (desc->pixmap == NULL) {
        if (border) {
            sgl_draw_fill_circle_with_border(surf, area, desc->cx, desc->cy, radius, color, border_color, border, alpha);
        }
        else {
            sgl_draw_fill_circle(surf, area, desc->cx, desc->cy, radius, color, alpha);
        }
    }
    else {
        sgl_draw_fill_circle_pixmap(surf, area, desc->cx, desc->cy, radius, desc->pixmap, alpha);
    }
}
