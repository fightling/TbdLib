#include <iostream>

#include <tbd/tuple_to_strings.h>

int main()
{
  auto&& _tuple = std::make_tuple(10,"test",3.1,true,3);

  auto&& _strings = tbd::tuple_to_strings(_tuple);
  
  std::cout << "Tuple to strings...." << std::endl;
  for (auto& _s : _strings)
  {
    std::cout << _s << std::endl;
  }

  std::cout << "ARGS to strings...." << std::endl;

  _strings = tbd::args_to_strings(40,2,"Test",true);
  for (auto& _s : _strings)
  {
    std::cout << _s << std::endl;
  }

  return EXIT_SUCCESS;
}
