#pragma once

#include <string>

namespace tbd
{
  template<typename CHAR>
  bool isQuote(const CHAR& _q)
  {
    return _q == '"' || _q == '\'';
  }

  static inline std::string trim(const std::string& str,
                                 const std::string& _whitespace = " \t\n\r")
  {
    const auto _begin = str.find_first_not_of(_whitespace);
    if (_begin == std::string::npos)
      return ""; // no content

    const auto _end = str.find_last_not_of(_whitespace);
    return str.substr(_begin, _end - _begin + 1);
  }
 
  typedef std::string ParameterToken;

  /// Split a token of the form "a"="b" into two tokens "a" and "b"
  static inline std::pair<ParameterToken,ParameterToken> splitToken(const ParameterToken& _token, char _separator = '=')
  {
    auto _pos = std::string(_token).find(_separator);
    std::pair<std::string,std::string> _result;
    if (_pos == std::string::npos) return _result;

    std::string _str(_token);
    _result.first = trim(_str.substr(0,_pos));
    _result.second = trim(_str.substr(_pos+1,_str.length() - _pos));
    return _result;
  }

  template<typename S>
  S removeQuotes(const S& _s)
  {
    return trim(_s,"\"\'");
  }

  template<typename ISTREAM, typename TOKENLIST>
  void parse(
    ISTREAM& _is,
    TOKENLIST& _tokens,
    std::string _left = "[(",
    std::string _right = ")]",
    std::string _sep = ",",
    int _parseLevel = 0)
  {
    std::string _token;
    char c = '\0';
    bool _quote = false;
    int _level = 0;
    while (_is.good())
    {
      _is.get(c);

      if (isQuote(c)) _quote = !_quote;
      if (!_quote)
      {
        if (_sep.find(c) != std::string::npos && _level == _parseLevel)
        {
          _token = trim(_token);
          if (!_token.empty())
            _tokens.push_back(_token);
          _token.clear();
          continue;
        }

        if (_left.find(c) != std::string::npos)
        {
          c = _level < _parseLevel ? ' ' : c;
          ++_level;
        }
        if (_right.find(c) != std::string::npos)
        {
          c = _level < _parseLevel ? ' ' : c;
          --_level;
        }


      }
      if (!_is.good()) break;
     
      if (_level >= _parseLevel)
      _token += c;
    }

    _token = trim(_token);
    if (!_token.empty())
      _tokens.push_back(_token);
  }
}
