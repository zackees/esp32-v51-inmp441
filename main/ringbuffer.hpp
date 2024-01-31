#ifndef _RINGBUFFER_H_
#define _RINGBUFFER_H_

#include <Arduino.h>
#include "alloc.h" // for StandardAllocator
#include "defs.h"

// A class representing a fixed-size circular buffer (ring buffer).
// Type T must be a POD type. A custom allocator can be provided.
// Warning - this class is broken. But it works for pushing values one
// at a time.
template <typename T, typename Allocator = StandardAllocator>
class RingBuffer
{
public:
  /**
   * create a ringbuffer with space for up to size elements.
   */
  RingBuffer() = default;
  RingBuffer(size_t size, size_t initial_size = 0) { init(size, initial_size); }

  void init(size_t count, size_t initial_size = 0)
  {
    ASSERT(buffer == nullptr, "RingBuffer already initialized.");
    ASSERT(count > 0, "RingBuffer size must be > 0.");
    buffer = static_cast<T *>(Allocator::malloc(sizeof(T) * count));
    memset(buffer, 0, sizeof(T) * count);
    // buffer.resize(count);
    capacity = count;
    begin = 0;
    size_ = initial_size;
    end = initial_size;
  }

  /**
   * copy constructor
   */
  RingBuffer(const RingBuffer<T> &rb) = delete;

  /**
   * destructor
   */
  ~RingBuffer()
  {
    Allocator::free(buffer);
  }

  void push_back(const T &item)
  {
    end = end % capacity;
    buffer[end] = item;
    end = (end + 1) % capacity;

    if (full())
    {
      begin = (begin + 1) % capacity;
    }
    else
    {
      ++size_;
    }
    check();
  }

  void push_back(const T *data, size_t n)
  {
    for (int i = 0; i < n; ++i)
    {
      push_back(data[i]);
    }
  }

  void pop_front(const T* data, size_t n)
  {
    for (int i = 0; i < n; ++i)
    {
      pop_front();
    }
  }

  T pop_front()
  {
    ASSERT(!empty(), "RingBuffer is empty.");
    T tmp = buffer[begin];
    begin = (begin + 1) % capacity;
    // assert(begin >= 0);
    // assert(begin < capacity);
    --size_;
    check();
    return tmp;
  }

  void check() const
  {
#if 0
        assert(begin >= 0);
        assert(begin < capacity);
        assert(end <= capacity);
        assert(end >= 0);
#endif
  }

  size_t read_back(T *dst, size_t n) const
  {
    n = std::min(n, size_);
    int curr_end = end;
    for (int i = 0; i < n; ++i)
    {
      if (curr_end <= 0)
      {
        curr_end = size_;
      }
      --curr_end;
      dst[i] = buffer[curr_end];
    }
    return n;
  }

  size_t emit_front(T *dst, size_t n)
  {
    size_t count = 0;
    while (!empty() && count < n)
    {
      T tmp = pop_front();
      dst[count++] = tmp;
    }
    return count;
  }

  size_t emit_back(T *dst, size_t n)
  {
    size_t count = 0;
    while (!empty() && count < n)
    {
      T tmp = pop_back();
      dst[count++] = tmp;
    }
    return count;
  }

  const T *data() const
  {
    return buffer;
  }

  T *data()
  {
    return buffer;
  }

  T pop_back()
  {
    ASSERT(!empty(), "RingBuffer is empty.");
    volatile RingBuffer<T, Allocator> *this_vol = this;
    if (end == 0)
    {
      end = capacity;
    }
    --end;
    --size_;
    T tmp = buffer[end];
    check();
    return tmp;
  }

  bool empty() const
  {
    check();
    return size_ == 0;
  }

  // operator []
  T &operator[](int index)
  {
    // ASSERT(!empty(), "RingBuffer is empty.");
    check();
    size_t target_idx = (begin + index) % capacity;
    return buffer[target_idx];
  }

  const T at(size_t index) const
  {
    // ASSERT(!empty(), "RingBuffer is empty.");
    check();
    size_t target_idx = (begin + index) % capacity;
    return buffer[target_idx];
  }

  size_t size() const
  {
    return size_;
  }

  bool full() const
  {
    check();
    return size_ == capacity;
  }

  void clear()
  {
    begin = 0;
    end = 0;
    size_ = 0;
  }

  void print() const
  {
#if 0
    cout << "object: capacity=" << capacity << ",size=" << size_ << ",begin=" << begin << ",end=" << end << "\n";
    for (int i = 0; i < size_; ++i)
    {
      cout << at(i) << ",";
    }
    cout << "\n";
#endif
  }

private:
  T *buffer = nullptr;
  // vector<T> buffer;
  size_t capacity = 0;
  size_t size_ = 0;
  int begin = 0;
  int end = 0;
};


#endif // _RINGBUFFER_H_
