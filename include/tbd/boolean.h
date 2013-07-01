#pragma once

#include <utility>
#include <set>
#include <algorithm>

namespace tbd
{
  namespace boolean
  {
    template<typename T>
    struct Union
    {
      template<typename INSERTER>
      void operator()(const T& _a, const T& _b, T& _result, INSERTER ins)
      {
        auto first1 = _a.begin(), last1 = _a.end();
        auto first2 = _b.begin(), last2 = _b.end();
        while (true)
        {
          if (first1==last1)
          {
            std::for_each(first2,last2,ins);
            return;
          }
          if (first2==last2)
          {
            std::for_each(first1,last1,ins);
            return;
          }
          if (*first1<*first2)
          {
            ins(*first1);
            ++first1;
          }
          else if (*first2<*first1)
          {
            ins(*first2);
            ++first2;
          }
          else
          {
            ins(*first1);
            ++first1;
            ++first2;
          }
        }
      }
    };

    template<typename T>
    struct Intersection
    {
      template<typename INSERTER>
      void operator()(const T& _a, const T& _b, T& _result, INSERTER ins)
      {
        auto first1 = _a.begin(), last1 = _a.end();
        auto first2 = _b.begin(), last2 = _b.end();
        while (first1!=last1 && first2!=last2)
        {
          if (*first1<*first2) ++first1;
          else if (*first2<*first1) ++first2;
          else {
            ins(*first1);
            ++first1; ++first2;
          }
        }
      }
    };

    template<typename T>
    struct Difference
    {
      template<typename INSERTER>
      void operator()(const T& _a, const T& _b, T& _result, INSERTER ins)
      {
        auto first1 = _a.begin(), last1 = _a.end();
        auto first2 = _b.begin(), last2 = _b.end();
        while (first1!=last1 && first2!=last2)
        {
          if (*first1<*first2) { ins(*first1); ++first1; }
          else if (*first2<*first1) ++first2;
          else { ++first1; ++first2; }
        }
        std::for_each(first1,last1,ins); 
      }
    };

    namespace 
    {
      template<template<class> class BOOLEAN_OP, typename T>
      void dispatch(const T& _lhs, const T& _rhs, T& _result) 
      {
        BOOLEAN_OP<T>()(_lhs,_rhs,_result,[&](const typename T::value_type& a)
        {
          _result.push_back(a);
        });
      }

      template<template<class> class BOOLEAN_OP, typename T>
      void dispatch(const std::set<T>& _lhs, const std::set<T>& _rhs, std::set<T>& _result)
      {
        BOOLEAN_OP<std::set<T>>()(_lhs,_rhs,_result,[&](const T& a)
        {
          _result.insert(a);
        });
      }
    }

    template<typename T>
    T operator|(const T& _lhs, const T& _rhs)
    {
      T _result;
      dispatch<Union>(_lhs,_rhs,_result);
      return _result;
    }

    template<typename T>
    T operator|=(T& _lhs, const T& _rhs)
    {
      _lhs = _lhs | _rhs;
      return _lhs;
    }

    template<typename T>
    T operator&(const T& _lhs, const T& _rhs)
    {
      T _result;
      dispatch<Intersection>(_lhs,_rhs,_result);
      return _result;
    }

    template<typename T>
    T operator&=(T& _lhs, const T& _rhs)
    {
      _lhs = _lhs & _rhs;
      return _lhs;
    }

    template<typename T>
    T operator-(const T& _lhs, const T& _rhs)
    {
      T _result;
      dispatch<Difference>(_lhs,_rhs,_result);
      return _result;
    }

    template<typename T>
    T operator-=(T& _lhs, const T& _rhs)
    {
      _lhs = _lhs - _rhs;
      return _lhs;
    }
  }
}
