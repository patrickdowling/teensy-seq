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

#ifndef UI_H_
#define UI_H_

#include "ringbuffer.h"

namespace UI {
  
  enum EventType {
    INVALID_EVENT,
    BUTTON_PRESS,
    ENCODER_DELTA
  };

  struct Event {
    UI::EventType type;
    int16_t sender;
    int32_t value;
  
    Event() : type( UI::INVALID_EVENT ), sender(-1), value(0) {}
    Event( UI::EventType _type, int16_t _sender, int32_t _value ) : type( _type ), sender( _sender ), value( _value ) {}
  };

  /**
   * To handle UI events asynchronously, they have to be queued up.
   * This uses an internal ring buffer to hold events and just discards
   * them if the buffer is full.
   * Looks similar to Mutable Instruments code since I've been reading
   * that and re-implementing things to get a better understanding.
   */
  class EventQueue   {
  public:
    uint32_t current_tick_;
  
    void init() {
      events_.init();
      current_tick_ = 0;
    }
  
    void tick() {
      ++current_tick_;
    }
  
    inline
    bool pending() const {
      return !events_.empty();
    }
    
    inline
    UI::Event nextEvent() {
      return events_.pop();
    }
  
    bool addEvent( UI::EventType _type, uint16_t _sender, int32_t _value ) {
      last_event_tick_ = current_tick_;
      return events_.push( UI::Event( _type, _sender, _value ) );
    }
  
  private:
    RingBuffer<UI::Event,16> events_;
    uint32_t last_event_tick_;
  };

};

#endif // UI_H_
