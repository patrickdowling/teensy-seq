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

#ifndef SEQUENCER_H_
#define SEQUENCER_H_

template <size_t _LENGTH, int _MIN, int _MAX>
struct seq_track {
  size_t current_step_;
  size_t start_;
  size_t end_;
  int values_[_LENGTH];
  
  void init( int _value ) {
    for ( size_t i = 0; i < _LENGTH; ++i )
      values_[ i ] = _value;
    current_step_ = 0;
    start_ = 0;
    end_ = _LENGTH - 1;
  }

  void step() {
    size_t step = current_step_;
    if ( step < end_ )
      ++step;
    else
      step = start_;

    current_step_ = step;
  }

  int value() const {
    return values_[ current_step_ ];
  }
  
  void edit( int _pos, int _delta ) {
    int value = values_[ _pos ];
    value += _delta;
    if ( value < _MIN ) value = _MAX;
    else if ( value > _MAX ) value = _MIN;
    values_[ _pos ] = value;
  }
};

struct sequencer_pattern {

  void init( int _note, int _octave, int _gate, int _accent, int _slide ) {
    notes_.init( _note );
    octave_.init( _octave );
    gate_.init( _gate );
    accent_.init( _accent );
    slide_.init( _slide );
  }

  void step() {
    notes_.step();
    octave_.step();
    accent_.step();
    slide_.step();
  }

  seq_track<8,0,11> notes_;
  seq_track<8,0,2> octave_;
  seq_track<8,0,2> gate_;
  seq_track<8,0,4> accent_;
  seq_track<8,0,4> slide_;
};

#endif // SEQUENCER_H_
