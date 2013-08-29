#pragma once

//#undef BOOST_PP_VARIADICS
#define BOOST_PP_VARIADICS 1

#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/variadic/size.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/preprocessor/tuple.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/ref.hpp>
#include <boost/bind.hpp>
#include "parameter.h"

#define REM(...) __VA_ARGS__
#define EAT(...)

// Retrieve the type
#define TYPEOF(x) DETAIL_TYPEOF(DETAIL_TYPEOF_PROBE x,)
#define DETAIL_TYPEOF(...) DETAIL_TYPEOF_HEAD(__VA_ARGS__)
#define DETAIL_TYPEOF_HEAD(x, ...) REM x
#define DETAIL_TYPEOF_PROBE(...) (__VA_ARGS__),
// Strip off the type
#define STRIP(x) EAT x
// Show the type without parenthesis
#define PAIR(x) REM x

#define TBD_PARAMETER_LIST(...) \
static constexpr int fields_n = BOOST_PP_VARIADIC_SIZE(__VA_ARGS__); \
friend struct tbd::detail::reflector; \
template<int N, class Self> \
struct field_data {}; \
BOOST_PP_SEQ_FOR_EACH_I(REFLECT_EACH, data, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

#define REFLECT_EACH(r, data, i, x) \
private:\
TYPEOF(x) BOOST_PP_CAT(STRIP(x),_); \
public:\
  auto STRIP(x)() const -> const decltype(BOOST_PP_CAT(STRIP(x),_))& { return BOOST_PP_CAT(STRIP(x),_); }\
private:\
template<class Self> \
struct field_data<i, Self> : tbd::ParameterInterface \
{ \
    Self & self; \
    typedef TYPEOF(x) type;\
    field_data(Self & self) : \
      self(self) {} \
    \
    std::string valueAsStr() const\
    { \
      std::stringstream ss;\
      ss << get();\
      return ss.str();\
    }\
    \
    typename tbd::detail::make_const<Self, type>::type & get() \
    { \
        return self.BOOST_PP_CAT(STRIP(x),_); \
    }\
    typename boost::add_const<type>::type & get() const \
    { \
        return self.BOOST_PP_CAT(STRIP(x),_); \
    }\
    const char * name() const \
    {\
        return BOOST_PP_STRINGIZE(STRIP(x)); \
    } \
}; \
 
#define TBD_EXPOSE_READONLY_EACH(r, data, x) \
  auto BOOST_PP_CAT(get_,x)() const -> const decltype(x)& { return x; }
#define TBD_EXPOSE_EACH(r, data, x) \
  auto BOOST_PP_CAT(get_,x)() -> decltype(x)& { return x; }\
  void BOOST_PP_CAT(set_,x)(const decltype(x)& BOOST_PP_CAT(_,x)) { x = BOOST_PP_CAT(_,x); }

#define TBD_EXPOSE_READONLY(...) \
  BOOST_PP_SEQ_FOR_EACH(TBD_EXPOSE_READONLY_EACH, _, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))

#define TBD_EXPOSE(...) \
  TBD_EXPOSE_READONLY(__VA_ARGS__)\
  BOOST_PP_SEQ_FOR_EACH(TBD_EXPOSE_EACH, _, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))


namespace tbd
{
  namespace detail
  {
    // A helper metafunction for adding const to a type
    template<class M, class T>
    struct make_const
    {
      typedef T type;
    };

    template<class M, class T>
    struct make_const<const M, T>
    {
      typedef typename boost::add_const<T>::type type;
    };

    struct reflector
    {
      //Get field_data at index N
      template<int N, class T>
      static typename T::template field_data<N, T> get_field_data(T& x)
      {
        return typename T::template field_data<N, T>(x);
      }

      // Get the number of fields
      template<class T>
      struct fields
      {
        static const int n = T::fields_n;
      };
    };

    struct field_visitor
    {
      template<class C, class Visitor, class I>
      void operator()(C& c, Visitor v, I)
      {
        v(reflector::get_field_data<I::value>(c));
      }
    };

    template<class C, class Visitor>
    void visit_each(C & c, Visitor v)
    {
      typedef boost::mpl::range_c<int,0,reflector::fields<C>::n> range;
      boost::mpl::for_each<range>(boost::bind<void>(field_visitor(), boost::ref(c), v, _1));
    }

    template<typename EVENTHANDLER>
    struct Parser
    {
      typedef std::string token_type;
      typedef std::map<token_type,token_type> tokens_type;

      Parser(const token_type& _prefix, const tokens_type& _tokens, EVENTHANDLER& _e) :
        prefix_(_prefix),
        tokens_(_tokens),
        e_(_e) {}

      Parser(const tokens_type& _tokens, EVENTHANDLER& _e) :
        tokens_(_tokens), e_(_e) {}

      template<typename F>
      void operator()(F _f)
      {
        token_type _key =
          (prefix_.empty()) ? std::string(_f.name()) :
          prefix_ + "." + std::string(_f.name());
        if (tokens_.count(_key) == 0) return;
        auto& _token = tokens_.at(_key);
        if (_token.empty()) return;
        std::stringstream ss(_token);
        ss >> _f.get();
        e_(_f);
      }

    private:
      token_type prefix_;
      const tokens_type& tokens_;
      EVENTHANDLER& e_;
    };

    template<typename CONFIG_PATH, typename CONFIG, typename EVENTHANDLER>
    struct ConfigSetter
    {
      ConfigSetter(bool& _updated, const CONFIG_PATH& _path, const CONFIG& _config, EVENTHANDLER& _e) :
        updated_(_updated),
        path_(_path),
        config_(_config),
        e_(_e) {}

      template<typename F>
      void operator()(F _f)
      {
        boost::optional<std::string> _value = config_.template get_optional<std::string>(path_ / CONFIG_PATH(_f.name()));
        if (!_value) return;
        if (_value.get() == _f.valueAsStr()) return;
        std::stringstream ss(_value.get());
        ss >> _f.get();
        e_(_f);
        updated_ = true;
      }
    private:
      bool& updated_;
      const CONFIG_PATH& path_;
      const CONFIG& config_;
      EVENTHANDLER& e_;
    };

    template<template<class> class TYPE_TO_STR>
    struct TypedTokenMapInserter
    {
      typedef std::string token_type;
      typedef std::map<token_type,std::pair<token_type,token_type>> typed_tokenmap_type;

      TypedTokenMapInserter(const token_type& _prefix, typed_tokenmap_type& _tokens) :
        prefix_(_prefix),
        tokens_(_tokens) {}

      TypedTokenMapInserter(typed_tokenmap_type& _tokens) :
        tokens_(_tokens) {}

      template<typename F>
      void operator()(const F& _f)
      {
        token_type _key =
          (prefix_.empty()) ? token_type(_f.name()) :
          prefix_ + "." + token_type(_f.name());
        tokens_.insert(make_pair(_key, // Name
                                 std::make_pair(
                                   _f.valueAsStr(), // Value
                                   TYPE_TO_STR<typename F::type>()() // Type
                                 )
                                ));
      }

    private:
      token_type prefix_;
      typed_tokenmap_type& tokens_;
    };

  }

  template<typename T>
  struct TypeToStr
  {
    std::string operator()()
    {
      return "var";
    }
  };

#define TBD_BASE_TYPETOSTR(type,str)\
    template<> struct TypeToStr<type> {\
      inline std::string operator()() { return str; }};
#define TBD_BASE_TYPETOSTR_(type)\
    TBD_BASE_TYPETOSTR(type,#type)

  TBD_BASE_TYPETOSTR_(int)
  TBD_BASE_TYPETOSTR_(bool)
  TBD_BASE_TYPETOSTR_(float)
  TBD_BASE_TYPETOSTR_(double)
  TBD_BASE_TYPETOSTR(std::string,"string")

  /**@brief SerializationInterface provides the
   *        interface for parse and print functions for all elements
   */
  struct SerializationInterface
  {
    typedef std::string typeid_type;
    typedef std::string token_type;
    typedef std::map<token_type,token_type> tokenmap_type;
    /// Parameter name, value, type
    typedef std::map<token_type,std::pair<token_type,token_type>> typed_tokenmap_type;

    virtual ~SerializationInterface() {}

    /**@brief Abstract method for parsing from a std::istream
     * @param[in] _is Input stream from which object is parsed
     */
    virtual void parse(std::istream& _is) {};

    /**@brief Abstract method for printing into a std::ostream
     * @param[out] _os Output stream
     */
    virtual void print(std::ostream& _os) const {};

    virtual void tokenMap(tokenmap_type& _tokens) const {};
    virtual void typedTokenMap(typed_tokenmap_type& _tokens) const {};
    virtual void parse(const tokenmap_type& _tokens) {};

    /// Overloap output stream operator for convenience
    friend std::ostream& operator<<(std::ostream& _os, const SerializationInterface& _rhs)
    {
      _rhs.print(_os);
      return _os;
    }

    /// Overloap input stream operator for convenience
    friend std::istream& operator>>(std::istream& _is, SerializationInterface& _rhs)
    {
      _rhs.parse(_is);
      return _is;
    }
  };

  template<typename T>
  struct EventHandler
  {
    EventHandler(T& _obj) : obj_(_obj) {}

    template<typename FIELD>
    void operator()(const FIELD& _f)
    {
    }

    T& obj()
    {
      return obj_;
    }
    T const& obj() const
    {
      return obj_;
    }

  private:
    T& obj_;
  };

  template<typename T, template<class> class EVENTHANDLER = EventHandler>
  struct Serializer : virtual SerializationInterface
  {
    typedef EVENTHANDLER<T> eventhandler_type;

    Serializer(T& _obj) : obj_(_obj) {}

    /**@brief Abstract method for printing into a std::ostream
     * @param[out] _os Output stream
     */
    virtual void print(std::ostream& _os) const
    {
      detail::visit_each(obj_,[&](const tbd::ParameterInterface& _f)
      {
        _os << _f.token() << ",";
      });
    }

    virtual void parse(std::istream& _is)
    {
      std::map<token_type,token_type> _tokens;
      tbd::parse(_is,_tokens,"([","])",",",1);
      parse(_tokens);
    }

    virtual void parse(const tokenmap_type& _tokens)
    {
      eventhandler_type _e(obj_);
      detail::visit_each(obj_,detail::Parser<eventhandler_type>(_tokens,_e));
    }

    bool hasParameter(const token_type& _parameter)
    {
      bool _found = false;
      detail::visit_each(obj_,[&](const tbd::ParameterInterface& _f)
      {
        if (token_type(_f.name()) == _parameter)
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
      eventhandler_type _e(obj_);
      detail::visit_each(obj_,detail::ConfigSetter<CONFIG_PATH,CONFIG,eventhandler_type>(_updated,_path,_config,_e));
      return _updated;
    }

    /**@brief Save parameter into a config
      *@param _path Config path (e.g. a std::string of the form my.path.to.param)
      *@param _config Config in which parameter (e.g. boost property tree)
     **/
    template<typename CONFIG_PATH, typename CONFIG>
    void save(const CONFIG_PATH& _path, CONFIG& _config) const
    {
      detail::visit_each(obj_,[&](const tbd::ParameterInterface& _f)
      {
        _config.put(_path / CONFIG_PATH(_f.name()),_f.valueAsStr());
      });
    }

    virtual void tokenMap(tokenmap_type& _tokens) const
    {
      detail::visit_each(obj_,[&](const tbd::ParameterInterface& _f)
      {
        _tokens.insert(make_pair(token_type(_f.name()),_f.valueAsStr()));
      });
    }

    virtual void typedTokenMap(typed_tokenmap_type& _tokens) const
    {
      detail::visit_each(obj_,detail::TypedTokenMapInserter<TypeToStr>(_tokens));
    }

    T& obj()
    {
      return obj_;
    }
    T const& obj() const
    {
      return obj_;
    }
  private:
    T& obj_;
  };

  template<typename T, typename BASE, template<class> class EVENTHANDLER = EventHandler>
  struct SerializerWithBase : private Serializer<T,EVENTHANDLER>, public BASE
  {
    typedef BASE base_type;
    typedef Serializer<T,EVENTHANDLER> inherited_type;
    template<typename...ARGS>
    SerializerWithBase(T& _obj, const ARGS&..._args) :
      inherited_type(_obj),
      base_type(_args...) {}

    typedef EVENTHANDLER<T> eventhandler_type;
    typedef std::string token_type;
    typedef std::map<token_type,std::pair<token_type,token_type>> typed_tokenmap_type;
    typedef std::map<token_type,token_type> tokenmap_type;

    /**@brief Abstract method for printing into a std::ostream
      * @param[out] _os Output stream
      */
    virtual void print(std::ostream& _os) const
    {
      BASE::print(_os);
      inherited_type::print(_os);
    }

    virtual void parse(std::istream& _is)
    {
      std::string _str;
      _is >> _str;
      std::istringstream _iss(_str);
      BASE::parse(_iss);
      _iss.str(_str);
      _iss.clear();
      inherited_type::parse(_iss);
    }

    void parse(const tokenmap_type& _tokens)
    {
      BASE::parse(_tokens);
      inherited_type::parse(_tokens);
    }

    bool hasParameter(const token_type& _parameter)
    {
      if (inherited_type::hasParameter(_parameter)) return true;
      return BASE::hasParameter(_parameter);
    }

    template<typename CONFIG_PATH, typename CONFIG>
    bool load(const CONFIG_PATH& _path, const CONFIG& _config)
    {
      bool _updated = BASE::load(_path,_config);
      _updated |= inherited_type::load(_path,_config);
      return _updated;
    }

    /**@brief Save parameter into a config
      *@param _path Config path (e.g. a std::string of the form my.path.to.param)
      *@param _config Config in which parameter (e.g. boost property tree)
     **/
    template<typename CONFIG_PATH, typename CONFIG>
    void save(const CONFIG_PATH& _path, CONFIG& _config) const
    {
      BASE::save(_path,_config);
      inherited_type::save(_path,_config);
    }

    void tokenMap(tokenmap_type& _tokens) const
    {
      BASE::tokenMap(_tokens);
      inherited_type::tokenMap(_tokens);
    }

    void typedTokenMap(typed_tokenmap_type& _tokens) const
    {
      BASE::typedTokenMap(_tokens);
      inherited_type::typedTokenMap(_tokens);
    }
  };

}

