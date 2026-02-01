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


#if (!CONFIG_SGL_PIXMAP_BILINEAR_INTERP)
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
    uint32_t scale_x = ((pixmap->width << SGL_FIXED_SHIFT) / (rect->x2 - rect->x1 + 1));
    uint32_t scale_y = ((pixmap->height << SGL_FIXED_SHIFT) / (rect->y2 - rect->y1 + 1));
    uint32_t step_x = 0, step_y = 0;

    buf = sgl_surf_get_buf(surf, clip.x1 - surf->x1, clip.y1 - surf->y1);
    if (radius == 0) {
        for (int y = clip.y1; y <= clip.y2; y++) {
            blend = buf;
            step_y = (scale_y * (y - rect->y1)) >> SGL_FIXED_SHIFT;
            for (int x = clip.x1; x <= clip.x2; x++, blend++) {
                step_x = (scale_x * (x - rect->x1)) >> SGL_FIXED_SHIFT;
                pbuf = sgl_pixmap_get_buf(pixmap, step_x, step_y);
                *blend = (alpha == SGL_ALPHA_MAX ? *pbuf : sgl_color_mixer(*pbuf, *blend, alpha));
            }
            buf += surf->w;
        }
    }
    else {
        for (int y = clip.y1; y <= clip.y2; y++) {
            blend = buf;
            step_y = (scale_y * (y - rect->y1)) >> SGL_FIXED_SHIFT;
            if (y > cy1 && y < cy2) {
                for (int x = clip.x1; x <= clip.x2; x++, blend++) {
                    step_x = (scale_x * (x - rect->x1)) >> SGL_FIXED_SHIFT;
                    pbuf = sgl_pixmap_get_buf(pixmap, step_x, step_y);
                    *blend = (alpha == SGL_ALPHA_MAX ? *pbuf : sgl_color_mixer(*pbuf, *blend, alpha));
                }
            }
            else {
                cy_tmp = y > cy1 ? cy2 : cy1;
                y2 = sgl_pow2(y - cy_tmp);

                for (int x = clip.x1; x <= clip.x2; x++, blend++) {
                    step_x = (scale_x * (x - rect->x1)) >> SGL_FIXED_SHIFT;
                    pbuf = sgl_pixmap_get_buf(pixmap, step_x, step_y);

                    if (x > cx1 && x < cx2) {
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

#else
/**
 * @brief calculate a point color by bilinear interpolate
 * @param pixmap point to pixmap
 * @param fx x coordinate of point
 * @param fy y coordinate of point
 * @return point color
 */
static sgl_color_t sgl_bilinear_interpolate(const sgl_pixmap_t *pixmap, int32_t fx, int32_t fy)
{
    sgl_color_t result;
    int32_t max_x = ((int32_t)pixmap->width - 1) << SGL_FIXED_SHIFT;
    int32_t max_y = ((int32_t)pixmap->height - 1) << SGL_FIXED_SHIFT;
    fx = fx < 0 ? 0 : (fx > max_x ? max_x : fx);
    fy = fy < 0 ? 0 : (fy > max_y ? max_y : fy);

    int32_t x0 = fx >> SGL_FIXED_SHIFT;
    int32_t y0 = fy >> SGL_FIXED_SHIFT;
    int32_t dx = fx & SGL_FIXED_MASK;
    int32_t dy = fy & SGL_FIXED_MASK;
    int32_t dx1 = SGL_FIXED_ONE - dx;
    int32_t dy1 = SGL_FIXED_ONE - dy;

    sgl_color_t *c00 = sgl_pixmap_get_buf(pixmap, x0, y0);
    sgl_color_t *c01 = sgl_pixmap_get_buf(pixmap, x0, y0 + 1);
    sgl_color_t *c10 = sgl_pixmap_get_buf(pixmap, x0 + 1, y0);
    sgl_color_t *c11 = sgl_pixmap_get_buf(pixmap, x0 + 1, y0 + 1);

    // bilinear interpolate expression
    // color = c00*dx1*dy1 + c01*dx1*dy + c10*dx*dy1 + c11*dx*dy
    #define INTERP_VAL(c00, c01, c10, c11) \
            (((c00 * dx1 * dy1) + (c01 * dx1 * dy) + (c10 * dx * dy1) + (c11 * dx * dy)) >> (2 * SGL_FIXED_SHIFT))

    result.ch.red   = (uint8_t)INTERP_VAL(c00->ch.red, c01->ch.red, c10->ch.red, c11->ch.red);
    result.ch.green = (uint8_t)INTERP_VAL(c00->ch.green, c01->ch.green, c10->ch.green, c11->ch.green);
    result.ch.blue  = (uint8_t)INTERP_VAL(c00->ch.blue, c01->ch.blue, c10->ch.blue, c11->ch.blue);

    #undef INTERP_VAL
    return result;
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
    sgl_color_t interpolated_color;
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

    int32_t rect_w = rect->x2 - rect->x1 + 1;
    int32_t rect_h = rect->y2 - rect->y1 + 1;
    int32_t scale_x = ((int32_t)pixmap->width << SGL_FIXED_SHIFT) / rect_w;
    int32_t scale_y = ((int32_t)pixmap->height << SGL_FIXED_SHIFT) / rect_h;

    buf = sgl_surf_get_buf(surf, clip.x1 - surf->x1, clip.y1 - surf->y1);
    
    if (radius == 0) {
        for (int y = clip.y1; y <= clip.y2; y++) {
            blend = buf;
            int32_t fy = (int32_t)(y - rect->y1) * scale_y;

            for (int x = clip.x1; x <= clip.x2; x++, blend++) {
                int32_t fx = (int32_t)(x - rect->x1) * scale_x;
                interpolated_color = sgl_bilinear_interpolate(pixmap, fx, fy);
                *blend = (alpha == SGL_ALPHA_MAX) ? interpolated_color : sgl_color_mixer(interpolated_color, *blend, alpha);
            }
            buf += surf->w;
        }
    }
    else {
        for (int y = clip.y1; y <= clip.y2; y++) {
            blend = buf;
            int32_t fy = (int32_t)(y - rect->y1) * scale_y;
            
            if (y > cy1 && y < cy2) {
                for (int x = clip.x1; x <= clip.x2; x++, blend++) {
                    int32_t fx = (int32_t)(x - rect->x1) * scale_x;
                    interpolated_color = sgl_bilinear_interpolate(pixmap, fx, fy);
                    *blend = (alpha == SGL_ALPHA_MAX) ? interpolated_color : sgl_color_mixer(interpolated_color, *blend, alpha);
                }
            }
            else {
                cy_tmp = y > cy1 ? cy2 : cy1;
                y2 = sgl_pow2(y - cy_tmp);

                for (int x = clip.x1; x <= clip.x2; x++, blend++) {
                    int32_t fx = (int32_t)(x - rect->x1) * scale_x;
                    interpolated_color = sgl_bilinear_interpolate(pixmap, fx, fy);

                    if (x > cx1 && x < cx2) {
                        *blend = (alpha == SGL_ALPHA_MAX) ? interpolated_color : sgl_color_mixer(interpolated_color, *blend, alpha);
                    }
                    else {
                        cx_tmp = x > cx1 ? cx2 : cx1;
                        real_r2 = sgl_pow2(x - cx_tmp) + y2;
                        if (real_r2 >= r2_edge) {
                            continue;
                        }
                        else if (real_r2 >= r2) {
                            edge_alpha = SGL_ALPHA_MAX - sgl_sqrt_error(real_r2);
                            *blend = (alpha == SGL_ALPHA_MAX) ? sgl_color_mixer(interpolated_color, *blend, edge_alpha) : 
                                                                sgl_color_mixer(sgl_color_mixer(interpolated_color, *blend, edge_alpha), *blend, alpha);
                        }
                        else {
                            *blend = (alpha == SGL_ALPHA_MAX) ? interpolated_color : sgl_color_mixer(interpolated_color, *blend, alpha);
                        }
                    }
                }
            }
            buf += surf->w;
        }
    }
}
#endif


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
    if (unlikely(desc->alpha == SGL_ALPHA_MIN)) {
        return;
    }

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
