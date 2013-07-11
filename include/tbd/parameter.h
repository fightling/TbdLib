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

#include <iostream>
#include <string>
#include <utility>
#include <sstream>
#include <vector>
#include "property.h"
#include "parse_utils.h"

namespace tbd  
{
  /// Basic ParameterInterface (does not store any data)
  struct ParameterInterface 
  {
    virtual ~ParameterInterface() {}

    /// Name of the parameter
    virtual char const* name() const = 0;

    /// Returns parameter as token of the form name=value
    std::string token() const
    {
      std::stringstream _ss;
      _ss << std::string(name()) << '=' << valueAsStr();
      return _ss.str();
    }

    /// Value needs to be convertable into a string
    virtual std::string valueAsStr() const = 0;

    /// Default value needs to be convertable into a string
    virtual std::string defaultAsStr() const = 0;
  };



  template<typename T>
  struct TypedParameterInterface : ParameterInterface
  {
    typedef T var_type;
    TypedParameterInterface() {}
    TypedParameterInterface(const var_type& _value) : value_(_value) {}
    
    /**@brief Return value as string 
     * @detail Uses std::stringstream
     **/
    std::string valueAsStr() const
    {
      std::stringstream _ss;
      _ss << value();
      return _ss.str();
    }
    
    /// A typed parameter needs to return a default value
    virtual var_type def() const = 0;

    /// Returns defaults value as string
    std::string defaultAsStr() const
    {
      std::stringstream _ss;
      _ss << def();
      return _ss.str();
    }

    /**@brief Load parameter from a config
      *@param _path Config path (e.g. a std::string of the form my.path.to.param)
      *@param _config Config in which parameter (e.g. boost property tree)
      *@returns Returns true if parameter changed
      */
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
    
    /**@brief Save parameter into a config 
      *@param _path Config path (e.g. a std::string of the form my.path.to.param)
      *@param _config Config in which parameter (e.g. boost property tree)
     **/
    template<typename CONFIG_PATH, typename CONFIG>
    void save(const CONFIG_PATH& _path, CONFIG& _config) const
    {
      _config.put(_path / CONFIG_PATH(name()),value_);
    }
     
    /// Changes value by a token of the form name=value
    void put(const ParameterToken& _token)
    {\
      auto&& _tokens = tbd::splitToken(_token);
      if (_tokens.first.empty() || _tokens.second.empty()) return;
      if (_tokens.first == std::string(name()))
      {
        std::stringstream ss(_tokens.second);
        ss >> value();
      }
    }

    /// Changes value by a token of the form name=value
    void put(const std::vector<ParameterToken>& _tokens)
    {
      for (auto& _token : _tokens)
      {
        put(_token);
      }
    }

    /// Retrieve value from a token_list
    void put(std::initializer_list<ParameterToken>&& _tokens)
    {
      for (auto& _token : _tokens)
      {
        put(_token);
      }
    }

    /// Retrieve value from a token
    template<typename VALUE>
    bool get(const ParameterToken& _token, VALUE& _value) const
    {
      auto&& _splitToken = splitToken(_token);
      if (_splitToken.first != std::string(name())) return false;
      std::stringstream ss(_splitToken.second);
      ss >> _value;
      return true;
    }
    
    /**@brief  Apply as functor onto this parameter (mutable version)
     * @detail Functor accepts a ParameterInterface as argument
     */
    template<typename FUNCTOR>
    void apply(FUNCTOR f)
    {
      f(*this); 
    }

    /**@brief  Apply as functor onto this parameter (const version)
     * @detail Functor accepts a ParameterInterface as argument
     */
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
    inline char const* name() const { return #var_name; }\
    inline var_type def() const { return var_def; }\
    inline var_type& var_name() { return value(); }\
    inline const var_type& var_name() const { return value(); }\
  };

/**@brief Macro for defining an array parameter
   @detail Should be declared in anonymous namespace 
*/
#define TBD_ARRAY_PARAMETER(var_type,parameter_name,var_name,...)\
  struct parameter_name : tbd::TypedParameterInterface<var_type>\
  {\
    typedef var_type type;\
    typedef tbd::TypedParameterInterface<var_type> inherited_type;\
    parameter_name() : inherited_type({__VA_ARGS__}) {} \
    parameter_name(const var_type& _value) : \
      inherited_type(_value) {}\
    inline char const* name() const { return #var_name; }\
    inline var_type def() const { return {__VA_ARGS__}; }\
    inline var_type& var_name() { return value(); }\
    inline const var_type& var_name() const { return value(); }\
  };

/// Macro for defining a parameter set
#define  TBD_PARAMETERSET(name,...)\
  typedef tbd::ParameterSet<__VA_ARGS__> name;

  /// A parameter set accepts a number of distinct properties as template parameters
  template<typename ...PARAMETERS> struct ParameterSet : PARAMETERS... 
  {
    template<typename TOKENS>
    ParameterSet(const TOKENS& _tokens) {}

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
  
  /**@brief A parameter set takes a number of parameters as templates parameter
   * @detail A parameter must implement a parameter interface
   */
  template<typename PARAMETER, typename...PARAMETERS>
  struct ParameterSet<PARAMETER,PARAMETERS...> : PARAMETER, ParameterSet<PARAMETERS...>
  {
  private:
    typedef ParameterSet<PARAMETERS...> parameterset_type;
    typedef PARAMETER parameter_type;
  public:
    ParameterSet() {}

    /// Constructs a parameter set from a initializer_list of tokens
    ParameterSet(std::initializer_list<ParameterToken>&& _tokens) 
    {
      put(_tokens);
    }

    /// Constructs a parameter set from a list of tokens
    ParameterSet(const std::vector<ParameterToken>& _tokens) 
    {
      put(_tokens);
    }

    /// Constructs a parameter by directly passing values to each parameter
    template<typename ARG, typename...ARGS>
    ParameterSet(ARG&& _arg, ARGS&&..._args) : 
      parameter_type(_arg), 
      ParameterSet<PARAMETERS...>(_args...) {}

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

    /**@brief  Apply as functor onto this parameter (const version)
     * @detail Functor accepts a ParameterInterface as argument
     */
    template<typename FUNCTOR>
    void apply(FUNCTOR f) const
    {
      parameter_type::apply(f);
      ParameterSet<PARAMETERS...>::apply(f);
    }

    /**@brief  Apply as functor onto this parameter (mutable version)
     * @detail Functor accepts a ParameterInterface as argument
     */
    template<typename FUNCTOR>
    void apply(FUNCTOR f)
    {
      parameter_type::apply(f);
      ParameterSet<PARAMETERS...>::apply(f);
    }

    /// Load the properties from a config, a certain config path given
    template<typename CONFIG_PATH, typename CONFIG>
    bool load(const CONFIG_PATH& _path, const CONFIG& _config)
    {
      bool _updated = false;
      _updated |= parameter_type::load(_path,_config);
      _updated |= ParameterSet<PARAMETERS...>::load(_path,_config);
      return _updated;
    }

    /// Saves the properties to a config
    template<typename CONFIG_PATH, typename CONFIG>
    void save(const CONFIG_PATH& _path, CONFIG& _config) const
    {
      parameter_type::save(_path,_config);
      ParameterSet<PARAMETERS...>::save(_path,_config);
    }
  };

  /// Parameter set for a single paramater (specialized template)
  template<typename PARAMETER>
  struct ParameterSet<PARAMETER> : PARAMETER
  {
  private:
    typedef PARAMETER parameter_type;
  public:
    ParameterSet() {}

    /// Constructs a parameter by directly passing values to each parameter
    template<typename ARG>
    ParameterSet(ARG&& _arg) : PARAMETER(_arg) {}

    /// Constructs a parameter set from a initializer_list of tokens
    ParameterSet(std::initializer_list<ParameterToken>&& _tokens) 
    {
      put(_tokens);
    }

    /// Constructs a parameter set from a list of tokens
    ParameterSet(const std::vector<ParameterToken>& _tokens) 
    {
      put(_tokens);
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
