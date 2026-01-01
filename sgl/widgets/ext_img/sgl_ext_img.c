/* source/widgets/sgl_ext_img.c
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
#include "sgl_ext_img.h"

static inline void ext_img_rle_init(sgl_ext_img_t *img)
{
    SGL_ASSERT(img != NULL);
    if (img->started == 0) {
        img->started = 1;
        img->index = 0;
        img->remainder = 0;
    }
}

static inline void rle_decompress_line(sgl_ext_img_t *img, sgl_area_t *coords, sgl_area_t *area, sgl_color_t *out)
{
    uint8_t tmp_buf[8] = {0};
    const uint8_t *bitmap = img->pixmap->bitmap;
    uint8_t format = img->pixmap->format;

    for (int i = coords->x1; i <= coords->x2; i++) {
        if (img->remainder == 0) {
            if (img->read != NULL) {
                img->read(bitmap + img->index, tmp_buf, sizeof(tmp_buf));
            }
            else {
                tmp_buf[0] = bitmap[img->index];
                tmp_buf[1] = bitmap[img->index + 1];
                tmp_buf[2] = bitmap[img->index + 2];
                tmp_buf[3] = bitmap[img->index + 3];
                tmp_buf[4] = bitmap[img->index + 4];
            }

            img->index ++;
            img->remainder = tmp_buf[0];

            if (format == SGL_PIXMAP_FMT_RLE_RGB332) {
                img->color = sgl_rgb332_to_color(tmp_buf[1]);
                img->index ++;
            }
            else if (format == SGL_PIXMAP_FMT_RLE_RGB565) {
                img->color = sgl_rgb565_to_color(tmp_buf[1] | (tmp_buf[2] << 8));
                img->index += 2;
            }
            else if (format == SGL_PIXMAP_FMT_RLE_RGB888) {
                img->color = sgl_rgb888_to_color(tmp_buf[1] | (tmp_buf[2] << 8) | (tmp_buf[3] << 16));
                img->index += 3;
            }
        }

        if (out != NULL && i >= area->x1 && i <= area->x2) {
            *out = (img->alpha == SGL_ALPHA_MAX ? img->color : sgl_color_mixer(img->color, *out, img->alpha));
            out ++;
        }
        img->remainder --;
    };
}


static void sgl_ext_img_construct_cb(sgl_surf_t *surf, sgl_obj_t* obj, sgl_event_t *evt)
{
    sgl_area_t clip = SGL_AREA_INVALID;
    sgl_ext_img_t *ext_img = (sgl_ext_img_t*)obj;
    const uint8_t *bitmap = ext_img->pixmap->bitmap;
    uint8_t pix_byte = sgl_pixmal_get_bits(ext_img->pixmap);

    sgl_area_t area = {
        .x1 = obj->coords.x1,
        .y1 = obj->coords.y1,
        .x2 = obj->coords.x1 + ext_img->pixmap->width - 1,
        .y2 = obj->coords.y1 + ext_img->pixmap->height - 1,
    };

    if(evt->type == SGL_EVENT_DRAW_MAIN) {
        sgl_color_t *buf = NULL, *blend = NULL;

        if (!sgl_surf_clip(surf, &obj->area, &clip)) {
            return;
        }
    
        if (ext_img->pixmap->format < SGL_PIXMAP_FMT_RLE_RGB332) {

            buf = sgl_surf_get_buf(surf, clip.x1 - surf->x1, clip.y1 - surf->y1);
            if (ext_img->read != NULL) {
                uint8_t *pixmap_buf = (uint8_t*)sgl_malloc(pix_byte * (clip.x2 - clip.x1 + 1));
                sgl_color_t tmp_color;
                uint32_t offset = 0, line_ofs = 0;

                for (int y = clip.y1; y <= clip.y2; y++) {
                    blend = buf;
                    offset = ((((y - area.y1) * ext_img->pixmap->width) + (clip.x1 - area.x1)) * pix_byte);

                    ext_img->read(bitmap + offset, pixmap_buf, pix_byte * (clip.x2 - clip.x1 + 1));
                    line_ofs = 0;

                    for (int x = clip.x1; x <= clip.x2; x++) {
                        if (ext_img->pixmap->format == SGL_PIXMAP_FMT_RGB332) {
                            tmp_color = sgl_rgb332_to_color(pixmap_buf[line_ofs]);
                        }
                        else if (ext_img->pixmap->format == SGL_PIXMAP_FMT_RGB565) {
                            tmp_color = sgl_rgb565_to_color(pixmap_buf[line_ofs] | (pixmap_buf[line_ofs + 1] << 8));
                        }
                        else if (ext_img->pixmap->format == SGL_PIXMAP_FMT_RGB888) {
                            tmp_color = sgl_rgb888_to_color(pixmap_buf[line_ofs] | (pixmap_buf[line_ofs + 1] << 8) | (pixmap_buf[line_ofs + 2] << 16));
                        }

                        line_ofs += pix_byte;
                        *blend = ext_img->alpha == SGL_ALPHA_MAX ? tmp_color : sgl_color_mixer(tmp_color, *blend, ext_img->alpha);
                        blend ++;
                    }
                    buf += surf->pitch;
                }
                sgl_free(pixmap_buf);
            }
            else {
                sgl_color_t tmp_color;
                uint32_t offset = 0;

                for (int y = clip.y1; y <= clip.y2; y++) {
                    blend = buf;
                    offset = ((((y - area.y1) * ext_img->pixmap->width) + (clip.x1 - area.x1)) * pix_byte);

                    for (int x = clip.x1; x <= clip.x2; x++) {
                        if (ext_img->pixmap->format == SGL_PIXMAP_FMT_RGB332) {
                            tmp_color = sgl_rgb332_to_color(bitmap[offset]);
                        }
                        else if (ext_img->pixmap->format == SGL_PIXMAP_FMT_RGB565) {
                            tmp_color = sgl_rgb565_to_color(bitmap[offset] | (bitmap[offset + 1] << 8));
                        }
                        else if (ext_img->pixmap->format == SGL_PIXMAP_FMT_RGB888) {
                            tmp_color = sgl_rgb888_to_color(bitmap[offset] | (bitmap[offset + 1] << 8) | (bitmap[offset + 2] << 16));
                        }

                        offset += pix_byte;
                        *blend = ext_img->alpha == SGL_ALPHA_MAX ? tmp_color : sgl_color_mixer(tmp_color, *blend, ext_img->alpha);
                        blend ++;
                    };
                    buf += surf->pitch;
                }
            }
        }
        else {
            /* RLE pixmap support */
            buf = sgl_surf_get_buf(surf, clip.x1 - surf->x1, (clip.y1 - surf->y1));
            clip.y2 = sgl_min(clip.y2, area.y2);
            ext_img_rle_init(ext_img);
            for (int y = clip.y1; y <= clip.y2; y++) {
                rle_decompress_line(ext_img, &area, &clip, buf);
                buf += surf->pitch;
            }
            if (surf->y2 >= area.y2) {
                ext_img->started = 0;
            }
        }
    }
}


/**
 * @brief create an ext_img object
 * @param parent parent of the ext_img
 * @return ext_img object
 */
sgl_obj_t* sgl_ext_img_create(sgl_obj_t* parent)
{
    sgl_ext_img_t *ext_img = sgl_malloc(sizeof(sgl_ext_img_t));
    if(ext_img == NULL) {
        SGL_LOG_ERROR("sgl_ext_img_create: malloc failed");
        return NULL;
    }

    /* set object all member to zero */
    memset(ext_img, 0, sizeof(sgl_ext_img_t));

    sgl_obj_t *obj = &ext_img->obj;
    sgl_obj_init(&ext_img->obj, parent);
    obj->construct_fn = sgl_ext_img_construct_cb;

    ext_img->alpha = SGL_ALPHA_MAX;
    ext_img->started = 0;

    return obj;
}
