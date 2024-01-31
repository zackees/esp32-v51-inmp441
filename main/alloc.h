#ifndef _ALLOC_H_
#define _ALLOC_H_

struct StandardAllocator
{
  static void *malloc(size_t size) { return ::malloc(size); }
  static void free(void *ptr)
  {
    ::free(ptr);
  }
};



#endif // _PSRAM_H_