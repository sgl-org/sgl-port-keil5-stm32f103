/* source/widgets/sgl_ext_img.c
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
#include <sgl_theme.h>
#include <sgl_cfgfix.h>
#include <string.h>
#include "sgl_ext_img.h"

static inline void ext_img_rle_init(sgl_ext_img_t *img)
{
    SGL_ASSERT(img != NULL);
    img->index = 0;
    img->remainder = 0;
}

static inline void rle_decompress_line(sgl_ext_img_t *img, sgl_area_t *coords, sgl_area_t *area, sgl_color_t *out)
{
    uint8_t tmp_buf[8];
    uint8_t* read_ptr = NULL;
    uint8_t* start_ptr = (uint8_t*)img->pixmap[img->pixmap_idx].bitmap.array;
    uintptr_t start_addr = img->pixmap[img->pixmap_idx].bitmap.addr;
    uint8_t format = img->pixmap->format;
    uint32_t pix_value;

    for (int i = coords->x1; i <= coords->x2; i++) {
        if (img->remainder == 0) {
            if (img->read != NULL) {
                read_ptr = tmp_buf;
                img->read(start_addr + img->index, tmp_buf, sizeof(tmp_buf));
            }
            else {
                read_ptr = start_ptr + img->index;
            }

            img->index ++;
            img->remainder = read_ptr[0];

            switch (format) {
            case SGL_PIXMAP_FMT_RLE_RGB332:
                pix_value = read_ptr[1];
                img->color = sgl_rgb332_to_color(pix_value);
                img->pix_alpha = SGL_ALPHA_MAX;
                img->index ++;
                break;
            case SGL_PIXMAP_FMT_RLE_ARGB2222:
                pix_value = read_ptr[1];
                img->color = sgl_rgb222_to_color(pix_value);
                img->pix_alpha = sgl_opa2_table[pix_value >> 6];
                img->index ++;
                break;
            case SGL_PIXMAP_FMT_RLE_RGB565:
                pix_value = read_ptr[1] | (read_ptr[2] << 8);
                img->color = sgl_rgb565_to_color(pix_value);
                img->pix_alpha = SGL_ALPHA_MAX;
                img->index += 2;
                break;
            case SGL_PIXMAP_FMT_RLE_ARGB4444:
                pix_value = read_ptr[1] | (read_ptr[2] << 8);
                img->color = sgl_rgb444_to_color(pix_value);
                img->pix_alpha = sgl_opa4_table[pix_value >> 12];
                img->index += 2;
                break;
            case SGL_PIXMAP_FMT_RLE_RGB888:
                pix_value = read_ptr[1] | (read_ptr[2] << 8) | (read_ptr[3] << 16);
                img->color = sgl_rgb888_to_color(pix_value);
                img->pix_alpha = SGL_ALPHA_MAX;
                img->index += 3;
                break;
            case SGL_PIXMAP_FMT_RLE_ARGB8888:
                pix_value = read_ptr[1] | (read_ptr[2] << 8) | (read_ptr[3] << 16);
                img->color = sgl_rgb888_to_color(pix_value);
                img->pix_alpha = read_ptr[4];
                img->index += 4;
                break;
            default:
                break;
            }
        }

        if (out != NULL && i >= area->x1 && i <= area->x2) {
            img->color = (img->pix_alpha == SGL_ALPHA_MAX ? img->color : sgl_color_mixer(img->color, *out, img->pix_alpha));
            *out = (img->alpha == SGL_ALPHA_MAX ? img->color : sgl_color_mixer(img->color, *out, img->alpha));
            out ++;
        }
        img->remainder --;
    };
}


static void sgl_ext_img_construct_cb(sgl_surf_t *surf, sgl_obj_t* obj, sgl_event_t *evt)
{
    sgl_area_t clip = SGL_AREA_INVALID;
    sgl_ext_img_t *ext_img = sgl_container_of(obj, sgl_ext_img_t, obj);
    const sgl_pixmap_t *pixmap = &ext_img->pixmap[ext_img->pixmap_idx];
    uintptr_t read_addr = pixmap->bitmap.addr;
    uint8_t pix_byte = sgl_pixmal_get_bytes_per_pixel(pixmap);
    sgl_color_t tmp_color, *buf = NULL, *blend = NULL;
    size_t pix_value = 0;
    size_t offset = 0;

    sgl_area_t area = {
        .x1 = obj->coords.x1,
        .y1 = obj->coords.y1,
        .x2 = obj->coords.x1 + pixmap->width - 1,
        .y2 = obj->coords.y1 + pixmap->height - 1,
    };

    if(evt->type == SGL_EVENT_DRAW_MAIN) {
        if (!sgl_surf_clip(surf, &obj->area, &clip)) {
            return;
        }

        if (pixmap->format < SGL_PIXMAP_FMT_RLE_RGB332) {
            buf = sgl_surf_get_buf(surf, clip.x1 - surf->x1, clip.y1 - surf->y1);

            uint8_t *pixmap_buf = (uint8_t*)pixmap->bitmap.array;
            if(ext_img->read != NULL){
                pixmap_buf = (uint8_t*)sgl_malloc(pix_byte * (clip.x2 - clip.x1 + 1));
            }

            for (int y = clip.y1; y <= clip.y2; y++) {
                blend = buf;
                offset = ((((y - area.y1) * pixmap->width) + (clip.x1 - area.x1)) * pix_byte);
                if(ext_img->read != NULL) {
                    ext_img->read(read_addr + offset, pixmap_buf, pix_byte * (clip.x2 - clip.x1 + 1));
                    offset = 0;
                }
                for (int x = clip.x1; x <= clip.x2; x++) {
                    switch (pixmap->format) {
                    case SGL_PIXMAP_FMT_RGB332:
                        pix_value = pixmap_buf[offset];
                        tmp_color = sgl_rgb332_to_color(pix_value);
                        break;
                    case SGL_PIXMAP_FMT_RGB565:
                        pix_value = pixmap_buf[offset] | (pixmap_buf[offset + 1] << 8);
                        tmp_color = sgl_rgb565_to_color(pix_value);
                        break;
                    case SGL_PIXMAP_FMT_ARGB2222:
                        pix_value = pixmap_buf[offset];
                        tmp_color = sgl_rgb222_to_color(pix_value);
                        tmp_color = sgl_color_mixer(tmp_color, *blend, sgl_opa2_table[pix_value >> 6]);
                        break;
                    case SGL_PIXMAP_FMT_ARGB4444:
                        pix_value = pixmap_buf[offset] | (pixmap_buf[offset + 1] << 8);
                        tmp_color = sgl_rgb444_to_color(pix_value);
                        tmp_color = sgl_color_mixer(tmp_color, *blend, sgl_opa4_table[pix_value >> 12]);
                        break;
                    case SGL_PIXMAP_FMT_RGB888:
                        pix_value = pixmap_buf[offset] | (pixmap_buf[offset + 1] << 8) | (pixmap_buf[offset + 2] << 16);
                        tmp_color = sgl_rgb888_to_color(pix_value);
                        break;
                    case SGL_PIXMAP_FMT_ARGB8888:
                        pix_value = pixmap_buf[offset] | (pixmap_buf[offset + 1] << 8) | (pixmap_buf[offset + 2] << 16);
                        tmp_color = sgl_rgb888_to_color(pix_value);
                        tmp_color = sgl_color_mixer(tmp_color, *blend, pixmap_buf[offset + 3]);
                        break;
                    default:
                        break;
                    }
                    *blend = ext_img->alpha == SGL_ALPHA_MAX ? tmp_color : sgl_color_mixer(tmp_color, *blend, ext_img->alpha);
                    offset += pix_byte;
                    blend ++;
                };
                buf += surf->w;
            }
            if(ext_img->read != NULL) {
                sgl_free(pixmap_buf);
            }
        }
        else {
            /* RLE pixmap support */
            if (clip.y1 == surf->dirty->y1 || clip.y1 == obj->area.y1) {
                ext_img->index = 0;
                ext_img->remainder = 0;
                for (int y = area.y1; y < clip.y1; y++) {
                    rle_decompress_line(ext_img, &area, &clip, NULL);
                }
            }

            buf = sgl_surf_get_buf(surf, clip.x1 - surf->x1, (clip.y1 - surf->y1));

            for (int y = clip.y1; y <= clip.y2; y++) {
                rle_decompress_line(ext_img, &area, &clip, buf);
                buf += surf->w;
            }
        }

        if (ext_img->pixmap_auto && (clip.y2 == surf->dirty->y2 || clip.y2 == obj->area.y2)) {
            uint32_t pixmap_idx = ext_img->pixmap_idx + 1;
            ext_img->pixmap_idx = pixmap_idx >= ext_img->pixmap_num ? 0 : pixmap_idx;
            sgl_obj_set_dirty(obj);
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
    ext_img->pixmap_idx = 0;
    ext_img->pixmap_num = 1;
    ext_img->pixmap_auto = 0;

    return obj;
}
