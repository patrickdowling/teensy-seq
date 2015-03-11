// Copyright (c) 2015 Patrick Dowling
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>

#include "encoder.h"
#include "sequencer.h"
#include "trigger_input.h"
#include "ui.h"

#include <stdint.h>

#define OLED_SCLK 13
#define OLED_MOSI 11
#define OLED_CS 10
#define OLED_RST 9
#define OLED_DC 8

#define BUTTON1_PIN 15
#define BUTTON2_PIN 16
#define BUTTON3_PIN 17
#define BUTTON4_PIN 18
#define ENCODER_PINA 19
#define ENCODER_PINB 20

#define TRIGGER_PIN 6

#define SEQ_INTERRUPT_US 20 /* 20us = 50KHz */
#define UI_INTERRUPT_US 1000 /* 1KHz */


#define	BLACK           0x0000
#define	BLUE            0x001F
#define	RED             0xF800
#define	GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0  
#define WHITE           0xFFFF

#define RGB565(r,g,b) (uint16_t)( (((uint16_t)r >> 3 ) << 11) | (((uint16_t)g>>2) << 5) | ((uint16_t)b>>3) )
#define LT_GREY RGB565(16,16,16)
#define DK_GREY RGB565(8,8,8)
#define GREY RGB565(120,120,120)

enum {
  ID_BUTTON_1 = 1,
  ID_BUTTON_2,
  ID_BUTTON_3,
  ID_BUTTON_4,
  ID_ENCODER
};

#define UI_BUTTON_DEBOUNCE_TICKS 4
#define TRIGGER_DEBOUNCE_TICKS 2

class seq_sequencerState {
public:

  void init() {
    current_tick_ = 0;
    pattern_.init( 6, 1, 1, 0, 0 );
    trigger_.init();
    
    for ( int i = 0; i < 7; ++i )
      pattern_.notes_.values_[i] = i;

    pattern_.accent_.values_[0] = 4;
    pattern_.accent_.values_[4] = 3;
    pattern_.accent_.values_[6] = 1;

    pattern_.slide_.values_[2] = 2;
    pattern_.slide_.values_[7] = 4;
  }
  
  void tick() {
    ++current_tick_;
    bool triggered = trigger_.triggered();
    if ( triggered )
      step();
  }
  
  void step() {
    ++current_step_;
    pattern_.step();
  }

  uint32_t current_tick_;
  uint32_t current_step_;
  sequencer_pattern pattern_;

private:
  
    DebouncedTrigger<TRIGGER_PIN,TRIGGER_DEBOUNCE_TICKS> trigger_;
};

static seq_sequencerState state;
static UI::EventQueue ui;

// Sequencer timebase
void FASTRUN seq_handleTick() {
  state.tick();
}

DebouncedTrigger<BUTTON1_PIN,UI_BUTTON_DEBOUNCE_TICKS> button1;
DebouncedTrigger<BUTTON2_PIN,UI_BUTTON_DEBOUNCE_TICKS> button2;
DebouncedTrigger<BUTTON3_PIN,UI_BUTTON_DEBOUNCE_TICKS> button3;
DebouncedTrigger<BUTTON4_PIN,UI_BUTTON_DEBOUNCE_TICKS> button4;
Encoder<ENCODER_PINA,ENCODER_PINB> encoder;

// UI interrupt (1Khz)
void FASTRUN ui_handleTick() {
  ui.tick();
  encoder.tick();

  if ( button1.triggered() )
    ui.addEvent( UI::BUTTON_PRESS, ID_BUTTON_1, 0 );
  if ( button2.triggered() )
    ui.addEvent( UI::BUTTON_PRESS, ID_BUTTON_2, 0 );
  if ( button3.triggered() )
    ui.addEvent( UI::BUTTON_PRESS, ID_BUTTON_3, 0 );
  if ( button4.triggered() )
    ui.addEvent( UI::BUTTON_PRESS, ID_BUTTON_4, 0 );

  int32_t delta = encoder.delta();
  if ( delta ) {
    ui.addEvent( UI::ENCODER_DELTA, ID_ENCODER, delta );
    Serial.print( delta );
  }
}

Adafruit_SSD1351 display = Adafruit_SSD1351(OLED_CS, OLED_DC, OLED_RST);
IntervalTimer seq_timerInterrupt;
IntervalTimer ui_timerInterrupt;

const char *NOTE_NAMES[] = {
  "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

uint16_t PROBABILITY_COLORS[] = { BLACK, WHITE, GREEN, YELLOW, RED };
int16_t PROBABILITY_RECT_H[] = { 0, 8, 6, 4, 2 };

#define COL_W (128/8)
#define ROW_START_Y 10
#define ROW_H 16

inline
void render_event( /*const struct sequencer_pattern &_pattern,*/ int16_t _x, int16_t _y, uint16_t _bg, uint16_t _fg, int _w, int _h ) {
  display.fillRect( _x, _y, COL_W-1, ROW_H, _bg );

  int16_t x = _x;
  int16_t y = _y + ROW_H - 2;

  display.drawFastHLine( x, y, 1, _fg );
  x += 1;
  y -= _h;
  display.drawFastVLine( x, y, _h, _fg );
  display.drawFastHLine( x, y, _w, _fg );
  
  x += _w;
  display.drawFastVLine( x, y, _h, _fg );
  y += _h;
  display.drawFastHLine( x, y, COL_W - _w - 1, _fg );
}


// render full display
void redraw( const struct sequencer_pattern &_pattern, int _cursor_pos, bool _edit ) {

  int16_t x = COL_W-1;
  for ( int i = 0; i < 8; ++i ) {
    display.drawLine( x, 0, x, 95, LT_GREY );
    x += COL_W;
  }  

  x = 0;
  for ( int i = 0; i < 8; ++i ) {

    uint16_t bg, fg;
    fg = i == _cursor_pos ? YELLOW : WHITE;
    int16_t y = ROW_START_Y;

    bg = _pattern.notes_.current_step_ == i ? DK_GREY : BLACK; 
    display.fillRect( x, y, COL_W-1, ROW_H, bg );
    display.setCursor( x, y + 2 );
    if ( _edit && i == _cursor_pos )
      display.setTextColor( RED );
    else
      display.setTextColor( fg );
    display.print( NOTE_NAMES[ _pattern.notes_.values_[ i ] ] );
  
    y += ROW_H;
    bg = _pattern.gate_.current_step_ == i ? DK_GREY : BLACK;
    render_event( x, y, bg, i == _cursor_pos ? YELLOW : WHITE, 14, 12 );

    y += ROW_H;
    bg = _pattern.octave_.current_step_ == i ? DK_GREY : BLACK;
    display.setCursor( x + 4, y + 3 );
    display.setTextColor( fg );
    display.fillRect( x, y, COL_W-1, ROW_H, bg );
    display.print( _pattern.octave_.values_[ i ] );

    y += ROW_H;
    bg = _pattern.accent_.current_step_ == i ? DK_GREY : BLACK;
    render_event( x, y, bg, i == _cursor_pos ? YELLOW : BLUE, 4, _pattern.accent_.values_[ i ] * 3 );

    y += ROW_H;
    bg = _pattern.slide_.current_step_ == i ? DK_GREY : BLACK;
    render_event( x, y, bg, i == _cursor_pos ? YELLOW : GREEN, 4, _pattern.slide_.values_[ i ] * 3 );

    x += COL_W;
  }
}

void setup() {
//  Serial.begin(9600);
//  Serial.print("hello!");

  display.begin();
  display.setCursor(0,0);
  display.fillScreen( BLACK );

  button1.init();
  button2.init();
  button3.init();
  button4.init();
  encoder.init();

  state.init();
  ui.init();

  seq_timerInterrupt.begin( seq_handleTick, SEQ_INTERRUPT_US );
  ui_timerInterrupt.begin( ui_handleTick, UI_INTERRUPT_US );

  display.setTextSize(1);
  display.drawRect( 0, 96, 128, 32, RED );
  redraw( state.pattern_, 0, false );
}

static int cursor_pos = 0;
static uint32_t last_step = -1;
static bool edit_mode = false;

void loop() {

  bool render = false;
  while ( ui.pending() ) {
    const UI::Event event = ui.nextEvent();
    switch( event.sender ) {
      case ID_BUTTON_1:
        edit_mode = !edit_mode;
        render = true;
        break;
      case ID_BUTTON_2:
        state.pattern_.octave_.edit( cursor_pos, 1 );
        render = true;
        break;
      case ID_BUTTON_3:
        state.pattern_.accent_.edit( cursor_pos, 1 );
        render = true;
        break;
      case ID_BUTTON_4:
        state.pattern_.slide_.edit( cursor_pos, 1 );
        render = true;
        break;
      case ID_ENCODER:
        if ( !edit_mode ) {
          cursor_pos += event.value;
          if ( cursor_pos < 0 ) cursor_pos = 0;
          else if ( cursor_pos > 7 ) cursor_pos = 7;
        } else {
          state.pattern_.notes_.edit( cursor_pos, event.value );
        }
        render = true;
        break;
      default:
        break;
    }
  }

  uint32_t current_step = state.current_step_;
  if ( current_step != last_step || render ) {
    redraw( state.pattern_, cursor_pos, edit_mode );
    last_step = current_step;
  }
}

