/*
 * config.h
 *
 *  Created on: March 2, 2012
 *      Author: winkelmann
 *
 *      MWChange:
 *      	Allow comment lines in user config file
 *      	Trim lines of config file wit h
 */
#pragma once

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "property.h"


namespace tbd 
{
  typedef boost::property_tree::path ConfigPath;

  /// Config class for reading and writing JSON files and storing in a boost property tree
  struct Config : boost::property_tree::ptree
	{
    typedef ConfigPath path_type;
		
    Config(const std::string& filename = std::string())
		{
			if (!filename.empty()) load(filename);
		}

    void fromStr(const std::string& _str)
    {
      std::istringstream _iss(_str);
      boost::property_tree::json_parser::read_json(_iss,static_cast<boost::property_tree::ptree&>(*this));
    }

    template<typename CNTR>
    void put_array(const path_type& _path, const CNTR& _cntr)
    {
      boost::property_tree::ptree&& _children = fromArray(_cntr);
      put_child(_path,_children);
    }

    template<typename CNTR>
    CNTR get_array(const path_type& _path, const CNTR& _defValue) const
    {
      CNTR _result;
      const auto& _child = get_child(_path,fromArray(_defValue));

      for (auto& _v : _child)
      {
        if (_v.second.empty() && get<std::string>(_v.first).empty())\
          _result.push_back(_v.second.template get_value<typename CNTR::value_type>());
      }
      return _result;
    }

    void load(const std::string _filename)
    {
      boost::property_tree::json_parser::read_json(_filename,static_cast<boost::property_tree::ptree&>(*this));
    }

    void save(const std::string& _filename) const
    {
      boost::property_tree::json_parser::write_json(_filename,boost::property_tree::ptree(*this));
    }
    
    void print(std::ostream& _os) const
    {
      print(_os,0,*this);
    }

    friend std::ostream& operator<<(std::ostream& _os, Config& _config)
    {
      _config.print(_os,0,_config);
      return _os;
    }

    Config& merge(const Config& _cfg) 
    {
      return merge(_cfg,"");
    }

    Config& merge(const Config& _cfg, path_type const& _path)
    {
      using boost::property_tree::ptree; 
      traverse(ptree(_cfg),[&](
            const ptree& _parent,
            const path_type& _childPath,
            const ptree& _child)
      {
        this->put(_path / _childPath, _child.data());
      });
      return *this; 
    }

  private:
    template<typename CNTR>
    boost::property_tree::ptree fromArray(const CNTR& _cntr) const
    {
      boost::property_tree::ptree _children;
      for (auto& _v : _cntr)
      {
        boost::property_tree::ptree _child;
        _child.put("",_v);
        _children.push_back(std::make_pair("",_child));
      }
      return _children;
    }

    template<typename T>
    void traverse_recursive(
        const boost::property_tree::ptree& _parent, 
        const boost::property_tree::ptree::path_type& _childPath, 
        const boost::property_tree::ptree& _child, T _method)
    {
      using boost::property_tree::ptree;
  
      _method(_parent, _childPath, _child);
      for(auto it= _child.begin();it != _child.end(); ++it) 
      {
        auto _curPath = _childPath / ptree::path_type(it->first);
        traverse_recursive(_parent, _curPath, it->second, _method);
      }
    }

    template<typename T>
    void traverse(const boost::property_tree::ptree &parent, T method)
    {
      traverse_recursive(parent, "", parent, method);
    }

    void print(std::ostream& _os, const int _depth, 
               const boost::property_tree::ptree& _tree) const
    {  
      using std::string;
      for (const auto& _v : _tree.get_child("") )
      {
        const auto& _subtree = _v.second;
        auto _nodeStr = _tree.get<string>(_v.first);
      
        // print current node  
        _os << string("").assign(_depth*2,' ') << "  " << _v.first;  
        if (!_subtree.empty())
        { 
          _os  << ": " << std::endl;  
        }
        if ( !_nodeStr.empty() )
        {
          _os << "=\"" << _tree.get<string>(_v.first) << "\"" << std::endl;  
        } else
        {
          if (_subtree.empty())
            /// We have an array element here
            _os << "\"" << _v.second.get_value<string>() << "\"" << std::endl;
        }

        // recursive go down the hierarchy  
        print(_os,_depth+1,_subtree);  
      }
    }
  };  

  struct ModifyableObject 
  {
    typedef Config config_type;
    typedef typename config_type::path_type path_type;

    ModifyableObject(const path_type& _pathName) : pathName_(_pathName) {}

    TBD_PROPERTY_MODIFY_FLAG()

    TBD_PROPERTY_REF_RO(path_type,pathName)
	};

  struct ConfigurableObject
  {
    typedef Config config_type;
    typedef typename config_type::path_type path_type;

    ConfigurableObject(const path_type& _pathName, config_type* _config = nullptr) : 
      pathName_(_pathName), config_(_config) {} 
      
    TBD_PROPERTY_REF_RO(path_type,pathName)
		TBD_PROPERTY(config_type*,config)
  };

#define TBD_PROPERTY_CFG_PATHNAME(name) \
	inline path_type name##_path() const { return pathName() / path_type(std::string(#name)); }

#define TBD_PROPERTY_CFG(type,name,def_value) \
public:\
	type name() const \
	{ \
    return config() ? config()->get(name##_path(),def_value) : def_value;\
  }\
  bool name(const type& _value) \
  {\
    if (!config()) return false;\
    \
    if (_value == name()) return false;\
    config()->put(name##_path(),_value);\
    return true;\
  }\
  TBD_PROPERTY_CFG_PATHNAME(name)\
	inline type name##_def() const { return def_value; }\
private:

#define TBD_PROPERTY_CFG_ARRAY_BASE(type,name,...)\
  TBD_PROPERTY_CFG_PATHNAME(name)\
  inline type name##_def() const\
  {\
    type _result = { __VA_ARGS__ }; \
    return _result;\
  }

#define TBD_PROPERTY_CFG_ARRAY(type,name,...)\
public:\
  type name() const \
  {\
    if (!config()) return name##_def();\
    return config()->get_array(name##_path(),name##_def());\
  }\
  bool name(const type& _cntr) \
  {\
    if (!config()) return false;\
    if (name() == _cntr) return false;\
    config()->put_array(name##_path(),_cntr);\
    return true;\
  }\
  TBD_PROPERTY_CFG_ARRAY_BASE(type,name,__VA_ARGS__)\
private:

#define TBD_PROPERTY_MODIFY_CFG_WRITE_ONLY(type,name,def_value)\
  public:  void (name)(const config_type* _config){\
              type _##name = _config ? _config->get(name##_path(),def_value) : def_value;\
              name(_##name); }\
  TBD_PROPERTY_CFG_PATHNAME(name)\
  private:

#define TBD_PROPERTY_MODIFY_CFG(type,name,def_value) \
  TBD_PROPERTY_MODIFY(type,name)\
  TBD_PROPERTY_MODIFY_CFG_WRITE_ONLY(type,name,def_value)\

#define TBD_PROPERTY_MODIFY_CFG_REF(type,name,def_value) \
  TBD_PROPERTY_REF_MODIFY(type,name)\
  TBD_PROPERTY_MODIFY_CFG_WRITE_ONLY(type,name,def_value)

#define TBD_PROPERTY_MODIFY_CFG_ARRAY(type,name,...)\
  public:  void (name)(const config_type* _config){\
              type _##name = _config ? _config->get_array(name##_path(),name##_def()) : name##_def();\
              name(_##name); }\
  TBD_PROPERTY_CFG_ARRAY_BASE(type,name,__VA_ARGS__)\
  TBD_PROPERTY_REF_MODIFY(type,name)\
  
}


