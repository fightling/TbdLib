#pragma once

#include <vector>

namespace tbd
{
  namespace seq_op
  {
    template<typename A, typename B, typename C>
    struct Add
    {
      template<typename RESERVER>
      void operator()(const A& _a, const B& _b, C& _c, RESERVER r)
      {
        _c = _a;
        r(_c,_c.size() + _b.size());
        for (auto& _e : _b)
          _c.push_back(_e);
      }
    };
    
    template<typename A, typename B, typename C>
    void add(const A& _a, const B& _b, C& _c)
    {
      Add<A,B,C>()(_a,_b,_c,[&](C& c, size_t _size) {});
    }

    template<typename A, typename B, typename T>
    void add(const A& _a, const B& _b, std::vector<T>& _c)
    {
      typedef std::vector<T> V;
      Add<A,B,V>()(_a,_b,_c,[&](V& _v, size_t _size) { _v.reserve(_size); });
    }

    template<typename A, typename B> 
    B add(const A& _a, const B& _b) 
    {
      B _c;
      add(_a,_b,_c);
      return _c;
    }

    template<typename A, typename B> 
    B operator+(const A& _a, const B& _b)
    {
      return add(_a,_b);
    }

    template<typename A, typename B> 
    B operator+=(A& _a, const B& _b)
    {
      _a = _a + _b;
      return _a;
    }
  }
}
