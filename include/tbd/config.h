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

#include <map>
#include <vector>
#include <string>
#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <tbd/property.h>

using namespace std;

namespace tbd {

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
	class Config
	{
		public:
			Config(const string& filename = string())
			{
				if (!filename.empty()) read(filename);
			}

			template <class T> inline T get(const string& key)
			{ 		
				return boost::lexical_cast<T>(parameters[key]); 
			}

			template <class T> inline void set(const string key, const T& value)
			{ 
				parameters[key] = boost::lexical_cast<string>(value); 
			}

			void set(string& key, string& value)
			{
				parameters[key] = value;
			}

			void read(const string& filename)
			{
				ifstream is;
				try
				{
					is.open(filename.c_str());

					while (is.good())
					{
						char line[1024];
						is.getline(line,sizeof(line));
						string l(line); boost::trim(l);
						vector<string> splitVec;
						boost::split( splitVec, l, boost::is_any_of("="), boost::token_compress_on);
						if (splitVec.size()<2) continue;

						string key   = splitVec[0]; boost::trim(key); boost::to_upper(key);
						string value = splitVec[1]; 

						// Remove comment 
						size_t commentPos = value.find("#");
						if (commentPos != string::npos) value = value.substr(0,commentPos-1);
						boost::trim(value);

						if (key.length()==0) continue;
						if (key[0] < 65 || key[0] > 90) continue; // Key must begin with a letter!

						parameters.insert(pair<string,string>(key,value));
						set(key,value);
					}
				} catch (std::exception e)
				{
					cerr << "Exception reading user configuration file '" << filename << "' :" << e.what() << endl;
				}

				is.close();
			}

			void write(const string& filename)
			{
				ofstream file;
				try 
				{
					file.open(filename.c_str(), ofstream::trunc);
					file << *this;
				} catch (std::exception e)
				{
					cerr << "Exception writing user configuration file '" << filename << "' :" << e.what() << endl;
				}
				file.close();
			}

			bool exists(const string& key)
			{
				return parameters.count(key) != 0;
			}

			friend ostream& operator<<(ostream& os, Config& cfg)
			{
				map<string,string>::iterator it;
					for (it = cfg.parameters.begin(); it != cfg.parameters.end(); ++it)
						os << it->first << " = " << it->second << endl;
				return os;
			}

		private:
			map<string,string> parameters;
	};

	class ConfigurableObject {
		public:
			ConfigurableObject(Config* _config = NULL) : config_(_config) {} 
			TBD_PROPERTY(Config*,config);
	};

#define TBD_PROPERTY_CFG(type,name,param_name,def_value) \
public:\
	type (name)() const \
	{ \
		if (config()) { \
		if (config()->exists((param_name))) return config()->get<type>((param_name)); \
		else config()->set<type>(string(param_name),def_value); } \
		return def_value; }\
	\
	inline string name##_param() const { return string(param_name); } \
	inline type name##_def() const { return def_value; }\
private:

}
