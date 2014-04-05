#pragma once

//#undef BOOST_PP_VARIADICS
#define BOOST_PP_VARIADICS 1

#include <boost/lexical_cast.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/variadic/size.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/preprocessor/tuple.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/ref.hpp>
#include <boost/bind.hpp>
#include <map>
#include "config.h"

namespace tbd
{
  /**@brief SerializationInterface provides the
   *        interface for load and save functions for all elements
   */
  struct SerializationInterface
  {
    virtual bool load(const ConfigPath& _path, Config const& _config) = 0;
    virtual void save(const ConfigPath& _path, Config& _config) const = 0;
    virtual void additionalParameters(Config&) const {};
  };

  namespace detail
  {
    template<bool HAS_SERIALIZATION_INTERFACE = true>
    struct Load
    {
      template<typename T>
      bool operator()(T& _t, const tbd::ConfigPath& _path, tbd::Config const& _config)
      {
        return _t.load(_path,_config);
      }
    };

    template<>
    struct Load<false>
    {
      template<typename T>
      bool operator()(T& _t, const tbd::ConfigPath& _path, tbd::Config const& _config)
      {
        auto&& _valueAsStr = _config.get_optional<std::string>(_path);
        if (!_valueAsStr) return false;
        auto&& _value = boost::lexical_cast<T>(_valueAsStr.get());
        if (_t == _value) return false;
        _t=_value;
        return true;
      }
    };

    template<bool HAS_SERIALIZATION_INTERFACE = true>
    struct Save
    {
      template<typename T, typename CONFIG>
      void operator()(const T& _t, const tbd::ConfigPath& _path, CONFIG& _config)
      {
        return _t.save(_path,_config);
      }
    };

    template<>
    struct Save<false>
    {
      template<typename T, typename CONFIG>
      void operator()(const T& _t, const tbd::ConfigPath& _path, CONFIG& _config)
      {
        _config.put(_path,boost::lexical_cast<std::string>(_t));
      }
    };
  }

  template<typename T>
  struct Serialize
  {
    typedef std::string token_type;
    constexpr static bool hasSerialization()
    {
      return std::is_base_of<SerializationInterface,T>::value;
    }

    template<typename CONFIG>
    static bool load(T& _t, const tbd::ConfigPath& _path, CONFIG const& _config)
    {
      return detail::Load<hasSerialization()>()(_t,_path,_config);
    }

    template<typename CONFIG>
    static void save(const T& _t, const tbd::ConfigPath& _path, CONFIG& _config)
    {
      detail::Save<hasSerialization()>()(_t,_path,_config);
    }
  };

  /// Specialize float and double to assure that a common locale is used
  template<>
  struct Serialize<float>
  {
    template<typename T, typename CONFIG>
    static bool load(T& _t, const tbd::ConfigPath& _path, CONFIG const& _config)
    {
      auto&& _valueAsStr = _config.template get_optional<std::string>(_path);
      if (!_valueAsStr) return false;
      
      T _new;
      std::istringstream _is(_valueAsStr.get());
      _is >> std::boolalpha >> _new;
      if (_t != _new)
      {
        _t = _new;
        return true;
      }
      return false;
    }
    
    template<typename T, typename CONFIG>
    static void save(const T& _t, const tbd::ConfigPath& _path, CONFIG& _config)
    {
      std::ostringstream _os;
      _os << std::boolalpha << _t;
      detail::Save<false>()(_os.str(),_path,_config);
    }
  };

  template<>
  struct Serialize<double> : Serialize<float> {};
  
  template<>
  struct Serialize<bool> : Serialize<float> {};

  template<typename T>
  struct Serialize<std::vector<T>>
  {
    typedef std::vector<T> type;

    static bool load(type& _ts, const tbd::ConfigPath& _path, tbd::Config const& _config)
    {
      bool _updated = false;
      _ts.clear();
      size_t _number = _config.get<size_t>(_path / "number");
      _ts.reserve(_number);
      for (size_t _idx = 0; _idx < _number; ++_idx)
      {
        T _t;
        _updated |= Serialize<T>::load(_t,_path / ConfigPath(std::to_string(_idx)),_config);
        _ts.push_back(std::move(_t));
      }
      return _updated;
    }

    static void save(const type& _ts, const tbd::ConfigPath& _path, tbd::Config& _config)
    {
      _config.put(_path / "number",_ts.size());
      for (size_t _idx = 0; _idx < _ts.size(); ++_idx)
      {
        Serialize<T>::save(_ts[_idx],_path / ConfigPath(std::to_string(_idx)),_config);
      }
    }
  };

  template<typename T>
  struct TypeInfo
  {
    typedef std::string token_type;
    token_type operator()()
    {
      return token_type();
    }
  };
}

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
private:\
static constexpr int fields_n = BOOST_PP_VARIADIC_SIZE(__VA_ARGS__); \
friend struct tbd::detail::reflector; \
template<int N, class Self> \
struct field_data {}; \
BOOST_PP_SEQ_FOR_EACH_I(REFLECT_EACH, data, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))
/*public:\
  typeinfo_map_type typeInfo() const\
  {\
    static typeinfo_map_type _typeInfo;\
    if (_typeInfo.empty())\
    {\
      BOOST_PP_SEQ_FOR_EACH_I(PARAMETER_EACH, data, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))\
    }\
    return _typeInfo;\
  }\
private:\*/

#define PARAMETER_EACH(r, data, i, x)\
  _typeInfo[BOOST_PP_STRINGIZE(STRIP(x))] = tbd::TypeInfo<TYPEOF(x)>()() ?\
    tbd::TypeInfo<TYPEOF(x)>()() : BOOST_PP_STRINGIZE(TYPEOF(x));

#define REFLECT_EACH(r, data, i, x) \
private:\
TYPEOF(x) BOOST_PP_CAT(STRIP(x),_); \
public:\
  auto STRIP(x)() const -> const decltype(BOOST_PP_CAT(STRIP(x),_))& { return BOOST_PP_CAT(STRIP(x),_); }\
private:\
template<class Self> \
struct field_data<i, Self> : private \
  tbd::Serialize<TYPEOF(x)> \
{ \
    typedef TYPEOF(x) type;\
    typedef tbd::Serialize<type> base_type;\
    \
    static bool load(Self& _self, const tbd::ConfigPath& _path, tbd::Config const& _config)\
    { \
      return base_type::load(get(_self),_path / name(),_config);\
    }\
    static void save(const Self& _self, const tbd::ConfigPath& _path, tbd::Config& _config)\
    {\
      base_type::save(get_const(_self),_path / name(),_config);\
    }\
    static const char * name() \
    {\
        return BOOST_PP_STRINGIZE(STRIP(x)); \
    }\
    static std::string typeId() \
    {\
      return !tbd::TypeInfo<TYPEOF(x)>()().empty() ?\
        tbd::TypeInfo<TYPEOF(x)>()() : BOOST_PP_STRINGIZE(TYPEOF(x));\
    }\
    static type const& get_const(Self const& _self)\
    {\
      return _self.BOOST_PP_CAT(STRIP(x),_);\
    }\
    static type& get(Self& _self)\
    {\
      return _self.BOOST_PP_CAT(STRIP(x),_);\
    }\
    static void const* get_void_const(Self const& _self)\
    {\
      return static_cast<void const*>(&_self.BOOST_PP_CAT(STRIP(x),_));\
    }\
    static void* get_void(Self& _self)\
    {\
      return static_cast<void*>(&_self.BOOST_PP_CAT(STRIP(x),_));\
    }\
};

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
      static typename T::template field_data<N, T> get_field_data(T&)
      {
        return typename T::template field_data<N, T>();
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

    struct HasParameter
    {
      typedef std::string token_type;
      HasParameter(bool& _has, token_type const& _parameter) :
        parameter_(_parameter),
        has_(_has) {}

      template<typename F>
      void operator()(const F& _f)
      {
        has_ |= token_type(_f.name()) == parameter_;
      }

    private:
      std::string const& parameter_;
      bool& has_;
    };

    struct ParameterTypeId
    {
      typedef std::string token_type;
      ParameterTypeId(token_type const& _parameter, token_type& _typeId) :
        parameter_(_parameter),
        typeId_(_typeId) {}

      template<typename F>
      void operator()(const F& _f)
      {
        if (_f.name() == parameter_)
          typeId_ = _f.typeId();
      }

    private:
      token_type const& parameter_;
      token_type& typeId_;
    };



    template<typename T>
    struct ParameterFromConfig
    {
      ParameterFromConfig(T& _t, bool& _updated, const ConfigPath& _path, const Config& _config) :
        t_(_t),
        updated_(_updated),
        path_(_path),
        config_(_config) {}

      template<typename F>
      void operator()(F _f)
      {
        updated_ |= _f.load(t_,path_,config_);
      }
    private:
      T& t_;
      bool& updated_;
      const ConfigPath& path_;
      const Config& config_;
    };

    template<typename T>
    struct ParameterToConfig
    {
      ParameterToConfig(const T& _t, const ConfigPath& _path, Config& _config) :
        t_(_t),
        path_(_path),
        config_(_config) {}

      template<typename F>
      void operator()(const F& _f)
      {
        _f.save(t_,path_,config_);
      }
    private:
      T const& t_;
      const ConfigPath& path_;
      Config& config_;
    };

    struct AdditionalParameters
    {
      AdditionalParameters(Config& _config) :
        config_(_config) {}

      template<typename F>
      void operator()(const F& _f)
      {
        config_.put(ConfigPath(_f.name()) / "type",_f.typeId());
      }
    private:
      Config& config_;
    };

  }

  template<typename T>
  struct Serializer : virtual SerializationInterface
  {
    typedef std::string token_type;

    Serializer()
    {
      //buildParameterMap();
    }

    bool hasParameter(const token_type& _id) const
    {
      bool _found = false;
      detail::visit_each(derived_const(),detail::HasParameter(_found,_id));
      return _found;
    }
  
    token_type parameterType(const token_type& _id) const
    {
      std::string _type;
      detail::visit_each(derived_const(),detail::ParameterTypeId(_id,_type));
      return _type;
    }

    virtual bool load(const ConfigPath& _path, const Config& _config)
    {
      bool _updated = false;
      detail::visit_each(derived(),detail::ParameterFromConfig<T>(derived(),_updated,_path,_config));
      return _updated;
    }

    /**@brief Save parameter into a config
      *@param _path Config path (e.g. a std::string of the form my.path.to.param)
      *@param _config Config in which parameter (e.g. boost property tree)
     **/
    virtual void save(const ConfigPath& _path, Config& _config) const
    {
      detail::visit_each(derived_const(),detail::ParameterToConfig<T>(derived_const(),_path,_config));
    }

    virtual void additionalParameters(Config& _config) const
    {
      detail::visit_each(derived_const(),detail::AdditionalParameters(_config));
    }

  private:
    T& derived()
    {
      return static_cast<T&>(*this);
    }
    T const& derived_const() const
    {
      return static_cast<T const&>(*this);
    }
  };

  namespace parameter
  {
    typedef std::map<std::string,std::string> ParameterMap;

    template<typename CONFIG, typename NAME>
    void make(
      CONFIG& _config,
      const NAME& _name,
      ParameterMap const& _map = ParameterMap())
    {
      for (auto& _keyValue : _map)
      {
        _config.put(tbd::ConfigPath(_name) / tbd::ConfigPath(_keyValue.first), _keyValue.second);
      }
    }

    template<typename SERIALIZER, typename CONFIG, typename NAME>
    void make(
      SERIALIZER const& _serializer,   
      CONFIG& _config,
      const NAME& _name,
      ParameterMap const& _map = ParameterMap())
    {
      _config.put(tbd::ConfigPath(_name) / "type",_serializer.parameterType(_name));
      make(_config,_name,_map);
    }
  }

}

