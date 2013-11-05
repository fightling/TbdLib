#pragma once

#include "serialize.h"

namespace tbd
{
  template<typename...SUBOBJECTS>
  struct SubobjectSerializer {};

  template<typename SUBOBJECT>
  struct SubobjectSerializer<SUBOBJECT> : virtual SerializationInterface
  {
    typedef std::string token_type;
    typedef std::map<token_type,token_type> tokenmap_type;
    typedef ParameterRules parameterrules_type;

    explicit SubobjectSerializer(SUBOBJECT& _subobject) :
      subobject_(_subobject) {}

    virtual void print(std::ostream& _os) const
    {
      detail::visit_each(subobject_,[&](const tbd::ParameterInterface& _f)
      {
        _os << withPath(_f.token()) << ",";
      });
    }

    virtual void parse(std::istream& _is)
    {
      std::map<token_type,token_type> _tokens;
      tbd::parse(_is,_tokens,"([","])",",",1);
      parse(_tokens);
    }

    void parse(const tokenmap_type& _tokens)
    {
      detail::visit_each(subobject_,detail::Parser(subobject_.typeId(),_tokens));
    }

    bool hasParameter(const token_type& _parameter)
    {
      bool _found = false;
      detail::visit_each(subobject_,[&](const tbd::ParameterInterface& _f)
      {
        if (withPath(_f.name()) == _parameter)
        {
          _found = true;
          return;
        }
      });
      return _found;
    }

    template<typename CONFIG_PATH, typename CONFIG>
    bool load(const CONFIG_PATH& _path, const CONFIG& _config)
    {
      bool _updated = false;
      detail::visit_each(subobject_,
          detail::ConfigSetter<
            CONFIG_PATH,
            CONFIG>
            (_updated,_path / SUBOBJECT::typeId(),_config));
      return _updated;
    }

    /**@brief Save parameter into a config
      *@param _path Config path (e.g. a std::string of the form my.path.to.param)
      *@param _config Config in which parameter (e.g. boost property tree)
     **/
    template<typename CONFIG_PATH, typename CONFIG>
    void save(const CONFIG_PATH& _path, CONFIG& _config) const
    {
      detail::visit_each(subobject_,[&](const tbd::ParameterInterface& _f)
      {
        _config.put(_path / CONFIG_PATH(withPath(_f.name())),_f.valueAsStr());
      });
    }

    void tokenMap(tokenmap_type& _tokens) const
    {
      detail::visit_each(subobject_,[&](const tbd::ParameterInterface& _f)
      {
        _tokens.insert(make_pair(withPath(_f.name()),_f.valueAsStr()));
      });
    }

    template<template<class> class TYPE_TO_STR = TypeToStr>
      void parameterRules(parameterrules_type& _rules) const
    {
      detail::visit_each(subobject_,detail::ParameterRuleInserter<TYPE_TO_STR>(subobject_.typeId(),_rules));
    }

  private:
    static token_type withPath(const token_type& _token) 
    {
      return SUBOBJECT::typeId() +"."+ _token;
    }
    SUBOBJECT& subobject_;
  };

  template<typename SUBOBJECT, typename...SUBOBJECTS>
  struct SubobjectSerializer<SUBOBJECT,SUBOBJECTS...> :
    private SubobjectSerializer<SUBOBJECT>,
    private SubobjectSerializer<SUBOBJECTS...>
  {
    typedef SubobjectSerializer<SUBOBJECT> head_type;
    typedef SubobjectSerializer<SUBOBJECTS...> tail_type;
    typedef std::string token_type;
    typedef std::map<token_type,token_type> tokenmap_type;
    typedef ParameterRules parameterrules_type;

    explicit SubobjectSerializer(SUBOBJECT& _subobject,
        SUBOBJECTS&..._subobjects) :
      head_type(_subobject),
      tail_type(_subobjects...) {}

    void print(std::ostream& _os) const
    {
      head_type::print(_os);
      tail_type::print(_os);
    }
    
    void parse(std::istream& _is)
    {
      std::map<token_type,token_type> _tokens;
      tbd::parse(_is,_tokens,"([","])",",",1);
      parse(_tokens);
    }
    
    void parse(const tokenmap_type& _tokens)
    {
      head_type::parse(_tokens);
      tail_type::parse(_tokens);
    }

    bool hasParameter(const token_type& _parameter)
    {
      return head_type::hasParameter(_parameter) || tail_type::hasParameter(_parameter);
    }

    template<typename CONFIG_PATH, typename CONFIG>
    bool load(const CONFIG_PATH& _path, const CONFIG& _config)
    {
      return head_type::load(_path,_config) || 
             tail_type::load(_path,_config);
    }

    /**@brief Save parameter into a config
      *@param _path Config path (e.g. a std::string of the form my.path.to.param)
      *@param _config Config in which parameter (e.g. boost property tree)
     **/
    template<typename CONFIG_PATH, typename CONFIG>
    void save(const CONFIG_PATH& _path, CONFIG& _config) const
    {
      head_type::save(_path,_config); 
      tail_type::save(_path,_config);
    }
  
    void tokenMap(tokenmap_type& _tokens) const
    {
      head_type::tokenMap(_tokens);
      tail_type::tokenMap(_tokens);
    }

    template<template<class> class TYPE_TO_STR = TypeToStr>
    void parameterRules(parameterrules_type& _rules) const
    {
      head_type::template parameterRules<TYPE_TO_STR>(_rules);
      tail_type::template parameterRules<TYPE_TO_STR>(_rules);
    }
  };
  
  template<typename T, typename...SUBOBJECTS>
  struct SerializeSubobjects : 
    public tbd::Serializer<T>,
    private SubobjectSerializer<SUBOBJECTS...>
  {
    typedef tbd::Serializer<T> base_type;
    typedef SubobjectSerializer<SUBOBJECTS...> subobjects_type;

    SerializeSubobjects(T& _obj, SUBOBJECTS&..._subobjects) :
      base_type(_obj),
      subobjects_type(_subobjects...) {}

    typedef std::string token_type;
    typedef std::map<token_type,token_type> tokenmap_type;
    typedef ParameterRules parameterrules_type;

    /**@brief Abstract method for printing into a std::ostream
      * @param[out] _os Output stream
      */
    virtual void print(std::ostream& _os) const
    {
      base_type::print(_os);
      subobjects_type::print(_os);
    }
    
    virtual void parse(std::istream& _is)
    {
      tokenmap_type _tokens;
      tbd::parse(_is,_tokens,"([","])",",",1);
      parse(_tokens);
    }
    
    void parse(const tokenmap_type& _tokens)
    {
      base_type::parse(_tokens);
      subobjects_type::parse(_tokens);
    }

    bool hasParameter(const token_type& _parameter)
    {
      return 
        base_type::hasParameter(_parameter) || 
        subobjects_type::hasParameter(_parameter);
    }

    template<typename CONFIG_PATH, typename CONFIG>
    bool load(const CONFIG_PATH& _path, const CONFIG& _config)
    {
      return base_type::load(_path,_config) || 
             subobjects_type::load(_path,_config);
    }

    /**@brief Save parameter into a config
      *@param _path Config path (e.g. a std::string of the form my.path.to.param)
      *@param _config Config in which parameter (e.g. boost property tree)
     **/
    template<typename CONFIG_PATH, typename CONFIG>
    void save(const CONFIG_PATH& _path, CONFIG& _config) const
    {
      return base_type::save(_path,_config) || 
             subobjects_type::save(_path,_config);
    }

    void tokenMap(tokenmap_type& _tokens) const
    {
      base_type::tokenMap(_tokens);
      subobjects_type::tokenMap(_tokens);
    }

    template<template<class> class TYPE_TO_STR = TypeToStr>
      void parameterRules(parameterrules_type& _rules) const 
    {
      base_type::template parameterRules<TYPE_TO_STR>(_rules);
      subobjects_type::template parameterRules<TYPE_TO_STR>(_rules);
    }
  };
}
