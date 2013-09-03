#pragma once

#include <boost/lexical_cast.hpp>
#include <string>
#include <vector>
#include <tuple>

namespace tbd
{
  namespace detail
  {
    template<int I, class Tuple, typename F> struct for_each_impl {
        static void for_each(const Tuple& t, F f) {
            for_each_impl<I - 1, Tuple, F>::for_each(t, f);
            f(std::get<I>(t));
        }
    };
    template<class Tuple, typename F> struct for_each_impl<0, Tuple, F> {
        static void for_each(const Tuple& t, F f) {
            f(std::get<0>(t));
        }
    };
    template<class Tuple, typename F>
    F for_each(const Tuple& t, F f) {
        for_each_impl<std::tuple_size<Tuple>::value - 1, Tuple, F>::for_each(t, f);
        return f;
    }

    struct TupleElementToString
    {
      TupleElementToString(std::vector<std::string>& _strings) :
        strings_(_strings) {}
      
      std::vector<std::string>& strings_;

      template<typename T> 
      void operator()(const T& _t)
      {
        strings_.push_back(boost::lexical_cast<std::string>(_t));
      }
    };
  }
 
  template<typename TUPLE>
  std::vector<std::string> tuple_to_strings(const TUPLE& _tuple)
  {
    typedef std::vector<std::string> out_type;
    out_type _strings;
    _strings.reserve(std::tuple_size<TUPLE>());
    detail::TupleElementToString _f(_strings); 
    detail::for_each(_tuple,_f);
    return _strings;
  }

  template<typename...ARGS>
  std::vector<std::string> args_to_strings(ARGS&&..._args)
  {
    return tuple_to_strings(std::make_tuple(_args...));
  }
}
