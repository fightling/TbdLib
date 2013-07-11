#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "tbd/parse_utils.h"

std::vector<std::string> _files = { "../testdata/SamplerTest.tomo" };

using namespace std;

template<typename TOKENS, typename FUNCTOR>
void printTokens(const TOKENS& _tokens, const std::string& _prefix, FUNCTOR f)
{
    size_t _id = 0;
  for (auto& _token : _tokens)
  {
    std::cout << _prefix << "----------" << _id << std::endl; 
    auto&& _trimToken = tbd::trim(_token);
    std::cout << _prefix << _trimToken << std::endl;
    f(_trimToken);
      _id++;
  }
}

void parseProperties(std::istream& _is)
{
  std::vector<std::string> _tokens;
  tbd::parse(_is,_tokens,"([","])",",");
  for (auto& _token : _tokens)
    std::cout << "\t" << _token << std::endl;

}

void parseChain(std::istream& _is);

void parseElement(std::istream& _is)
{
  char c;
  std::string _token;
  while (_is.good())
  {
    _is.get(c);
    if (!_is.good()) break;
    _token += c;
  }
  auto _pos = _token.find_first_of('(');
  if (_pos != std::string::npos)
  {
    _token = _token.substr(0,_pos) + " " + _token.substr(_pos,_token.length());
  }

  std::stringstream _element(_token);

  std::vector<std::string> _tokens;
  tbd::parse(_element,_tokens,"{[(",")]}","\t\n\r ",0);
  for (auto& _token : _tokens)
    std::cout << "\t\t" << _token << std::endl;

  if (_tokens.size() < 2) return;

  std::string _argStr;

  if (!_tokens.back().empty())
  {
    if (_tokens.back()[0] == '(')
    {
      _argStr = tbd::trim(_tokens.back(),"()");
    }
  }

  if ((_tokens.size() == 2 && _argStr.empty()) || (_tokens.size() == 3))
  {
    std::cout << "\t\t\t It's an item of type " << _tokens[0] << " with id '" << _tokens[1] << "'" << std::endl;
    if (!_argStr.empty())
    {
      std::stringstream _arguments(_argStr);
      parseProperties(_arguments);
    }
  } else
  if (_tokens.size() == 2)
  {
    std::cout << "\t\t\t It's a tool of type "  << _tokens[0] << std::endl;
    std::stringstream _arguments(_argStr);
    parseProperties(_arguments);
  } else
  if (_tokens.size() == 4)
  {
    if (_tokens[2] != ":") return;
    std::cout << "\t\t\t It's a chain of type "  << _tokens[0] << std::endl;
    
    std::stringstream _chain(_tokens[1] + _tokens[2] + _tokens[3]);
    parseChain(_chain);
  }
}

void parseChain(std::istream& _is)
{
  std::vector<std::string> _tokens;
  tbd::parse(_is,_tokens,"{[(",")]}",":;",1);
  if (_tokens.size() != 2) return;

  std::stringstream _properties(_tokens[0]);
  std::stringstream _elements(_tokens[1]);

  std::cout << "Properties: " << std::endl;
  parseProperties(_properties);
  
  std::cout << "Elements: " << std::endl;
  _tokens.clear();
  tbd::parse(_elements,_tokens,"([{","}])",";",0);
  for (auto& _token : _tokens)
  {
    std::cout << "\t" << _token << std::endl;
    std::stringstream _element(_token);
    parseElement(_element);
  }
}

int main(int ac, char* av[])
{
  for (auto& _file : _files)
  {
    ifstream _ifs(_file);

    vector<string> _tokens;

    tbd::parse(_ifs,_tokens,"{[(",")]}",";");

    std::cout << _tokens.size() << std::endl;

    printTokens(_tokens,"",[&](const string& _token)
    {
      std::stringstream _ss(_token); 
      parseChain(_ss);

//      tbd::parse(_ss,_subTokens,"{(",")}",":;",1);
      //tbd::parse(_ss,_subTokens,"{","}",";",1);
  //    printTokens(_subTokens,"\t",[&](const string& _token) 
     // {
     /*   vector<string> _subTokens;
        std::stringstream _ss(_token); 
        tbd::parse(_ss,_subTokens,"(",")",",",1);
        printTokens(_subTokens,"\t\t",[&](const string& _token){});*/  
   //   });
    });
  }
  return 0;
}


