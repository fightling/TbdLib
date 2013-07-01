#include <iostream>
#include "tbd/parameter.h" 

TBD_PARAMETER(float,Bar1,bar1,0.4)
TBD_PARAMETER(std::string,Bar2,bar2,"DefaultValue")

TBD_PARAMETERSET(ParameterSet1,Bar1,Bar2)

struct MySet : public tbd::ParameterSet<Bar1,Bar2>
{
};


int main(int ac, char* av[])
{
  /// Test token split
  auto&& _tokens = tbd::detail::splitToken("Bar1=0.3");
  std::cout << _tokens.first << " : " << _tokens.second << std::endl;
  _tokens = tbd::detail::splitToken("Bar");
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

  /*

  _p.put("Bar1:0.2");

  float _bar1;
//  _p.get("Bar1",_bar1); 
  std::cout << _bar1 << std::endl;*/
  return 0;
}

