#include <iostream>
#include "tbd/parameter.h" 

TBD_PARAMETER(float,Bar1,bar1,0.4)
TBD_PARAMETER(std::string,Bar2,bar2,"DefaultValue")

TBD_PARAMETERSET(ParameterSet1,Bar1,Bar2)

struct MySet : public tbd::ParameterSet<Bar1,Bar2>
{
};

template<typename T>
struct TypeToStr 
{
  std::string operator()() { return "var"; }
};

template<>
struct TypeToStr<double>
{
  std::string operator()() { return "double"; }
};

template<>
struct TypeToStr<float>
{
  std::string operator()() { return "float"; }
};

template<>
struct TypeToStr<std::string>
{
  std::string operator()() { return "string"; }
};


int main(int ac, char* av[])
{
  /// Test token split
  auto&& _tokens = tbd::splitToken("Bar1=0.3");
  std::cout << _tokens.first << " : " << _tokens.second << std::endl;
  _tokens = tbd::splitToken("Bar");
  std::cout << _tokens.first << " : " << _tokens.second << std::endl;
  ParameterSet1 _p({"bar1=0.3","bar2=NewValue"});
  std::cout << _p.bar1() << ", " << _p.bar2() << std::endl;

  _p.apply([&](const tbd::ParameterInterface& _param)
  {
    std::cout << _param.token() << std::endl;
  });

  MySet _mySet;
  _mySet.apply([&](const tbd::ParameterInterface& _param)
  {
    std::cout << _param.token() << std::endl;
  });

  auto&& _typeKeyValueList = _mySet.get<TypeToStr>();
  for (auto& _t : _typeKeyValueList) 
  {
    // Prints out type-key-value of the form "$type $key=$value" 
    std::cout << std::get<0>(_t) << " " << std::get<1>(_t) << "=" << std::get<2>(_t) << std::endl;
  }

  return 0;
}

