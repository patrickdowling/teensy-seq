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

#ifndef TRIGGER_INPUT_H_
#define TRIGGER_INPUT_H_

/**
 * Debouncing trigger input
 * Assumes HIGH is untriggered, LOW state is triggered
 */
template<int _PIN, int _TICKS> class DebouncedTrigger {
public:
  static const uint32_t MASK = ~(0xffffffff << _TICKS);

  void init() {
    pinMode( _PIN, INPUT_PULLUP );
    state_ = 0;
    triggered_ = false;
  }

  bool triggered() {
    state_ = (state_ << 1 ) | !digitalReadFast( _PIN );
    bool state = MASK == (state_ & MASK);
    bool rising_edge = !triggered_ && state;
    triggered_ = state;
    return rising_edge;
  }

private:
  uint32_t state_;
  bool triggered_;
};

#endif // TRIGGER_INPUT_H_
