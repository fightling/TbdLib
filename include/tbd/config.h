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
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "property.h"


namespace tbd 
{
	/** @brief 		The Parameter class represents a parameter for the user configuration
	 *  @details	The key is stored as an Id
	 *  			The value is always stored as string
	 *  			possible INTEGER data type string formats:
	 * 					only DECIMAL: -11244 (signed), 12133(unsigned)
	 *
	 * 				possible FLOAT data type string formats:
	 * 					only DECIMAL: 365.454 or -45.23 or 6E-12 or 6e-12 or -6e-12 or 6E+12 etc ...
	 *
	 * 				Object is not responsible for validating formats. 0 is returned if conversion from string fails
	 */
	struct Config : boost::property_tree::ptree
	{
		Config(const std::string& filename = std::string())
		{
			if (!filename.empty()) load(filename);
		}

    void load(const std::string& _filename)
    {
      boost::property_tree::json_parser::read_json(_filename,*dynamic_cast<boost::property_tree::ptree*>(this));
    }

    void save(const std::string& _filename) const
    {
      boost::property_tree::json_parser::write_json(_filename,boost::property_tree::ptree(*this));
    }

    friend std::ostream& operator<<(std::ostream& _os, Config& _config)
    {
      _config.print(_os,0,_config);
      return _os;
    }

  private:
    void print(std::ostream& _os, const int _depth, 
               const boost::property_tree::ptree& _tree) 
    {  
      using std::string;
      for (auto& _v : _tree.get_child("") )
      {
        auto& _subtree = _v.second;
        auto _nodeStr = _tree.get<string>(_v.first);
      
        // print current node  
        _os << string("").assign(_depth*2,' ') << "  " << _v.first;  
        if (!_subtree.empty()) _os  << ": " << std::endl;  

        if ( !_nodeStr.empty() )
        {
          _os << "=\"" << _tree.get<string>(_v.first) << "\"" << std::endl;  
        }
        // recursive go down the hierarchy  
        print(_os,_depth+1,_subtree);  
      }
    }
  };  

	struct ConfigurableObject 
  {
		ConfigurableObject(const std::string& _objName, Config* _config = nullptr) : 
      objName_(_objName), config_(_config) {} 
      
    TBD_PROPERTY_RO(std::string,objName)
		TBD_PROPERTY(Config*,config)
	};

#define TBD_PROPERTY_CFG(type,name,def_value) \
public:\
	type name() const \
	{ \
    return config() ? config()->get(name##_path(),def_value) : def_value;\
  }\
  bool name(const type& _value) \
  {\
    if (config())\
    {\
      if (_value == config()->get(name##_path(),def_value)) return false;\
      config()->put(name##_path(),_value);\
      return true;\
    }\
    return _value != def_value;\
  }\
  \
	inline std::string name##_path() const { return objName() + "." + std::string(#name); }\
	inline type name##_def() const { return def_value; }\
private:
  
}
