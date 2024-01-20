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

#if !defined(__lui_h__)
#define __lui_h__

#include <stdint.h>

enum {
  LUI_WINDOW_TITLE,  // 14-chars
  LUI_WINDOW_ICON,   //  2-chars
  LUI_WINDOW_MAIN,   // 16-chars
};

void lui_init();
void lui_sync();

void lui_set(uint8_t window, const char* s);

void lui_cursor_on(uint8_t window, uint8_t x);
void lui_cursor_off();

typedef struct {
  uint8_t mode;
  const char* title;
  uint8_t n;  // used only for static modes
  const char* items;  // used only for static modes
  const char* (*item)(uint8_t n);
} LUI_MENU;

enum {
  LUI_MENU_STATIC_HALF,
  LUI_MENU_STATIC_FULL,
  LUI_MENU_DYNAMIC_FULL
};

enum {
  LUI_BUTTON_LEFT = 1 << 2,
  LUI_BUTTON_RIGHT = 1 << 1,
  LUI_BUTTON_OK = 1 << 0,
};

void lui_menu_set(LUI_MENU* menu);
int16_t lui_menu_run(LUI_MENU* menu, uint8_t buttons);

typedef struct {
  const char* title;
  const char* chars;
  char* data;
} LUI_EDIT;

void lui_edit_set(LUI_EDIT* edit);
int8_t lui_edit_run(LUI_EDIT* edit, uint8_t buttons);

#endif // !defined(__lui_h__)
