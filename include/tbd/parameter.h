/************************FreeBSD license header*****************************
 * Copyright (c) 2013, Wilston Oreo
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***********************************************************************/

#pragma once

#include "property.h"
#include <boost/lexical_cast.hpp>

namespace tbd  
{
  typedef std::string ParameterToken;
  
  namespace detail
  {
    static std::pair<ParameterToken,ParameterToken> splitToken(const ParameterToken& _token, char _separator = '=')
    {
      auto _pos = std::string(_token).find(_separator);
      std::pair<std::string,std::string> _result;
      if (_pos == std::string::npos) return _result;

      std::string _str(_token);
      _result.first = _str.substr(0,_pos);
      _result.second = _str.substr(_pos+1,_str.length() - _pos);
      return _result;
    }
  }

  struct ParameterInterface 
  {
    virtual ~ParameterInterface() {}

    virtual char const* name() const = 0; 
    virtual char const* varname() const = 0;
    virtual std::string token() const = 0;
    virtual std::string valueAsStr() const = 0;
    virtual std::string defaultAsStr() const = 0;
  };


  template<typename T>
  struct TypedParameterInterface : ParameterInterface
  {
    typedef T var_type;
    TypedParameterInterface() {}
    TypedParameterInterface(const var_type& _value) : value_(_value) {}
    
    std::string token() const
    {
      std::stringstream _ss;
      _ss << std::string(varname()) << '=' << valueAsStr();
      return _ss.str();
    }

    std::string valueAsStr() const
    {
      std::stringstream _ss;
      _ss << value();
      return _ss.str();
    }
    
    virtual var_type def() const = 0;

    std::string defaultAsStr() const
    {
      std::stringstream _ss;
      _ss << def();
      return _ss.str();
    }

    template<typename CONFIG_PATH, typename CONFIG>
    bool load(const CONFIG_PATH& _path, const CONFIG& _config)
    {
      auto _value = _config.get(_path / CONFIG_PATH(name()),def());
      if (_value == value_)
      {
        return false;
      } else
      {
        value_ = _value;
      }
      return true;
    }
    template<typename CONFIG_PATH, typename CONFIG>
    void save(const CONFIG_PATH& _path, CONFIG& _config) const
    {
      _config.put(_path / CONFIG_PATH(name()),value_);
    }
      
    void put(const ParameterToken& _token)
    {\
      auto&& _tokens = tbd::detail::splitToken(_token);
      if (_tokens.first.empty() || _tokens.second.empty()) return;
      if (_tokens.first == std::string(varname()))
      {
        value(boost::lexical_cast<var_type>(_tokens.second));
      }
    }

    void put(const std::vector<ParameterToken>& _tokens)
    {
      for (auto& _token : _tokens)
      {
        put(_token);
      }
    }

    void put(std::initializer_list<ParameterToken>&& _tokens)
    {
      for (auto& _token : _tokens)
      {
        put(_token);
      }
    }

    template<typename VALUE>\
    bool get(const ParameterToken& _token, VALUE& _value) const
    {
      if (std::string(_token) != std::string(varname())) return false;
      _value = boost::lexical_cast<VALUE>(value());
      return true;
    }
    
    template<typename FUNCTOR>
    void apply(FUNCTOR f)
    {
      f(*this); 
    }

    template<typename FUNCTOR>
    void apply(FUNCTOR f) const
    {
      f(*this); 
    }

    TBD_PROPERTY_REF(var_type,value)
  };


/**@brief Macro for defining a parameter
   @detail Should be declared in anonymous namespace 
*/
#define TBD_PARAMETER(var_type,parameter_name,var_name,var_def)\
  struct parameter_name : tbd::TypedParameterInterface<var_type>\
  {\
    typedef var_type type;\
    typedef tbd::TypedParameterInterface<var_type> inherited_type;\
    parameter_name() : inherited_type(var_def) {} \
    parameter_name(const var_type& _value) : \
      inherited_type(_value) {}\
    char const* name() const { return #parameter_name; }\
    char const* varname() const { return #var_name; }\
    var_type def() const { return var_def; }\
    var_type& var_name() { return value(); }\
    const var_type& var_name() const { return value(); }\
  };

/**@brief Macro for defining an array parameter
   @detail Should be declared in anonymous namespace 
*/
#define TBD_PARAMETER_ARRAY(var_type,parameter_name,var_name,...)\
  class parameter_name\
  {\
  public:\
    parameter_name(const var_type& _##var_name) : var_name##_(_##var_name) {}\
    parameter_name() : var_name##_({__VA_ARGS__}) {} \
    TBD_PARAMETER_REF(var_type,var_name)\
  protected:\
    typedef var_type type;\
    \
    template<typename CONFIG_PATH, typename CONFIG>\
    bool load(const CONFIG_PATH& _path, const CONFIG& _config)\
    {\
      var_type _##var_name = _config.get_array(_path / CONFIG_PATH(#var_name),{__VA_ARGS__});\
      if (_##var_name == var_name##_)\
      {\
        return false;\
      } else\
      {\
        var_name##_ = _##var_name;\
      }\
      return true;\
    }\
    template<typename CONFIG_PATH, typename CONFIG>\
    void save(const CONFIG_PATH& _path, CONFIG& _config) const\
    {\
      _config.put_array(_path / CONFIG_PATH(#var_name),var_name());\
    }\
    const var_type& value() const { return var_name(); }\
    var_type& value() { return var_name(); }\
    char const* varname() const { return #var_name; }\
    var_type def() const { return {__VA_ARGS__}; }\
    char const* name() const { return #var_name; }\
    template<typename FUNCTOR>\
    void apply(FUNCTOR f)\
    {\
      f(name(),varname(),value(),def());\
    }\
    template<typename FUNCTOR>\
    void apply(FUNCTOR f) const\
    {\
      f(name(),varname(),value(),def());\
    }\
  };

/// Macro for defining a parameter set
#define  TBD_PARAMETERSET(name,...)\
  typedef tbd::ParameterSet<__VA_ARGS__> name;


  /// A propertie set accepts a number of distinct properties as template parameters
  template<typename ...PARAMETERS> struct ParameterSet : PARAMETERS... 
  {
    ParameterSet(PARAMETERS&&..._properties) : 
      PARAMETERS(_properties)... {}

    template<typename FUNCTOR>
    void apply(FUNCTOR f) {}

    template<typename FUNCTOR>
    void apply(FUNCTOR f) const {}

    template<typename PARAMETER_TOKENS>
    void put(const PARAMETER_TOKENS& _tokens) {}

    template<typename PARAMETER_TOKEN, typename VALUE>
    bool get(const PARAMETER_TOKEN& _token, VALUE& _value) const {}
  };
  
  template<typename PARAMETER, typename...PARAMETERS>
  struct ParameterSet<PARAMETER,PARAMETERS...> : PARAMETER, ParameterSet<PARAMETERS...>
  {
  private:
    typedef ParameterSet<PARAMETERS...> parameterset_type;
    typedef PARAMETER parameter_type;

  public:
    ParameterSet() {}

    ParameterSet(const ParameterToken& _token)
    {
      put(_token);
    }

    ParameterSet(std::initializer_list<ParameterToken>&& _tokens) 
    {
      put(_tokens);
    }

    ParameterSet(const std::vector<ParameterToken>& _tokens) 
    {
      put(_tokens);
    }

    template<typename ARG, typename...ARGS>
    ParameterSet(ARG&& _arg, ARGS&&..._args) : 
      PARAMETER(_arg), 
      ParameterSet<PARAMETERS...>(_args...) {}

    void put(const ParameterToken& _token)
    {
      parameter_type::put(_token);
      parameterset_type::put(_token);
    }

    void put(const std::vector<ParameterToken>& _tokens)
    {
      parameter_type::put(_tokens);
      parameterset_type::put(_tokens);
    }

    void put(std::initializer_list<ParameterToken>&& _tokens)
    {
      parameter_type::put(_tokens);
      parameterset_type::put(_tokens);
    }

    template<typename VALUE>
    bool get(const ParameterToken& _token, VALUE& _value) const
    {
      return parameter_type::get(_token,_value) ? true : 
        parameterset_type::get(_token,_value);
    }

    template<typename FUNCTOR>
    void apply(FUNCTOR f) const
    {
      PARAMETER::apply(f);
      ParameterSet<PARAMETERS...>::apply(f);
    }

    template<typename FUNCTOR>
    void apply(FUNCTOR f)
    {
      PARAMETER::apply(f);
      ParameterSet<PARAMETERS...>::apply(f);
    }

    /// Load the properties from a config, a certain config path given
    template<typename CONFIG_PATH, typename CONFIG>
    bool load(const CONFIG_PATH& _path, const CONFIG& _config)
    {
      bool _updated = false;
      _updated |= PARAMETER::load(_path,_config);
      _updated |= ParameterSet<PARAMETERS...>::load(_path,_config);
      return _updated;
    }

    /// Saves the properties to a config
    template<typename CONFIG_PATH, typename CONFIG>
    void save(const CONFIG_PATH& _path, CONFIG& _config) const
    {
      PARAMETER::save(_path,_config);
      ParameterSet<PARAMETERS...>::save(_path,_config);
    }
  };

  template<typename PARAMETER>
  struct ParameterSet<PARAMETER> : PARAMETER
  {
  private:
    typedef PARAMETER parameter_type;
  public:
    ParameterSet() {}

    template<typename ARG>
    ParameterSet(ARG&& _arg) : PARAMETER(_arg) {}

    ParameterSet(const ParameterToken& _token)
    {
      put(_token);
    }

    ParameterSet(std::initializer_list<ParameterToken>&& _tokens) 
    {
      put(_tokens);
    }

    ParameterSet(const std::vector<ParameterToken>& _tokens) 
    {
      put(_tokens);
    }

    void put(const ParameterToken& _token)
    {
      parameter_type::put(_token);
    }

    void put(const std::vector<ParameterToken>& _tokens)
    {
      parameter_type::put(_tokens);
    }

    void put(std::initializer_list<ParameterToken>&& _tokens)
    {
      parameter_type::put(_tokens);
    }

    template<typename VALUE>
    bool get(const ParameterToken& _token, VALUE& _value) const
    {
      return parameter_type::get(_token,_value);
    }

    template<typename FUNCTOR>
    void apply(FUNCTOR f) const
    {
      parameter_type::apply(f);
    }

    template<typename FUNCTOR>
    void apply(FUNCTOR f)
    {
      parameter_type::apply(f);
    }

    /// Load the properties from a config, a certain config path given
    template<typename CONFIG_PATH, typename CONFIG>
    bool load(const CONFIG_PATH& _path, const CONFIG& _config)
    {
      return parameter_type::load(_path,_config);
    }

    /// Saves the properties to a config
    template<typename CONFIG_PATH, typename CONFIG>
    void save(const CONFIG_PATH& _path, CONFIG& _config) const
    {
      parameter_type::save(_path,_config);
    }
  };
}
