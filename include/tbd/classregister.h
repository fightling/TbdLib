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

#ifndef H_TBD_CLASSREGISTER_H
#define H_TBD_CLASSREGISTER_H

#include <map>
#include <memory>
#include <functional>

namespace tbd
{
  template<typename REGISTRY, typename T, typename...ARGS>
  struct ClassRegistrar;

  template<typename KEY, typename INTERFACE, typename...ARGS>
  struct ClassRegistry
  {
    typedef KEY key_type;
    typedef INTERFACE interface_type;
    typedef ClassRegistry<key_type,interface_type> type;
    typedef std::unique_ptr<INTERFACE> ptr_type;
    typedef std::function<ptr_type(ARGS...)> constructor_type;
    typedef std::map<key_type,constructor_type> class_map_type;

    template<typename T>
    using registrar_type = ClassRegistrar<type,T,ARGS...>;

    static bool exists(const key_type& _key)
    {
      return privClasses().count(_key) > 0;
    }

    template<typename T>
      static void reg(const key_type& _key)
    {
      if (exists(_key) > 0) return;
      privClasses()[_key] = registrar_type<T>::create;
    }

    template<typename T>
    static void reg()
    {
      reg<T>(T::typeId());
    }

    static ptr_type create(const key_type& _key, const ARGS&..._args)
    {
      if (!exists(_key)) return ptr_type(nullptr);
      return privClasses()[_key](_args...);
    }

    static class_map_type const& classes()
    {
      return privClasses();
    }

  private:
    static class_map_type& privClasses()
    {
      static class_map_type _classes;
      return _classes;
    }
  };

  template<typename REGISTRY, typename T, typename...ARGS>
  struct ClassRegistrar
  {
    typedef REGISTRY registry_type;
    typedef typename registry_type::key_type key_type;
    typedef typename registry_type::interface_type interface_type;
    typedef typename registry_type::ptr_type ptr_type;
    static ClassRegistrar* instance_;
    static bool registered_;

    ~ClassRegistrar()
    {
      instance_ = nullptr;
      registered_ = false;
    }

    ClassRegistrar()
    {
      if (!registered_) registry_type::template reg<T>(T::typeId());
    }

    static ptr_type create(const ARGS&..._args)
    {
      return ptr_type(new T(_args...));
    }

    static key_type typeId()
    {
      return key_type();
    }

  private:
    ClassRegistrar& operator=(const ClassRegistrar&)
    {
      return *this;
    }

    static ClassRegistrar* instance()
    {
      static ClassRegistrar theInstance;
      instance_ = &theInstance;
      registered_ = true;
      return instance_;
    }
  };

  template<typename R, typename T, typename...ARGS>
  ClassRegistrar<R,T,ARGS...>* ClassRegistrar<R,T,ARGS...>::instance_
  = ClassRegistrar<R,T,ARGS...>::instance();
  template<typename R, typename T, typename...ARGS>
  bool ClassRegistrar<R,T,ARGS...>::registered_ = false;
}

#endif /* H_TBD_CLASSREGISTER_H */

