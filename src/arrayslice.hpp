#ifndef _ARRAY_SLICE_H_
#define _ARRAY_SLICE_H_

#include <Arduino.h>
#include "defs.h"

template <typename T>
class ArraySlice
{
public:
  ArraySlice() = default;
  ArraySlice(T *data, size_t count) : data_(data), size_(count) {}
  ArraySlice(const ArraySlice &) = default;
  ArraySlice(ArraySlice &&) = default;
  ArraySlice &operator=(const ArraySlice &) = default;

  template <typename U>
  ArraySlice(const ArraySlice<U> &other) : data_(other.data()), size_(other.size()) {}

  // Converts fixed arrays to ArraySlice
  template <int N>
  ArraySlice(T (&data)[N]) : data_(data), size_(N) {}

  size_t size() const { return size_; }
  bool empty() const { return size_ == 0; }
  T &operator[](int index)
  {
    ASSERT(index < size_, "ArraySlice index out of bounds");
    return data_[index];
  }
  const T &operator[](int index) const
  {
    ASSERT(index < size_, "ArraySlice index out of bounds");
    return data_[index];
  }

  T *data() { return data_; }
  const T *data() const { return data_; }
  T *begin() { return data_; }
  const T *begin() const { return data_; }
  T *end() { return data_ + size_; }
  const T *end() const { return data_ + size_; }

private:
  T *data_ = nullptr;
  size_t size_ = 0;
};

#endif // _ARRAY_SLICE_H_
