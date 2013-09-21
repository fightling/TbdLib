#pragma once

#include <list>
#include <string>

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
      type_(_type)
    {
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
