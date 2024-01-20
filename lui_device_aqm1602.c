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

#include "lui_device.h"

#include "lui_bus.h"

static void write_instruction(uint8_t instruction) {
  lui_bus_write(0x7c, 0x00, instruction);
  lui_bus_wait(1);
}

static void write_data(uint8_t data) {
  lui_bus_write(0x7c, 0x40, data);
}

void lui_device_init() {
  lui_bus_wait(40);
  write_instruction(0x38);  // Function Set (8-bits 2-lines)
  write_instruction(0x39);  // Function Set (Select Instruction table 1)
  write_instruction(0x14);  //   Internal OSC frequency
  write_instruction(0x74);  //   Contrast set
//  write_instruction(0x57);  //   Power/ICON control/Contrast set
  write_instruction(0x56);  //   Power/ICON control/Contrast set
  write_instruction(0x6c);  //   Follower control
  lui_bus_wait(200);
  write_instruction(0x38);  // Function Set (Select Instruction table 0)
  write_instruction(0x0c);  // Display ON/OFF
  write_instruction(0x01);  // Clear Display
}

void lui_device_locate(uint8_t x, uint8_t y) {
  write_instruction(0x80 + x + (y ? 0x40 : 0x00));
}

void lui_device_putc(uint8_t c) {
  write_data(c);
}

void lui_device_cursor_on() {
  write_instruction(0x0d);  // Display ON/OFF (w/Cursor OFF, Blink ON)
}

void lui_device_cursor_off() {
  write_instruction(0x0c);  // Display ON/OFF (w/Cursor OFF, Blink OFF)
}
