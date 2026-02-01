/* source/widgets/sgl_2dball.c
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
#include "sgl_2dball.h"


static void sgl_2dball_construct_cb(sgl_surf_t *surf, sgl_obj_t* obj, sgl_event_t *evt)
{
    sgl_2dball_t *ball = (sgl_2dball_t*)obj;
    int16_t cx = 0, cy = 0;

    if(evt->type == SGL_EVENT_DRAW_MAIN) {
        sgl_area_t clip;
        sgl_color_t *buf = NULL, *blend = NULL;

        cx = (ball->obj.coords.x1 + ball->obj.coords.x2) / 2;
        cy = (ball->obj.coords.y1 + ball->obj.coords.y2) / 2;

        if (!sgl_surf_clip(surf, &obj->area, &clip)) {
            return;
        }

        sgl_area_t c_rect = {
            .x1 = cx - obj->radius,
            .x2 = cx + obj->radius,
            .y1 = cy - obj->radius,
            .y2 = cy + obj->radius
        };
        if (!sgl_area_selfclip(&clip, &c_rect)) {
            return;
        }

        int y2 = 0, real_r2 = 0, edge_alpha = 0;
        int r2 = sgl_pow2(obj->radius);
        int r2_edge = sgl_pow2(obj->radius + 1);
        int ds_alpha = SGL_ALPHA_MIN;

        buf = sgl_surf_get_buf(surf, clip.x1 - surf->x1, clip.y1 - surf->y1);
        for (int y = clip.y1; y <= clip.y2; y++) {
            y2 = sgl_pow2(y - cy);
            blend = buf;

            for (int x = clip.x1; x <= clip.x2; x++, blend++) {
                real_r2 = sgl_pow2(x - cx) + y2;
                ds_alpha = real_r2 * SGL_ALPHA_NUM / r2;

                if (real_r2 >= r2_edge) {
                    if(x > cx)
                        break;
                    continue;
                }
                else if (real_r2 >= r2) {
                    edge_alpha = SGL_ALPHA_MAX - sgl_sqrt_error(real_r2);
                    sgl_color_t color_mix = sgl_color_mixer(ball->bg_color, *blend, edge_alpha);
                    *blend = sgl_color_mixer(color_mix, *blend, ball->alpha);
                }
                else {
                    *blend = sgl_color_mixer(sgl_color_mixer(ball->bg_color, ball->color, ds_alpha), *blend, ball->alpha);
                }
            }
            buf += surf->w;
        }
    }
}


/**
 * @brief create a 2dball object
 * @param parent parent of the 2dball
 * @return 2dball object
 */
sgl_obj_t* sgl_2dball_create(sgl_obj_t* parent)
{
    sgl_2dball_t *ball = sgl_malloc(sizeof(sgl_2dball_t));
    if(ball == NULL) {
        SGL_LOG_ERROR("sgl_2dball_create: malloc failed");
        return NULL;
    }

    /* set object all member to zero */
    memset(ball, 0, sizeof(sgl_2dball_t));

    sgl_obj_t *obj = &ball->obj;
    sgl_obj_init(&ball->obj, parent);
    obj->construct_fn = sgl_2dball_construct_cb;
    obj->needinit = 1;

    ball->alpha = SGL_ALPHA_MAX;
    ball->color = SGL_THEME_COLOR;
    ball->bg_color = SGL_THEME_BG_COLOR;

    return obj;
}
