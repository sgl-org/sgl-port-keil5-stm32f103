/* source/widgets/sgl_keyboard.c
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
#include <sgl_draw.h>
#include <string.h>
#include "sgl_keyboard.h"


#define  KEYBOARD_ICON_ENTER                      ("\uE8F6")
#define  KEYBOARD_ICON_NEWLINE                    ("\uE9B8")
#define  KEYBOARD_ICON_BACKSPACE                  ("\uE83E")
#define  KEYBOARD_ICON_KEYBD                      ("\uE9B0")
#define  KEYBOARD_ICON_LEFT                       ("\uE9B2")
#define  KEYBOARD_ICON_RIGHT                      ("\uE9B3")
#define  KEYBOARD_ICON_INVALID                    (-1)


#define  KEYBOARD_KEYMODE_UPPER                   (0)
#define  KEYBOARD_KEYMODE_LOWER                   (1)
#define  KEYBOARD_KEYMODE_SPEC                    (2)
#define  KEYBOARD_KEYMODE_MAX                     (3)

#define  KEYBOARD_BTN_LINES                       (4)
#define  KEYBOARD_BTN_COLUMNS                     (12)
#define  KEYBOARD_BTN_NUM                         (40)


#define  KEYBOARD_KEY_INVALID                     (-1)
#define  KEYBOARD_KEY_TO_UPPER                    (255)
#define  KEYBOARD_KEY_TO_LOWER                    (254)
#define  KEYBOARD_KEY_TO_SPEC                     (253)
#define  KEYBOARD_KEY_TO_CLOSE                    (252)

#define  KEYBOARD_KEY_MODE(x)                     ((x) >> 1)

#define  KEYBOARD_KEY_BACKSPACE                   (11)
#define  KEYBOARD_KEY_NUMBER_BACKSPACE            (10)
#define  KEYBOARD_KEY_SPACE                       (37)
#define  KEYBOARD_KEY_NEWLINE                     (22)
#define  KEYBOARD_KEY_ENTER                       (39)
#define  KEYBOARD_KEY_CLOSE                       (35)
#define  KEYBOARD_KEY_LEFT                        (36)
#define  KEYBOARD_KEY_RIGHT                       (38)


static const uint8_t font_bitmap[] = {
    /* U+E83E "" */
    0x00, 0x00, 0x03, 0x44, 0x44, 0x44, 0x44, 0x44,
    0x44, 0x30, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xfa, 0x00, 0x03, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x0d,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00, 0x8f, 0xff, 0xff, 0x5c, 0xff, 0xff, 0x4d,
    0xff, 0xff, 0x03, 0xff, 0xff, 0xfc, 0x00, 0xcf,
    0xf3, 0x08, 0xff, 0xff, 0x0d, 0xff, 0xff, 0xff,
    0xb0, 0x0b, 0x30, 0x7f, 0xff, 0xff, 0x8f, 0xff,
    0xff, 0xff, 0xfb, 0x00, 0x06, 0xff, 0xff, 0xff,
    0xcf, 0xff, 0xff, 0xff, 0xff, 0x30, 0x0c, 0xff,
    0xff, 0xff, 0x2f, 0xff, 0xff, 0xff, 0xf4, 0x05,
    0x01, 0xcf, 0xff, 0xff, 0x07, 0xff, 0xff, 0xff,
    0x40, 0x6f, 0xb0, 0x1d, 0xff, 0xff, 0x00, 0xcf,
    0xff, 0xfc, 0x06, 0xff, 0xfb, 0x08, 0xff, 0xff,
    0x00, 0x2f, 0xff, 0xff, 0xdf, 0xff, 0xff, 0xdf,
    0xff, 0xff, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0xcf, 0xff,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00,
    0x1a, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xbb, 0xa3,

    /* U+E8F6 "" */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0xf3,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x8f, 0xc0,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0xfc, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x8f, 0xc0, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x08, 0xfc, 0x00, 0x00,
    0x07, 0x70, 0x00, 0x00, 0x8f, 0xc0, 0x00, 0x00,
    0x0c, 0xf7, 0x00, 0x08, 0xfc, 0x00, 0x00, 0x00,
    0x00, 0xcf, 0x70, 0x7f, 0xc0, 0x00, 0x00, 0x00,
    0x00, 0x0c, 0xfb, 0xfc, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xcf, 0xc0, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00,

    /* U+E9B0 "" */
    0x0b, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe,
    0x70, 0x5f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xf0, 0x6f, 0xf9, 0x9e, 0x8f, 0x89, 0xe8,
    0xf8, 0xbf, 0xf1, 0x6f, 0xf1, 0x1c, 0x0e, 0x01,
    0xc0, 0xe0, 0x6f, 0xf1, 0x6f, 0xfe, 0xef, 0xef,
    0xee, 0xfe, 0xfe, 0xef, 0xf1, 0x6f, 0xf1, 0x1c,
    0x0e, 0x01, 0xc0, 0xe0, 0x6f, 0xf1, 0x6f, 0xf1,
    0x1c, 0x0e, 0x01, 0xc0, 0xe0, 0x7f, 0xf1, 0x6f,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf1,
    0x6f, 0xff, 0xfe, 0xaa, 0xaa, 0xaa, 0xff, 0xff,
    0xf1, 0x6f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0xef,
    0xff, 0xf1, 0x5f, 0xff, 0xfe, 0xaa, 0xaa, 0xaa,
    0xff, 0xff, 0xf0, 0x0b, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xfe, 0x70,

    /* U+E9B2 "" */
    0x00, 0x00, 0x02, 0x00, 0x00, 0x04, 0xf8, 0x00,
    0x04, 0xfe, 0x20, 0x04, 0xfe, 0x20, 0x04, 0xfe,
    0x20, 0x01, 0xff, 0x40, 0x00, 0x06, 0xfd, 0x10,
    0x00, 0x06, 0xfd, 0x10, 0x00, 0x06, 0xfd, 0x10,
    0x00, 0x06, 0xf9, 0x00, 0x00, 0x04, 0x00,

    /* U+E9B3 "" */
    0x00, 0x00, 0x00, 0x06, 0xd2, 0x00, 0x00, 0x5f,
    0xe2, 0x00, 0x00, 0x5f, 0xe2, 0x00, 0x00, 0x5f,
    0xe2, 0x00, 0x00, 0x5f, 0xe1, 0x00, 0x0b, 0xf9,
    0x00, 0x0b, 0xf9, 0x00, 0x0b, 0xf9, 0x00, 0x09,
    0xf9, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00,

    /* U+E9B8 "" */
    0x00, 0x00, 0x08, 0x30, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x08, 0xfa, 0x00, 0x00, 0x00, 0x01,
    0xf9, 0x00, 0x08, 0xfb, 0x00, 0x00, 0x00, 0x00,
    0x1f, 0x90, 0x08, 0xfb, 0x00, 0x00, 0x00, 0x00,
    0x01, 0xf9, 0x08, 0xff, 0xba, 0xaa, 0xaa, 0xaa,
    0xaa, 0xbf, 0x90, 0xcf, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xf9, 0x00, 0xcf, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0xcf, 0x80, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcf, 0x80,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc6,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};


static const sgl_font_table_t font_table[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 320, .box_w = 20, .box_h = 16, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 160, .adv_w = 320, .box_w = 16, .box_h = 12, .ofs_x = 2, .ofs_y = 1},
    {.bitmap_index = 256, .adv_w = 320, .box_w = 18, .box_h = 12, .ofs_x = 1, .ofs_y = 1},
    {.bitmap_index = 364, .adv_w = 320, .box_w = 7, .box_h = 11, .ofs_x = 6, .ofs_y = 2},
    {.bitmap_index = 403, .adv_w = 320, .box_w = 7, .box_h = 11, .ofs_x = 7, .ofs_y = 2},
    {.bitmap_index = 442, .adv_w = 320, .box_w = 17, .box_h = 11, .ofs_x = 1, .ofs_y = 1}
};

static const uint32_t unicode_list_0[] = {
    0x00, 0xb8, 0x172, 0x174, 0x175, 0x17a
};

static const sgl_font_unicode_t font_unicode[] =
{
    { .offset = 0xe83e, .len = 6, .list = unicode_list_0, .tab_offset = 1, }
};

static const sgl_font_t keyboard_icon = {
    .bitmap = font_bitmap,
    .table = font_table,
    .font_table_size = SGL_ARRAY_SIZE(font_table),
    .font_height = 16,
    .base_line = 1,
    .bpp = 4,
    .unicode = font_unicode,
    .unicode_num = 1,
};


static const char keyboard_key_ascii[KEYBOARD_KEYMODE_MAX][KEYBOARD_BTN_NUM] = {
    {
        KEYBOARD_KEY_TO_SPEC, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '\b',
        KEYBOARD_KEY_TO_LOWER, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', '\n',
        '_', '-', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '.', ',', ':',
        KEYBOARD_KEY_TO_CLOSE, KEYBOARD_KEY_LEFT, ' ', KEYBOARD_KEY_RIGHT, '\r'
    },
    {
        KEYBOARD_KEY_TO_SPEC, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '\b',
        KEYBOARD_KEY_TO_UPPER, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '\n',
        '_', '-', 'z', 'x', 'c', 'v', 'b', 'n', 'm', '.', ',', ':',
        KEYBOARD_KEY_TO_CLOSE, KEYBOARD_KEY_LEFT, ' ', KEYBOARD_KEY_RIGHT, '\r'
    },
    {
        '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\b',
        KEYBOARD_KEY_TO_LOWER, '+', '-', '/', '*', '=', '%', '!', '?', '#', '<', '>',
        '\\', '@', '$', '(', ')', '{', '}', '[', ']', ';', '\"', '\'',
        KEYBOARD_KEY_TO_CLOSE, KEYBOARD_KEY_LEFT, ' ', KEYBOARD_KEY_RIGHT, '\r'
    },
};


static const char keybd_btn_map[KEYBOARD_KEYMODE_MAX][KEYBOARD_BTN_NUM][4] = {
    {
        "1#", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "<<",
        "abc", "A", "S", "D", "F", "G", "H", "J", "K", "L", "nl",
        "_", "-", "Z", "X", "C", "V", "B", "N", "M", ".", ",", ":",
        "kbd", "<", " ", ">", "OK"
    },
    {
        "1#", "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "<<",
        "ABC", "a", "s", "d", "f", "g", "h", "j", "k", "l", "nl",
        "_", "-", "z", "x", "c", "v", "b", "n", "m", ".", ",", ":",
        "kbd", "<", " ", ">", "OK"
    },
    {
        "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "<<",
        "abc", "+", "-", "/", "*", "=", "%", "!", "?", "#", "<", ">",
        "\\", "@", "$", "(", ")", "{", "}", "[", "]", ";", "\"", "'",
        "kbd", "<", " ", ">", "OK"
    },
};


static const uint8_t keybd_btn_width[2][KEYBOARD_BTN_LINES][KEYBOARD_BTN_COLUMNS] = {
    /* upper and lower mode */
    {
        {5, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 7},
        {6, 3, 3, 3, 3, 3, 3, 3, 3, 3, 7},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {2, 2, 6, 2, 2},
    },
    /* number mode and spec mode */
    {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2},
        {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {2, 2, 6, 2, 2},
    },
};


static const uint8_t keybd_btn_height[KEYBOARD_BTN_LINES] = {
    1, 1, 1, 1
};


static const uint8_t keyboard_btn_count[2][KEYBOARD_BTN_LINES] = {
    /* upper and lower mode */
    {
        12, 11, 12, 5
    },
    /* number mode and spec mode */
    {
        11, 12, 12, 5
    }
};


static void keyboard_btn_handler(sgl_keyboard_t *keyboard)
{
    char *edit = keyboard->edit;
    int index = keyboard->edit_max_len;
    while (*edit && index) {
        edit ++;
        index --;
    }

    if (keyboard->opcode == '\b') {
        *(--edit) = '\0';
        return;
    }

    if (index == 0) {
        SGL_LOG_ERROR("keyboard edit buffer is full\n");
        return;
    }

    switch (keyboard->opcode) {
    case '\n':
    case '\r':
        *(edit++) = '\n';
        *(edit++) = '\0';
        break;
    default :
        *(edit++) = keyboard->opcode;
        *(edit++) = '\0';
        break;
    }
}


static const char* keyindex_is_icon(uint8_t mode, uint8_t index)
{
    if(index == KEYBOARD_KEY_NUMBER_BACKSPACE && (mode == KEYBOARD_KEYMODE_SPEC)) {
        return KEYBOARD_ICON_BACKSPACE;
    }
    else if(index == KEYBOARD_KEY_BACKSPACE && (mode < KEYBOARD_KEYMODE_SPEC)) {
        return KEYBOARD_ICON_BACKSPACE;
    }
    else if(index == KEYBOARD_KEY_ENTER) {
        return KEYBOARD_ICON_ENTER;
    }
    else if(index == KEYBOARD_KEY_NEWLINE && (mode != KEYBOARD_KEYMODE_SPEC)) {
        return KEYBOARD_ICON_NEWLINE;
    }
    else if(index == KEYBOARD_KEY_CLOSE) {
        return KEYBOARD_ICON_KEYBD;
    }
    else if(index == KEYBOARD_KEY_LEFT) {
        return KEYBOARD_ICON_LEFT;
    }
    else if(index == KEYBOARD_KEY_RIGHT) {
        return KEYBOARD_ICON_RIGHT;
    }

    return NULL;
}


static inline uint8_t keyboard_index_to_ascii(uint8_t keymode, uint8_t index)
{
    return keyboard_key_ascii[keymode][index];
}


static int8_t keyboard_pos_to_index(int16_t x, int16_t y, sgl_keyboard_t *keyboard, int16_t width, int16_t height)
{
    sgl_obj_t *obj = &keyboard->obj;
    int16_t index = 0, x_index = -1, y_index = -1, key_mode = KEYBOARD_KEY_MODE(keyboard->key_mode);
    int16_t btn_width[KEYBOARD_BTN_COLUMNS] = {0};
    int16_t btn_height[KEYBOARD_BTN_LINES] = {0};
    sgl_rect_t btn = {0};

    /* calculate button height */
    sgl_split_len(keybd_btn_height, KEYBOARD_BTN_LINES, height, keyboard->key_margin, btn_height);
    btn.y1 = obj->coords.y1;
    for(int i = 0; i < KEYBOARD_BTN_LINES; i++) {
        btn.y1 += keyboard->key_margin;
        btn.y2 = btn.y1 + btn_height[i] - 1;

        if(y >= btn.y1 && y <= btn.y2) {
            y_index = i;
            break;
        }
        btn.y1 += btn_height[i];
    }

    if(y_index < 0 || y_index >= KEYBOARD_BTN_LINES) {
        return -1;
    }

    sgl_split_len(keybd_btn_width[key_mode][y_index], keyboard_btn_count[key_mode][y_index], width, keyboard->key_margin, btn_width);

    btn.x1 = obj->coords.x1;
    for(int i = 0; i < keyboard_btn_count[key_mode][y_index]; i++) {
        btn.x1 += keyboard->key_margin;
        btn.x2 = btn.x1 + btn_width[i] - 1;
        if(x >= btn.x1 && x <= btn.x2) {
            x_index = i;
            break;
        }
        btn.x1 += btn_width[i];
    }

    if(x_index < 0 || x_index >= keyboard_btn_count[key_mode][y_index]) {
        return -1;
    }

    for(int i = 0; i < y_index; i ++) {
        index += keyboard_btn_count[key_mode][i];
    }

    return (index + x_index);
}


/**
 * @brief keyboard constructor function
 * @param surf: pointer to surface
 * @param obj: pointer to keyboard object
 * @param evt: pointer to event
 * @return none
 */
static void sgl_keyboard_construct_cb(sgl_surf_t *surf, sgl_obj_t* obj, sgl_event_t *evt)
{
    sgl_keyboard_t *keyboard = (sgl_keyboard_t*)obj;
    int16_t body_w = obj->coords.x2 - obj->coords.x1 + 1;
    int16_t body_h = obj->coords.y2 - obj->coords.y1 + 1;

    int8_t index = 0, key_mode = KEYBOARD_KEY_MODE(keyboard->key_mode);
    sgl_color_t btn_color = keyboard->btn_desc.color;
    int16_t text_x = 0, text_y = 0;
    const char *text = NULL;
    const sgl_font_t *font = keyboard->font;
    int16_t btn_width[KEYBOARD_BTN_COLUMNS] = {0};
    int16_t btn_height[KEYBOARD_BTN_LINES] = {0};
    sgl_rect_t btn = {0};

    /* calculate button height */
    sgl_split_len(keybd_btn_height, KEYBOARD_BTN_LINES, body_h, keyboard->key_margin, btn_height);

    if(evt->type == SGL_EVENT_DRAW_MAIN) {
        sgl_draw_rect(surf, &obj->area, &obj->coords, &keyboard->body_desc);

        btn.y1 = obj->coords.y1;

        for(int i = 0; i < KEYBOARD_BTN_LINES; i++) {
            sgl_split_len(keybd_btn_width[key_mode][i], keyboard_btn_count[key_mode][i], body_w, keyboard->key_margin, btn_width);

            btn.x1 = obj->coords.x1;
            btn.y1 += keyboard->key_margin;
            btn.y2 = btn.y1 + btn_height[i] - 1;

            for(int j = 0; j < keyboard_btn_count[key_mode][i]; j++) {
                btn.x1 += keyboard->key_margin;
                btn.x2 = btn.x1 + btn_width[j] - 1;

                if(index == keyboard->key_index) {
                    keyboard->btn_desc.color = sgl_color_mixer(btn_color, keyboard->text_color, 128);
                }
                else {
                    keyboard->btn_desc.color = btn_color;
                }
                sgl_draw_rect(surf, &btn, &btn, &keyboard->btn_desc);

                text = keyindex_is_icon(keyboard->key_mode, index);
                if (text != NULL) {
                    font = &keyboard_icon;
                }
                else {
                    font = keyboard->font;
                    text = keybd_btn_map[keyboard->key_mode][index];
                }
                text_x = btn.x1 + (btn_width[j] - sgl_font_get_string_width(text, font)) / 2;
                text_y = btn.y1 + (btn_height[i] - font->font_height) / 2;
                sgl_draw_string(surf, &btn, text_x, text_y, text, keyboard->text_color, SGL_ALPHA_MAX, font);

                btn.x1 += btn_width[j];
                index ++;
            }

            btn.y1 += btn_height[i];
        }

        keyboard->btn_desc.color = btn_color;
    }
    else if(evt->type == SGL_EVENT_PRESSED || evt->type == SGL_EVENT_OPTION_TAP) {
        if  (evt->type == SGL_EVENT_PRESSED) {
            index = keyboard_pos_to_index(evt->pos.x, evt->pos.y, keyboard, body_w, body_h);
            if(index < 0) {
                return;
            }
            keyboard->key_index = index;
        }

        uint8_t key_ascii = keyboard_index_to_ascii(keyboard->key_mode, index);
        if(key_ascii == KEYBOARD_KEY_TO_UPPER) {
            keyboard->key_mode = KEYBOARD_KEYMODE_UPPER;
            keyboard->key_index = KEYBOARD_KEY_INVALID;
        }
        else if(key_ascii == KEYBOARD_KEY_TO_LOWER) {
            keyboard->key_mode = KEYBOARD_KEYMODE_LOWER;
            keyboard->key_index = KEYBOARD_KEY_INVALID;
        }
        else if(key_ascii == KEYBOARD_KEY_TO_SPEC) {
            keyboard->key_mode = KEYBOARD_KEYMODE_SPEC;
            keyboard->key_index = KEYBOARD_KEY_INVALID;
        }
        else if(key_ascii == KEYBOARD_KEY_TO_CLOSE) {
            sgl_obj_set_destroyed(obj);
            return;
        }
        else {
            keyboard->opcode = key_ascii;
            if (keyboard->edit) {
                keyboard_btn_handler(keyboard);
            }
        }

        if(obj->event_fn) {
            obj->event_fn(evt);
        }
    }
    else if(evt->type == SGL_EVENT_RELEASED) {
        keyboard->key_index = KEYBOARD_KEY_INVALID;
    }
    else if(evt->type == SGL_EVENT_DRAW_INIT) {
        keyboard->opcode = 0;
        keyboard->key_index = KEYBOARD_KEY_INVALID;

        if (keyboard->key_margin == 0) {
            keyboard->key_margin = sgl_max(body_w / 128, 1);
        }

        if (keyboard->btn_desc.radius == 0) {
            keyboard->btn_desc.radius = sgl_max(keyboard->key_margin, 2);
        }

        SGL_ASSERT(keyboard->font != NULL);
    }
    else if (evt->type == SGL_EVENT_OPTION_WALK) {
        keyboard->key_index ++;

        if (keyboard->key_index >= KEYBOARD_BTN_NUM) {
            keyboard->key_index = 0;
        }
    }
}


/**
 * @brief create keyboard object
 * @param parent parent of object
 * @return keyboard object
 */
sgl_obj_t* sgl_keyboard_create(sgl_obj_t* parent)
{
    sgl_keyboard_t *keyboard = sgl_malloc(sizeof(sgl_keyboard_t));
    if(keyboard == NULL) {
        SGL_LOG_ERROR("sgl_keyboard_create: malloc failed");
        return NULL;
    }

    /* set object all member to zero */
    memset(keyboard, 0, sizeof(sgl_keyboard_t));

    sgl_obj_t *obj = &keyboard->obj;
    sgl_obj_init(&keyboard->obj, parent);
    obj->construct_fn = sgl_keyboard_construct_cb;
    sgl_obj_set_border_width(obj, SGL_THEME_BORDER_WIDTH);

    obj->clickable = 1;
    obj->needinit  = 1;

    keyboard->body_desc.alpha = SGL_THEME_ALPHA;
    keyboard->body_desc.color = SGL_THEME_COLOR;
    keyboard->body_desc.radius = SGL_THEME_RADIUS;
    keyboard->body_desc.border = 1;
    keyboard->body_desc.border_color = SGL_THEME_BORDER_COLOR;
    keyboard->body_desc.pixmap = NULL;
    keyboard->text_color = SGL_THEME_TEXT_COLOR;

    keyboard->btn_desc.alpha = SGL_THEME_ALPHA;
    keyboard->btn_desc.color = SGL_THEME_COLOR;
    keyboard->btn_desc.radius = 0;
    keyboard->btn_desc.border = 1;
    keyboard->btn_desc.border_color = SGL_THEME_BORDER_COLOR;
    keyboard->btn_desc.pixmap = NULL;

    keyboard->key_mode = KEYBOARD_KEYMODE_LOWER;

    keyboard->key_margin = 0;

    return obj;
}

