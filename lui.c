// Copyright (c) 2020, Takashi Toyoshima <toyoshim@gmail.com>. All rights
// reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//    * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//    * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "lui.h"

#include <memory.h>

#include "lui_device.h"

static char buffer[2][17];
static uint8_t cursor = 0;
static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;
static uint8_t dirty = 0;

void lui_init() {
  lui_device_init();
  for (int i = 0; i < 16; ++i)
    buffer[0][i] = buffer[1][0] = 0x20;
  buffer[0][16] = buffer[1][16] = 0;
  dirty = 1;
}

void lui_sync() {
  if (!dirty)
    return;
  dirty = 0;
  lui_device_locate(0, 0);
  for (int i = 0; buffer[0][i]; ++i)
    lui_device_putc(buffer[0][i]);
  lui_device_locate(0, 1);
  for (int i = 0; buffer[0][i]; ++i)
    lui_device_putc(buffer[1][i]);
  if (cursor & 2) {
    cursor &= 1;
    if (cursor) {
      lui_device_cursor_on();
      lui_device_locate(cursor_x, cursor_y);
    } else {
      lui_device_cursor_off();
    }
  } else if (cursor) {
    lui_device_locate(cursor_x, cursor_y);
  }
}

void lui_set(uint8_t window, const char* s) {
  char* p = 0;
  uint8_t size = 0;
  switch (window) {
    case LUI_WINDOW_TITLE:
      p = buffer[0];
      size = 14;
      break;
    case LUI_WINDOW_ICON:
      p = &buffer[0][14];
      size = 2;
      break;
    case LUI_WINDOW_MAIN:
      p = buffer[1];
      size = 16;
      break;
    default:
      return;
  }
  int i = 0;
  for (; *s && i < size; ++i)
    p[i] = *s++;
  for (; i < size; ++i)
    p[i] = 0x20;

  dirty = 1;
}

void lui_cursor_on(uint8_t window, uint8_t x) {
  cursor = 3;
  switch (window) {
    case LUI_WINDOW_TITLE:
      cursor_x = x;
      cursor_y = 0;
      break;
    case LUI_WINDOW_ICON:
      cursor_x = 14 + x;
      cursor_y = 0;
      break;
    case LUI_WINDOW_MAIN:
      cursor_x = x;
      cursor_y = 1;
      break;
    default:
      return;
  }

  dirty = 1;
}

void lui_cursor_off() {
  cursor = 2;
  dirty = 1;
}

int item_select = 0;
int item_name_offset = 0;
char item_buffer[48];
int8_t item_scroll_offset = 0;

static const char*
menu_set_item(int buffer_start, const char* item, uint8_t size) {
  const int buffer_end = buffer_start + size;
  int i = buffer_start;
  while (i < buffer_end) {
    item_buffer[i++] = *item++;
    if (*item == 0)
      break;
  }
  while (i < buffer_end)
    item_buffer[i++] = ' ';
  while (*item)
    item++;
  return &item[1];
}

static void menu_set_items(LUI_MENU* menu) {
  const char* item =
      (menu->mode != LUI_MENU_DYNAMIC_FULL) ?
          &menu->items[item_name_offset] : NULL;
  uint8_t size = (menu->mode == LUI_MENU_STATIC_HALF) ? 7 : 15;
  // Draw the prev item.
  if (item_select > 0) {
    item_buffer[16 - size - 1] = 0xa5;  // Dot char.
    if (menu->mode != LUI_MENU_DYNAMIC_FULL) {
      int offset = item_name_offset - 2;
      while (menu->items[offset] != 0 && offset >= 0)
      offset--;
      if (offset)
      offset++;
      item = &menu->items[offset];
    } else {
      item = menu->item(item_select - 1);
    }
    item = menu_set_item(16 - size, item, size);
  }
  // Draw selected item.
  item_buffer[16] = '>';
  if (menu->mode == LUI_MENU_DYNAMIC_FULL)
    item = menu->item(item_select);
  item = menu_set_item(17, item, size);
  if (menu->mode == LUI_MENU_DYNAMIC_FULL)
    item = menu->item(item_select + 1);
  // Draw the next item.
  if ((menu->mode == LUI_MENU_DYNAMIC_FULL) ?
      !!item : (item_select + 1) < menu->n) {
    item_buffer[16 + size + 1] = 0xa5;
    item = menu_set_item(17 + size + 1, item, size);
    if (menu->mode == LUI_MENU_DYNAMIC_FULL)
      menu->n = item_select + 2;
  } else {
    item_buffer[16 + size + 1] = ' ';
    item = menu_set_item(17 + size + 1, "", size);
    if (menu->mode == LUI_MENU_DYNAMIC_FULL)
      menu->n = item_select + 1;
  }
  // Draw the next to the next item.
  if ((size == 7) && (item_select + 2) < menu->n) {
    item_buffer[16 + (size + 1) * 2] = 0xa5;
    item = menu_set_item(17 + (size + 1) * 2, item, size);
  } else {
    item_buffer[16 + (size + 1) * 2] = ' ';
    item = menu_set_item(17 + (size + 1) * 2, "", size);
  }
}

void lui_menu_set(LUI_MENU* menu) {
  lui_set(LUI_WINDOW_TITLE, menu->title);
  for (int i = 0; i < 48; ++i)
    item_buffer[i] = ' ';
  item_select = 0;
  item_name_offset = 0;
  item_scroll_offset = 0;
  menu_set_items(menu);
}

int16_t lui_menu_run(LUI_MENU* menu, uint8_t buttons) {
  int16_t result = -1;
  char icon[2];
  icon[0] = item_select == 0 ? ' ' : 0x7f;
  icon[1] =
      ((item_select + (menu->mode == LUI_MENU_STATIC_HALF ? 2 : 1)) >= menu->n) ?
          ' ' : 0x7e;
  if (!item_scroll_offset) {
    if ((buttons & LUI_BUTTON_RIGHT) && (item_select + 1) < menu->n) {
      item_scroll_offset = 1;
      item_buffer[16] = 0xa5;
    } else if ((buttons & LUI_BUTTON_LEFT) && item_select) {
      item_scroll_offset = -1;
      item_buffer[16] = 0xa5;
    } else if (buttons & LUI_BUTTON_OK) {
      result = item_select;
    }
  } else {
    int scroll_range = (menu->mode == LUI_MENU_STATIC_HALF) ? 8 : 16;
    if (item_scroll_offset > 0) {
      item_scroll_offset++;
      if (item_scroll_offset == scroll_range) {
        item_select++;
        while (menu->items[item_name_offset] != 0)
          item_name_offset++;
        item_name_offset++;
        item_scroll_offset = 0;
        menu_set_items(menu);
      }
    } else {
      item_scroll_offset--;
      if (item_scroll_offset == -scroll_range) {
        item_select--;
        item_name_offset -= 2;
        while (menu->items[item_name_offset] != 0 && item_name_offset >= 0)
          item_name_offset--;
        if (item_name_offset)
          item_name_offset++;
        item_scroll_offset = 0;
        menu_set_items(menu);
      }
    }
  }
  lui_set(LUI_WINDOW_ICON, icon);
  lui_set(LUI_WINDOW_MAIN, &item_buffer[16 + item_scroll_offset]);
  return result;
}

uint8_t edit_x;
int8_t edit_c;
int8_t edit_need_init = -1;

void lui_edit_set(LUI_EDIT* edit) {
  edit_x = 0;
  edit_c = -1;
  edit_need_init = 0;
  lui_set(LUI_WINDOW_TITLE, edit->title);
  char icon[2] = { 0x7f, 0x7e };
  lui_set(LUI_WINDOW_ICON, icon);
  for (int i = 0; i < 16; ++i) {
    item_buffer[i] = edit->data[i];
    if (!edit->data[i])
      break;
  }
  lui_set(LUI_WINDOW_MAIN, edit->data);
  lui_cursor_on(LUI_WINDOW_MAIN, edit_x);
}

int8_t lui_edit_run(LUI_EDIT* edit, uint8_t buttons) {
  int8_t result = edit_need_init;
  edit_need_init = -1;
  if (item_buffer[edit_x] == ' ') {
    if (buttons & LUI_BUTTON_LEFT) {
      edit_c--;
      if (edit_c < 0)
        edit_c = strlen(edit->chars) - 1;
      edit->data[edit_x] = edit->chars[edit_c];
      lui_set(LUI_WINDOW_MAIN, edit->data);
      result = 0;
    } else if (buttons & LUI_BUTTON_RIGHT) {
      edit_c++;
      if (!edit->chars[edit_c])
        edit_c = 0;
      edit->data[edit_x] = edit->chars[edit_c];
      lui_set(LUI_WINDOW_MAIN, edit->data);
      result = 0;
    } else if (buttons & LUI_BUTTON_OK) {
      if (edit_c == -1) {
        lui_cursor_off();
        result = 1;
      } else {
        edit_c = -1;
        do {
          edit_x++;
        } while (item_buffer[edit_x] != ' ' && edit->data[edit_x]);
        result = edit->data[edit_x] ? 0 : 1;
        if (!result)
          lui_cursor_on(LUI_WINDOW_MAIN, edit_x);
        else
          lui_cursor_off();
      }
    }
  }
  return result;
}
