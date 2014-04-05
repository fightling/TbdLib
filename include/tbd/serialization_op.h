#pragma once

#include <boost/lexical_cast.hpp>

namespace tbd
{
  template<typename T>
  struct SerializationOperator
  {
  };
}

#define TBD_SERIALIZATION_OPERATOR_INPUT(type)\
template <typename charT, typename traits>\
inline  std::basic_istream<charT,traits>& operator>>\
  (std::basic_istream<charT,traits>& _is, type& _t)\
{\
  tbd::SerializationOperator<type>::in(_is,_t);\
  return _is;\
}\
namespace boost\
{\
  template<>\
  inline  type lexical_cast<type, std::string>(const std::string& _s)\
  {\
    type _p;\
    std::istringstream _is(_s);\
    tbd::SerializationOperator<type>::in(_is,_p);\
    return _p;\
  }\
}

#define TBD_SERIALIZATION_OPERATOR_OUTPUT(type)\
template <typename charT, typename traits>\
inline  std::basic_ostream<charT,traits>& \
  operator<<(std::basic_ostream<charT,traits>& _os, const type& _t)\
{\
  tbd::SerializationOperator<type>::out(_os,_t);\
  return _os;\
}\
namespace boost\
{\
  template<>\
  inline  std::string lexical_cast<std::string,type>(const type& _t)\
  {\
    type _p;\
    std::ostringstream _os;\
    tbd::SerializationOperator<type>::out(_os,_p);\
    return _os.str();\
  }\
}

#define TBD_SERIALIZATION_OPERATOR(type)\
  TBD_SERIALIZATION_OPERATOR_INPUT(type)\
  TBD_SERIALIZATION_OPERATOR_OUTPUT(type)


