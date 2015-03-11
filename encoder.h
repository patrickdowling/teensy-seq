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

#ifndef ENCODER_H_
#define ENCODER_H_

template <int _PINA, int _PINB>
class Encoder {
public:

  void init() {
    pins_ = 0;
    delta_ = 0;
    pinMode( _PINA, INPUT_PULLUP );
    pinMode( _PINB, INPUT_PULLUP );
  }

  void tick() {
    delta_ = 0;
    uint8_t pins = 0;
    if ( !digitalReadFast( _PINA ) )
      pins |= 0x1;
    if ( !digitalReadFast( _PINB ) )
      pins |= 0x2;

    // Simple version: Check for falling edge of A and determine direction using B state
    if ( (pins & 0x1) && !(pins_ & 0x1) )
      delta_ = pins & 0x2 ? -1 : 1;

    pins_ = pins;
  }

  int32_t delta() const {
    return delta_;
  }

private:

  uint8_t pins_;
  int32_t delta_;
};

#endif // ENCODER_H_

