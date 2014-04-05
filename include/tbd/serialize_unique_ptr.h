#pragma once

#include "serialize.h"
#include "classregister.h"
#include <memory>

namespace tbd
{
  namespace detail
  {
    template<typename T>
    struct ParameterTypeInfo
    {
      typedef tbd::ClassRegistry<std::string,T> registry_type;

      template<typename PTR>
      std::string operator()(const PTR& _ptr)
      {
        return _ptr->getTypeId();
      }
    };
  }

    template<typename T>
    struct Serialize<std::unique_ptr<T>>
    {
      typedef std::unique_ptr<T> ptr_type;
      typedef typename detail::ParameterTypeInfo<T>::registry_type registry_type;

      static bool load(ptr_type& _ptr, const tbd::ConfigPath& _path, tbd::Config const& _config)
      { 
        auto&& _typeId = _config.get<std::string>(_path / "typeid");
        _ptr = registry_type::create(_typeId);
        if (!_ptr) return true; 
        return _ptr->load(_path/ConfigPath(_typeId),_config); 
      }

      static void save(const ptr_type& _ptr, const tbd::ConfigPath& _path, tbd::Config& _config) 
      {
        if (!_ptr) return;
        auto&& _typeId = detail::ParameterTypeInfo<T>()(_ptr);
        _config.put(_path/"typeid", _typeId);
        _ptr->save(_path/ConfigPath(_typeId),_config);
      }
    };

  
  template<typename REGISTRY>
  struct SerializeRegistered
  {
    typedef REGISTRY registry_type;
    typedef typename REGISTRY::interface_type interface_type;
    typedef typename REGISTRY::ptr_type ptr_type;

    template<typename...ARGS>
    static ptr_type load(const ConfigPath& _path, const Config& _config, const ARGS&..._args)
    {
      auto&& _typeId = _config.get<std::string>(_path / "typeid");
      auto _ptr = registry_type::create(_typeId,_args...);
      if (!_ptr) return ptr_type(nullptr); 
      _ptr->load(_path/ConfigPath(_typeId),_config); 
      return std::move(_ptr);
    }

    template<typename TYPEID>
    static void save(const ptr_type& _ptr, const TYPEID& _typeId, const ConfigPath& _path, Config& _config) 
    {
      if (!_ptr) return;
      _config.put(_path/"typeid", _typeId);
      _ptr->save(_path/ConfigPath(_typeId),_config);
    }
  };

}
