/* source/draw/sgl_draw_line.c
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
#include <sgl_log.h>
#include <sgl_draw.h>
#include <sgl_math.h>

/**
 * @brief draw a horizontal line with alpha
 * @param surf surface
 * @param area area that contains the line
 * @param y line y position
 * @param x1 line start x position
 * @param x2 line end x position
 * @param color line color
 * @param alpha alpha of color
 * @return none
 */
void sgl_draw_fill_hline(sgl_surf_t *surf, sgl_area_t *area, int16_t y, int16_t x1, int16_t x2, uint8_t width, sgl_color_t color, uint8_t alpha)
{
	sgl_color_t *buf = NULL, *blend = NULL;
	sgl_area_t c_rect = {.x1 = x1, .x2 = x2, .y1 = y - (width - 1) / 2, .y2 = y + width / 2}, clip = SGL_AREA_MAX;

	if (c_rect.x1 > c_rect.x2) {
		sgl_swap(&c_rect.x1, &c_rect.x2);
	}

	sgl_surf_clip_area_return(surf, area, &clip);
	if (!sgl_area_selfclip(&clip, &c_rect)) {
		return;
	}

    buf = sgl_surf_get_buf(surf,  clip.x1 - surf->x1, clip.y1 - surf->y1);
    for (int y = clip.y1; y <= clip.y2; y++) {
		blend = buf;
        for (int x = clip.x1; x <= clip.x2; x++, blend++) {
            *blend = alpha == SGL_ALPHA_MAX ? color : sgl_color_mixer(color, *blend, alpha);
        }
		buf += surf->w;
    }
}


/**
 * @brief draw a vertical line with alpha
 * @param surf surface
 * @param area area that contains the line
 * @param x x coordinate
 * @param y1 y1 coordinate
 * @param y2 y2 coordinate
 * @param color line color
 * @param alpha alpha of color
 * @return none
 */
void sgl_draw_fill_vline(sgl_surf_t *surf, sgl_area_t *area, int16_t x, int16_t y1, int16_t y2, uint8_t width, sgl_color_t color, uint8_t alpha)
{
	sgl_color_t *buf = NULL, *blend = NULL;
	sgl_area_t c_rect = {.x1 = x - (width - 1) / 2, .x2 = x + width / 2, .y1 = y1,.y2 = y2}, clip = SGL_AREA_MAX;

	if (c_rect.y1 > c_rect.y2) {
		sgl_swap(&c_rect.y1, &c_rect.y2);
	}

	sgl_surf_clip_area_return(surf, area, &clip);
	if (!sgl_area_selfclip(&clip, &c_rect)) {
		return;
	}

    buf = sgl_surf_get_buf(surf,  clip.x1 - surf->x1, clip.y1 - surf->y1);
    for (int y = clip.y1; y <= clip.y2; y++) {
		blend = buf;
        for (int x = clip.x1; x <= clip.x2; x++, blend++) {
            *blend = (alpha == SGL_ALPHA_MAX ? color : sgl_color_mixer(color, *blend, alpha));
        }
        buf += surf->w;
    }
}

/**
 * SDF draw anti-aliased line
 * @param thickness line thickness (in pixels)
 */
static int32_t sgl_capsule_sdf_optimized(int16_t px, int16_t py, int16_t ax, int16_t ay, int16_t bx, int16_t by)
{
	int64_t pax = px - ax, pay = py - ay, bax = bx - ax, bay = by - ay;
	int64_t b_sqd = bax * bax + bay * bay;
	int64_t h = (sgl_max(sgl_min((pax * bax + pay * bay), b_sqd), 0)) << 8;
	int64_t dx = (pax << 8) - bax * h / b_sqd;
	int64_t dy = (pay << 8) - bay * h / b_sqd;

	return sgl_sqrt(dx * dx + dy * dy);
}


/**
 * @brief draw a slanted line with alpha
 * @param surf surface
 * @param area area that contains the line
 * @param x1 line start x position
 * @param y1 line start y position
 * @param x2 line end x position
 * @param y2 line end y position
 * @param thickness line width
 * @param color line color
 * @param alpha alpha of color
 * @return none
 * @note This algorithm is SDF algorithm
 */
void draw_line_fill_slanted(sgl_surf_t *surf, sgl_area_t *area, int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t thickness, sgl_color_t color, uint8_t alpha)
{
	uint8_t c;
	int64_t len;
	sgl_area_t clip = SGL_AREA_MAX;
	sgl_color_t *buf = NULL, *blend = NULL;
	int16_t thick_half = (thickness >> 1);
	sgl_area_t c_rect = {.x1 = x1 - thick_half, .x2 = x2 + thick_half, .y1 = y1 - thick_half,.y2 = y2 + thick_half};

	if (c_rect.x1 > c_rect.x2) {
		sgl_swap(&c_rect.x1, &c_rect.x2);
	}
	if (c_rect.y1 > c_rect.y2) {
		sgl_swap(&c_rect.y1, &c_rect.y2);
	}

	sgl_surf_clip_area_return(surf, area, &clip);
	if (!sgl_area_selfclip(&clip, &c_rect)) {
		return;
	}

	buf = sgl_surf_get_buf(surf, clip.x1 - surf->x1, clip.y1 - surf->y1);
	for (int y = clip.y1; y <= clip.y2; y++) {
		blend = buf;

		for (int x = clip.x1; x <= clip.x2; x++, blend++) {
			len = sgl_capsule_sdf_optimized(x, y, x1, y1, x2, y2);

			if (len <= (thick_half - 1) << 8) {
				*blend = (alpha == SGL_ALPHA_MAX ? color : sgl_color_mixer(color, *blend, alpha));
				continue;
			}

			if (len > ((thick_half - 1) << 8) && len < (thick_half << 8)) {
				c = len - ((thick_half - 1) << 8);

				if (alpha == SGL_ALPHA_MAX)
					*blend = sgl_color_mixer(*blend, color, c);
				else
					*blend = sgl_color_mixer(sgl_color_mixer(*blend, color, c), *blend, alpha);
			}
		}
		buf += surf->w;
	}
}


/**
 * @brief draw a line
 * @param surf surface
 * @param area area that contains the line
 * @param coords line coords
 * @param desc line description
 * @return none
 */
void sgl_draw_line(sgl_surf_t *surf, sgl_area_t *area, sgl_area_t *coords, sgl_draw_line_t *desc)
{
	uint8_t alpha = desc->alpha;
	int16_t x1 = coords->x1;
	int16_t y1 = coords->y1;
	int16_t x2 = coords->x2;
	int16_t y2 = coords->y2;

	if (x1 == x2) {
		sgl_draw_fill_vline(surf, area, x1, y1, y2, desc->width / 2, desc->color, alpha);
	}
	else if (y1 == y2) {
		sgl_draw_fill_hline(surf, area, y1, x1, x2, desc->width / 2, desc->color, alpha);
	}
	else {
		draw_line_fill_slanted(surf, area, x1, y1, x2, y2, desc->width, desc->color, alpha);
	}
}
