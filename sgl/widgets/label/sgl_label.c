#include <sgl_core.h>
#include <sgl_draw.h>
#include <sgl_math.h>
#include <sgl_log.h>
#include <sgl_mm.h>
#include <sgl_theme.h>
#include <sgl_cfgfix.h>
#include <string.h>
#include "sgl_label.h"


/**
 * @brief 旋转文本绘制函数
 * @param surf 绘制表面
 * @param area 绘制区域
 * @param x x坐标
 * @param y y坐标
 * @param str 要绘制的字符串
 * @param color 颜色
 * @param alpha 透明度
 * @param font 字体
 * @param rotation 旋转角度（0-359度）
 */
static void sgl_draw_rotated_string(sgl_surf_t *surf, sgl_area_t *area, int16_t x, int16_t y, 
                                   const char *str, sgl_color_t color, uint8_t alpha, 
                                   const sgl_font_t *font, int16_t rotation, sgl_color_t bg_color)
{
    // 计算文本尺寸
    int16_t text_width = 0;
    int16_t text_height = font->font_height;

    uint32_t unicode = 0;
    const char *temp_str = str;
    while (*temp_str) {
        temp_str += sgl_utf8_to_unicode(temp_str, &unicode);
        uint32_t ch_index = sgl_search_unicode_ch_index(font, unicode);
        text_width += (font->table[ch_index].adv_w >> 4);
    }

    // 如果没有旋转，直接调用普通绘制函数
    if (rotation == 0) {
        sgl_draw_string(surf, area, x, y, str, color, alpha, font);
        return;
    }

    // 使用定点数计算旋转角度的正弦和余弦值
    int32_t sin_val = sgl_sin(rotation);
    int32_t cos_val = sgl_cos(rotation);
    
    // 计算旋转后的边界框 - 将文本中心作为原点
    int16_t half_w = text_width / 2;
    int16_t half_h = text_height / 2;
    
    // 旋转后的四个角点 (相对文本中心)
    // 使用定点数计算，最后除以32767来归一化
    int16_t x1r = (cos_val * (-half_w) - sin_val * (-half_h)) / 32767;
    int16_t y1r = (sin_val * (-half_w) + cos_val * (-half_h)) / 32767;
    
    int16_t x2r = (cos_val * half_w - sin_val * (-half_h)) / 32767;
    int16_t y2r = (sin_val * half_w + cos_val * (-half_h)) / 32767;
    
    int16_t x3r = (cos_val * half_w - sin_val * half_h) / 32767;
    int16_t y3r = (sin_val * half_w + cos_val * half_h) / 32767;
    
    int16_t x4r = (cos_val * (-half_w) - sin_val * half_h) / 32767;
    int16_t y4r = (sin_val * (-half_w) + cos_val * half_h) / 32767;

    // 计算旋转后的边界
    int16_t min_x = sgl_min4(x1r, x2r, x3r, x4r);
    int16_t min_y = sgl_min4(y1r, y2r, y3r, y4r);
    int16_t max_x = sgl_max4(x1r, x2r, x3r, x4r);
    int16_t max_y = sgl_max4(y1r, y2r, y3r, y4r);
    
    // 文本的中心点坐标
    int16_t center_x = x + text_width / 2;
    int16_t center_y = y + text_height / 2;
    
    // 创建临时缓冲区来绘制原始文本
    int16_t buf_width = text_width > 0 ? text_width : 1;
    int16_t buf_height = text_height > 0 ? text_height : 1;
    int16_t buf_size = buf_width * buf_height;
    
    if (buf_size <= 0 || text_width <= 0) {
        // 如果文本为空或尺寸无效，直接返回
        return;
    }
    
    sgl_color_t *temp_buf = sgl_malloc(buf_size * sizeof(sgl_color_t));
    if (!temp_buf) {
        // 如果分配失败，回退到普通绘制
        sgl_draw_string(surf, area, x, y, str, color, alpha, font);
        return;
    }
    
    // 初始化临时缓冲区为指定的背景色
    for (int i = 0; i < buf_size; i++) {
        temp_buf[i] = bg_color;
    }
    
    // 先在临时缓冲区绘制原始文本
    int16_t curr_x = 0;
    temp_str = str;
    while (*temp_str) {
        temp_str += sgl_utf8_to_unicode(temp_str, &unicode);
        uint32_t ch_index = sgl_search_unicode_ch_index(font, unicode);
        
        // 创建一个临时的surface用于绘制单个字符到缓冲区
        sgl_surf_t char_surf = {
            .x1 = 0,
            .y1 = 0,
            .x2 = buf_width - 1,
            .y2 = buf_height - 1,
            .buffer = temp_buf,
            .size = buf_size * sizeof(sgl_color_t),
            .w = buf_width,
            .h = buf_height,
            .dirty = NULL
        };
        
        sgl_area_t char_area = {
            .x1 = curr_x,
            .y1 = 0,
            .x2 = buf_width - 1,
            .y2 = buf_height - 1
        };
        
        sgl_draw_character(&char_surf, &char_area, curr_x, 0, ch_index, color, alpha, font);
        curr_x += (font->table[ch_index].adv_w >> 4);
    }
    
    // 现在将旋转后的像素复制到目标表面
    for (int py = (int)min_y; py <= max_y; py++) {
        for (int px = (int)min_x; px <= max_x; px++) {
            // 计算相对于旋转中心的坐标
            int rel_x = px;
            int rel_y = py;
            
            // 逆向旋转，得到原始坐标（相对于临时缓冲区的坐标）
            // 使用定点数运算: (rel_x, rel_y) 是相对于文本中心的坐标
            int32_t orig_x_fixed = cos_val * rel_x + sin_val * rel_y;
            int32_t orig_y_fixed = -sin_val * rel_x + cos_val * rel_y;
            
            // 将定点数结果转换为整数，并恢复到缓冲区坐标系
            int orig_x = (orig_x_fixed / 32767) + half_w;
            int orig_y = (orig_y_fixed / 32767) + half_h;
            
            // 检查边界
            if (orig_x >= 0 && orig_x < buf_width && orig_y >= 0 && orig_y < buf_height) {
                int src_idx = orig_y * buf_width + orig_x;
                
                // 目标坐标
                int dst_x = center_x + px;
                int dst_y = center_y + py;
                
                // 检查目标区域是否在绘制区域内
                if (dst_x >= area->x1 && dst_x <= area->x2 && 
                    dst_y >= area->y1 && dst_y <= area->y2 &&
                    dst_x >= surf->x1 && dst_x <= surf->x2 && 
                    dst_y >= surf->y1 && dst_y <= surf->y2) {
                    
                    // 获取源像素
                    sgl_color_t src_color = temp_buf[src_idx];
                    
                    int dst_idx = (dst_y - surf->y1) * surf->w + (dst_x - surf->x1);
                    surf->buffer[dst_idx] = sgl_color_mixer(src_color, surf->buffer[dst_idx], alpha);
                }
            }
        }
    }
    
    // 释放临时缓冲区
    sgl_free(temp_buf);
}


/**
 * @brief construct the label object
 * @param surf pointer to the surface
 * @param obj pointer to the label object
 * @param evt pointer to the event
 * @return none
 */
static void sgl_label_construct_cb(sgl_surf_t *surf, sgl_obj_t* obj, sgl_event_t *evt)
{
    sgl_label_t *label = (sgl_label_t*)obj;
    sgl_pos_t align_pos;

    SGL_ASSERT(label->font != NULL);

    if (evt->type == SGL_EVENT_DRAW_MAIN) {
        if (label->bg_flag) {
            sgl_draw_fill_rect(surf, &obj->area, &obj->coords, obj->radius, label->bg_color, label->alpha);
        }

        align_pos = sgl_get_text_pos(&obj->coords, label->font, label->text, 0, (sgl_align_type_t)label->align);

        // 如果设置了文本旋转，则使用旋转绘制函数
        if (label->rota == 0) {
            sgl_draw_string(surf, &obj->area, align_pos.x + label->transform.offset.offset_x, 
                                              align_pos.y + label->transform.offset.offset_y, 
                                              label->text, label->color, label->alpha, label->font);
        }
        else {
            sgl_draw_rotated_string(surf, &obj->area, 
                                  align_pos.x, 
                                  align_pos.y, 
                                  label->text, label->color, label->alpha, 
                                  label->font, label->transform.rotation, 
                                  label->bg_flag ? label->bg_color : surf->buffer[0]);
        }
    }
}


/**
 * @brief create a label object
 * @param parent parent of the label
 * @return pointer to the label object
 */
sgl_obj_t* sgl_label_create(sgl_obj_t* parent)
{
    sgl_label_t *label = sgl_malloc(sizeof(sgl_label_t));
    if(label == NULL) {
        SGL_LOG_ERROR("sgl_label_create: malloc failed");
        return NULL;
    }

    /* set object all member to zero */
    memset(label, 0, sizeof(sgl_label_t));

    sgl_obj_t *obj = &label->obj;
    sgl_obj_init(&label->obj, parent);
    obj->construct_fn = sgl_label_construct_cb;

    label->alpha = SGL_ALPHA_MAX;
    label->bg_flag = 0;
    label->color = SGL_THEME_TEXT_COLOR;
    label->text = "";
    label->transform.rotation = 0;
    label->rota = 0;

    return obj;
}
