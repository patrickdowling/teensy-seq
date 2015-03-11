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

#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

/**
 * implement a basic ring buffer class
 * - Assumes _SIZE is power-of-two
 * - Holds _SIZE - 1 elements to simplify full/empty checks
 * - Adding of elements fails if full
 * TODO Pass-by-ref?
 */
template < typename _T, size_t _SIZE >
class RingBuffer {
public:
  static const size_t MASK = _SIZE - 1;

  RingBuffer()
  : read_pos_( 0 )
  , write_pos_( 0 )
  {
  }

  inline void init()
  {
    clear();
  }

  inline size_t size() const
  {
    return _SIZE;
  }

  inline void clear()
  {
    read_pos_ = write_pos_ = 0;
  }

  inline bool full() const
  {
    return ((write_pos_ + 1 ) & MASK) == read_pos_;
  }

  inline bool empty() const
  {
    return read_pos_ == write_pos_;
  }

  inline bool push( _T _item )
  {
    if ( full() )
      return false;
    size_t write_pos = write_pos_;
    buffer_[ write_pos ] = _item;
    write_pos_ = (write_pos + 1) & MASK;
    return true;
  }

  inline _T pop()
  {
    size_t read_pos = read_pos_;
    _T item = buffer_[ read_pos ];
    read_pos = (read_pos + 1) & MASK;
    read_pos_ = read_pos;
    return item;
  }

private:
  _T buffer_[ _SIZE ];
  volatile size_t read_pos_;
  volatile size_t write_pos_;
};

#endif // RINGBUFFER_H_

