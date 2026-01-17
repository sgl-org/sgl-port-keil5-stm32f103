/* source/draw/sgl_draw_rect.c
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


/**
 * @brief fill a round rectangle with alpha
 * @param surf point to surface
 * @param area area of rectangle that you want to draw
 * @param rect point to rectangle that you want to draw
 * @param radius radius of round
 * @param color color of rectangle
 * @param alpha alpha of rectangle
 * @return none
 */
void sgl_draw_fill_rect(sgl_surf_t *surf, sgl_area_t *area, sgl_area_t *rect, int16_t radius, sgl_color_t color, uint8_t alpha)
{
    sgl_area_t clip;
    sgl_color_t *buf = NULL, *blend = NULL;
    uint8_t edge_alpha = 0;
    int cx1 = rect->x1 + radius;
    int cx2 = rect->x2 - radius;
    int cy1 = rect->y1 + radius;
    int cy2 = rect->y2 - radius;
    int cx_tmp = 0;
    int cy_tmp = 0;

    if (!sgl_surf_clip(surf, area, &clip)) {
        return;
    }

    if (!sgl_area_selfclip(&clip, rect)) {
        return;
    }

    int y2 = 0, real_r2 = 0;
    int r2 = sgl_pow2(radius);
    int r2_edge = sgl_pow2(radius + 1);

    buf = sgl_surf_get_buf(surf, clip.x1 - surf->x1, clip.y1 - surf->y1);
    if (radius == 0) {
        for (int y = clip.y1; y <= clip.y2; y++) {
            blend = buf;
            for (int x = clip.x1; x <= clip.x2; x++, blend++) {
                *blend = alpha == SGL_ALPHA_MAX ? color : sgl_color_mixer(color, *blend, alpha);
            }
            buf += surf->w;
        }
    }
    else {
        for (int y = clip.y1; y <= clip.y2; y++) {
            blend = buf;
            if (y > cy1 && y < cy2) {
                for (int x = clip.x1; x <= clip.x2; x++, blend++) {
                    *blend = (alpha == SGL_ALPHA_MAX ? color : sgl_color_mixer(color, *blend, alpha));
                }
            }
            else {
                cy_tmp = y > cy1 ? cy2 : cy1;
                y2 = sgl_pow2(y - cy_tmp);

                for (int x = clip.x1; x <= clip.x2; x++, blend++) {
                    if (x > cx1 && x < cx2) {
                        *blend = (alpha == SGL_ALPHA_MAX ? color : sgl_color_mixer(color, *blend, alpha));
                    }
                    else {
                        cx_tmp = x > cx1 ? cx2 : cx1;
                        real_r2 = sgl_pow2(x - cx_tmp) + y2;
                        if (real_r2 >= r2_edge) {
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
                }
            }
            buf += surf->w;
        }
    }
}


#include <math.h>
/**
 * @brief fill a round rectangle with alpha and border
 * @param surf point to surface
 * @param area area of rectangle that you want to draw
 * @param rect point to rectangle that you want to draw
 * @param radius radius of round
 * @param color color of rectangle
 * @param border_color color of border
 * @param border_width width of border
 * @param alpha alpha of rectangle
 * @return none
 */
void sgl_draw_fill_rect_with_border(sgl_surf_t *surf, sgl_area_t *area, sgl_area_t *rect, int16_t radius, sgl_color_t color, sgl_color_t border_color, uint8_t border_width, uint8_t alpha)
{
    int radius_in = sgl_max(radius - border_width + 1, 0);
    int y2 = 0, real_r2 = 0;
    int in_r2 = sgl_pow2(radius_in);
    int out_r2 = sgl_pow2(radius);
    sgl_color_t *buf = NULL, *blend = NULL;
    uint8_t edge_alpha = 0;

    int cx1 = rect->x1 + radius;
    int cx2 = rect->x2 - radius;
    int cy1 = rect->y1 + radius;
    int cy2 = rect->y2 - radius;
    int cx1i = rect->x1 + border_width;
    int cx2i = rect->x2 - border_width;
    int cyi1 = rect->y1 + border_width;
    int cyi2 = rect->y2 - border_width;
    int cx_tmp = 0;
    int cy_tmp = 0;

    int in_r2_max = sgl_pow2(radius_in - 1);
    int out_r2_max = sgl_pow2(radius + 1);

    sgl_area_t clip;

    if (!sgl_surf_clip(surf, area, &clip)) {
        return;
    }

    if (!sgl_area_selfclip(&clip, rect)) {
        return;
    }

    buf = sgl_surf_get_buf(surf, clip.x1 - surf->x1, clip.y1 - surf->y1);
    if (radius == 0) {
        for (int y = clip.y1; y <= clip.y2; y++) {
            blend = buf;
            for (int x = clip.x1; x <= clip.x2; x++, blend++) {
                if (x >= cx1i && x <= cx2i && y >= cyi1 && y <= cyi2) {
                    *blend = alpha == SGL_ALPHA_MAX ? color : sgl_color_mixer(color, *blend, alpha);
                }
                else {
                    *blend = alpha == SGL_ALPHA_MAX ? border_color : sgl_color_mixer(border_color, *blend, alpha);
                }
            }
            buf += surf->w;
        }
    }
    else {
        for (int y = clip.y1; y <= clip.y2; y++) {
            blend = buf;
            if (y > cy1 && y < cy2) {
                for (int x = clip.x1; x <= clip.x2; x++, blend++) {
                    if (x < cx1i || x > cx2i) {
                        *blend = (alpha == SGL_ALPHA_MAX ? border_color : sgl_color_mixer(border_color, *blend, alpha));
                    }
                    else {
                        *blend = (alpha == SGL_ALPHA_MAX ? color: sgl_color_mixer(color, *blend, alpha));
                    }
                }
            }
            else {
                cy_tmp = y > cy1 ? cy2 : cy1;
                y2 = sgl_pow2(y - cy_tmp);

                for (int x = clip.x1; x <= clip.x2; x++, blend++) {
                    if (x >= cx1 && x <= cx2) {
                        if (y < cyi1 || y > cyi2) {
                            *blend = (alpha == SGL_ALPHA_MAX ? border_color : sgl_color_mixer(border_color, *blend, alpha));
                        }
                        else {
                            *blend = (alpha == SGL_ALPHA_MAX ? color : sgl_color_mixer(color, *blend, alpha));
                        }
                    }
                    else {
                        cx_tmp = x > cx1 ? cx2 : cx1;
                        real_r2 = sgl_pow2(x - cx_tmp) + y2;

                        if (real_r2 >= out_r2_max) {
                            continue;
                        }
                        if (real_r2 < in_r2_max) {
                            *blend = (alpha == SGL_ALPHA_MAX ? color : sgl_color_mixer(color, *blend, alpha));
                            continue;
                        }
                        if (real_r2 < in_r2 ) {
                            edge_alpha = sgl_sqrt_error(real_r2);
                            *blend = (alpha == SGL_ALPHA_MAX ? sgl_color_mixer(border_color, color, edge_alpha) : sgl_color_mixer(sgl_color_mixer(border_color, color, edge_alpha), *blend, alpha));
                        }
                        else if (real_r2 > out_r2) {
                            edge_alpha = SGL_ALPHA_MAX - sgl_sqrt_error(real_r2);
                            *blend = (alpha == SGL_ALPHA_MAX ? sgl_color_mixer(border_color, *blend, edge_alpha) : sgl_color_mixer(sgl_color_mixer(border_color, *blend, edge_alpha), *blend, alpha));
                        }
                        else {
                            *blend = (alpha == SGL_ALPHA_MAX ? border_color : sgl_color_mixer(border_color, *blend, alpha));
                        }
                    }
                }
            }
            buf += surf->w;
        }
    }
}


/**
 * @brief fill a round rectangle pixmap with alpha
 * @param surf point to surface
 * @param area area of rectangle that you want to draw
 * @param rect point to rectangle that you want to draw
 * @param radius radius of round
 * @param pixmap pixmap of rectangle
 * @param alpha alpha of rectangle
 * @return none
 */
void sgl_draw_fill_rect_pixmap(sgl_surf_t *surf, sgl_area_t *area, sgl_area_t *rect, int16_t radius, const sgl_pixmap_t *pixmap, uint8_t alpha)
{
    sgl_area_t clip;
    sgl_color_t *buf = NULL, *blend = NULL;
    sgl_color_t *pbuf = NULL;
    uint8_t edge_alpha = 0;
    int cx1 = rect->x1 + radius;
    int cx2 = rect->x2 - radius;
    int cy1 = rect->y1 + radius;
    int cy2 = rect->y2 - radius;
    int cx_tmp = 0;
    int cy_tmp = 0;

    if (!sgl_surf_clip(surf, area, &clip)) {
        return;
    }

    if (!sgl_area_selfclip(&clip, rect)) {
        return;
    }

    int y2 = 0, real_r2 = 0;
    int r2 = sgl_pow2(radius);
    int r2_edge = sgl_pow2(radius + 1);
    uint32_t scale_x = ((pixmap->width << 10) / (rect->x2 - rect->x1 + 1));
    uint32_t scale_y = ((pixmap->height << 10) / (rect->y2 - rect->y1 + 1));
    uint32_t step_x = 0, step_y = 0;

    buf = sgl_surf_get_buf(surf, clip.x1 - surf->x1, clip.y1 - surf->y1);
    if (radius == 0) {
        for (int y = clip.y1; y <= clip.y2; y++) {
            blend = buf;
            step_y = (scale_y * (y - rect->y1)) >> 10;
            for (int x = clip.x1; x <= clip.x2; x++, blend++) {
                step_x = (scale_x * (x - rect->x1)) >> 10;
                pbuf = sgl_pixmap_get_buf(pixmap, step_x, step_y);
                *blend = (alpha == SGL_ALPHA_MAX ? *pbuf : sgl_color_mixer(*pbuf, *blend, alpha));
            }
            buf += surf->w;
        }
    }
    else {
        for (int y = clip.y1; y <= clip.y2; y++) {
            blend = buf;
            step_y = (scale_y * (y - rect->y1)) >> 10;
            if (y > cy1 && y < cy2) {
                for (int x = clip.x1; x <= clip.x2; x++, blend++) {
                    step_x = (scale_x * (x - rect->x1)) >> 10;
                    pbuf = sgl_pixmap_get_buf(pixmap, step_x, step_y);
                    *blend = (alpha == SGL_ALPHA_MAX ? *pbuf : sgl_color_mixer(*pbuf, *blend, alpha));
                }
            }
            else {
                cy_tmp = y > cy1 ? cy2 : cy1;
                y2 = sgl_pow2(y - cy_tmp);

                for (int x = clip.x1; x <= clip.x2; x++, blend++) {
                    step_x = (scale_x * (x - rect->x1)) >> 10;
                    pbuf = sgl_pixmap_get_buf(pixmap, step_x, step_y);

                    if(x > cx1 && x < cx2) {
                        *blend = (alpha == SGL_ALPHA_MAX ? *pbuf : sgl_color_mixer(*pbuf, *blend, alpha));
                    }
                    else {
                        cx_tmp = x > cx1 ? cx2 : cx1;
                        real_r2 = sgl_pow2(x - cx_tmp) + y2;
                        if (real_r2 >= r2_edge) {
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
                }
            }
            buf += surf->w;
        }
    }
}


/**
 * @brief fill a round rectangle with alpha
 * @param surf point to surface
 * @param area area of rectangle that you want to draw
 * @param rect point to rectangle that you want to draw
 * @param desc rectangle description
 * @return none
 */
void sgl_draw_rect(sgl_surf_t *surf, sgl_area_t *area, sgl_rect_t *rect, sgl_draw_rect_t *desc)
{
    if (desc->pixmap == NULL) {
        if (desc->border == 0) {
            sgl_draw_fill_rect(surf, area, rect, desc->radius, desc->color, desc->alpha);
        }
        else {
            sgl_draw_fill_rect_with_border(surf, area, rect, desc->radius, desc->color, desc->border_color, desc->border, desc->alpha);
        }
    }
    else {
        sgl_draw_fill_rect_pixmap(surf, area, rect, desc->radius, desc->pixmap, desc->alpha);
    }
}
