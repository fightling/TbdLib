#pragma once

namespace tbd
{
  template<typename T, bool IS_CONST = false>
  struct AddConstIf
  {
    typedef T type;
    typedef T& ref_type;
    typedef T* pointer_type;
  };
  
  template<typename T>
  struct AddConstIf<T,true>
  {
    typedef const T type;
    typedef T const& ref_type;
    typedef T const* pointer_type;
  };
}

