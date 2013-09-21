#pragma once

#include <list>
#include <string>
#include <boost/lexical_cast.hpp>

namespace tbd
{
  struct ParameterRule
  {
    typedef std::string token_type;
    typedef std::vector<token_type> tokens_type;

    ParameterRule(
        const token_type& _name,
        const token_type& _value,
        const token_type& _type,
        const tokens_type& _parameters = tokens_type()) : 
      name_(_name),
      value_(_value),
      type_(_type),
      parameters_(_parameters)
    {
    }
    
    template<typename VALUE>
    ParameterRule(
        const token_type& _name,
        const VALUE& _value,
        const token_type& _type,
        const tokens_type& _parameters = tokens_type()) : 
      name_(_name),
      type_(_type),
      parameters_(_parameters)
    {
      value_ = boost::lexical_cast<token_type>(_value);
    }
    
    friend bool operator<(const ParameterRule& _lhs, const ParameterRule& _rhs)
    {
      return _lhs.name() < _rhs.name();
    }

    friend bool operator==(const ParameterRule& _lhs, const ParameterRule& _rhs)
    {
      return _lhs.name() == _rhs.name() &&
        _lhs.type() == _rhs.type();
    }

    friend bool operator!=(const ParameterRule& _lhs, const ParameterRule& _rhs)
    {
      return !(_lhs == _rhs);
    }

    TBD_PROPERTY_REF_RO(token_type,name)
    TBD_PROPERTY_REF_RO(token_type,value)
    TBD_PROPERTY_REF_RO(token_type,type)
    TBD_PROPERTY_REF_RO(tokens_type,parameters)
  };

  typedef std::list<ParameterRule> ParameterRules;
}
