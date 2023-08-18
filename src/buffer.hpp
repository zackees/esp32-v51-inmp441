#ifndef _BUFFER_H_
#define _BUFFER_H_

#include "alloc.h" // for StandardAllocator
#include <Arduino.h>
#include "defs.h"

template <typename T, typename Allocator = StandardAllocator>
class Buffer
{
public:
  Buffer() = default;
  Buffer(const Buffer &) = delete;

  void init(size_t n)
  {
    // Allocate memory for the array.
    ASSERT(!data_, "Buffer already initialized");
    void *ptr = Allocator::malloc(sizeof(T) * n);
    memset(ptr, 0, sizeof(T) * n);
    data_ = static_cast<T *>(ptr);
    capacity_ = n;
    curr_size_ = 0;
  }

  size_t size() const { return curr_size_; }

  T &operator[](size_t index)
  {
    ASSERT(!empty(), "FixedArray is empty.");
    return data_[index];
  }

  const T &operator[](size_t index) const
  {
    ASSERT(!empty(), "FixedArray is empty.");
    return data_[index];
  }

  bool empty() const { return curr_size_ == 0; }
  size_t remaining() const { return capacity_ - curr_size_; }
  void clear() { curr_size_ = 0; }
  void pop_front()
  {
    ASSERT(!empty(), "FixedArray is empty.");
    curr_size_--;
    for (size_t i = 0; i < curr_size_; i++)
    {
      data_[i] = data_[i + 1];
    }
  }
  void push_back(const T &value)
  {
    ASSERT(!full(), "Buffer overflow");
    data_[curr_size_++] = value;
  }

  void write(const T *data, size_t count)
  {
    for (size_t i = 0; i < count; i++)
    {
      push_back(data[i]);
    }
  }

  // Return a float indicating the percentage of the buffer that is full.
  float fullness() const { return (float)curr_size_ / (float)capacity_; }

  ~Buffer()
  {
    Allocator::free(data_);
  }

  T *begin() { return data_; }
  T *end() { return data_ + curr_size_; }
  bool full() const { return curr_size_ == capacity_; }
  T *data() { return data_; }
  const T *data() const { return data_; }

private:
  T *data_ = nullptr;
  size_t curr_size_ = 0;
  size_t capacity_ = 0;
};

#endif // _BUFFER_H_
