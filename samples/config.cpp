/************************FreeBSD license header*****************************
 * Copyright (c) 2012, Wilston Oreo
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

#include <array>
#include <vector>
#include <iostream>

#include "tbd/config.h" 
#include "tbd/configurable.h" 

#include <boost/algorithm/string/split.hpp>

using namespace std;

/// An example MyCustomType which hold for parameters
struct MyCustomType
{
  template<typename T>
  MyCustomType(const T& s)
  {
    parse(s);
  }

  template<typename T>
  void parse(const T& s)
  {
    std::stringstream ss(s);
    ss >> r >> g >> b >> a;
  }

  friend bool operator==(const MyCustomType& lhs, const MyCustomType& rhs)
  {
    return 
      lhs.r == rhs.r &&
      lhs.g == rhs.g &&
      lhs.b == rhs.b &&
      lhs.a == rhs.a;
  }

  int r,g,b,a;
};


// Custom translator for MyCustomType (only supports std::string)
struct MyCustomTypeTranslator
{
    typedef std::string internal_type;
    typedef MyCustomType external_type;

    // Converts a string to MyCustomType
    boost::optional<external_type> get_value(const internal_type& str)
    {
        if (!str.empty())
        {
            return boost::optional<external_type>(external_type(str));
        }
        else
            return boost::optional<external_type>(boost::none);
    }

    // Converts a MyCustomType to string
    boost::optional<internal_type> put_value(const external_type& b)
    {
      std::stringstream ss;
      ss << b.r << " " << b.g << " " << b.b << " " << b.a;
      return boost::optional<internal_type>(ss.str());
    }
};

/*  Specialize translator_between so that it uses our custom translator for
    bool value types. Specialization must be in boost::property_tree
    namespace. */
namespace boost {
namespace property_tree {

template<typename Ch, typename Traits, typename Alloc> 
struct translator_between<std::basic_string< Ch, Traits, Alloc >, MyCustomType>
{
    typedef MyCustomTypeTranslator type;
};

} // namespace property_tree
} // namespace boost


namespace 
{
  TBD_CONFIG_PROPERTY(float,Bar1,bar1,0.4)
  TBD_CONFIG_PROPERTY(std::string,Bar2,bar2,"DefaultValue")
  TBD_CONFIG_PROPERTY(int,Bar3,bar3,10)
  TBD_CONFIG_PROPERTY_ARRAY(std::vector<int>,Bar4,bar4,1,2,3,4)

  TBD_CONFIG_PROPERTY(MyCustomType,Custom,custom,"255 200 100 255");

  TBD_PROPERTYSET(PropertySet1,Bar1,Bar2)
  TBD_PROPERTYSET(PropertySet2,Bar3,Bar4)
}

/// An example type which has two properties
class Foo : 
  public tbd::Configurable<Bar1,Bar2>
{
public:
	Foo() : tbd::Configurable<Bar1,Bar2>("Foo") {}
	
	void print()
	{
    cout << "Print #" << print_ << endl;
    cout << "bar1: " << bar1() << endl;
		cout << "bar2: " << bar2() << endl;
    cout << endl;

    print_++;
  }
  static int print_;
};


/// An example type which has three properties and one property set
class Bar :
  public tbd::Configurable<PropertySet1,Bar3,Bar4,Custom>
{
public:
  Bar() : tbd::Configurable<PropertySet1,Bar3,Bar4,Custom>("Bar") {}
};


/// This MyFunctor defines operations which will be executed for each property
struct MyFunctor
{
  template<typename NAME, typename T>
  void operator()(NAME _name, NAME _varName, T _value, T _def)
  {
    std::cout << _name << "::" << _varName 
              << ": Undefined Type" << std::endl;
  }

  template<typename NAME>
  void operator()(NAME _name, NAME _varName, float _value, float _def)
  {
    std::cout << _name << "::" << _varName  
              << "(has type 'float') = " << _value 
              << " " << _def << std::endl;
  }

  template<typename NAME>
  void operator()(NAME _name, NAME _varName, int _value, int _def)
  {
    std::cout << _name << "::" << _varName 
              << "(has type 'int') = " 
              << _value << " " << _def << std::endl;
  }

  template<typename NAME>
  void operator()(NAME _name, NAME _varName, std::string _value, std::string _def)
  {
    std::cout << _name << "::" << _varName 
              << "(has type 'std::string') = " 
              << _value << " " << _def << std::endl;
  }

  template<typename NAME>
  void operator()(NAME _name, NAME _varName, MyCustomType _value, MyCustomType _def)
  {
    std::cout << _name << "::" << _varName 
              << "(has type 'MyCustomType') = " 
              << _value.r << " " << _def.r << std::endl;
  }
};



int Foo::print_ = 0;

int main(int ac, char* av[])
{
	// Instantiate an object of class 'Foo'
	tbd::Config config;
	Foo test;

  test.bar1(10);
  test.bar2("Test1");
	test.print();

  test.save(config);
  std::cout << config;
	
  test.bar1(4);
  test.bar2("Test2");
  test.print();
  test.load(config);
	test.print();

  config.clear();
  Bar bar;
  bar.save(config);
  std::cout << config;

  MyFunctor _myFunctor;
  test.apply(_myFunctor);

  bar.apply(_myFunctor);
  
  /*

	// Change some values
	config.put(test.bar1_path(),"2.178");
	config.put(test.bar2_path(),"Yeah!");
  std::cout << (test.bar2("Yeah2") ? std::string("Value changed to " + test.bar2()) : 
                                      "Value didn't change.") << std::endl;
  std::cout << (test.bar2("Yeah2") ? std::string("Value changed to " + test.bar2()) : 
                                      "Value didn't change.") << std::endl;

  boost::property_tree::ptree children;
  std::vector<int> vec = { 1,2,3,4,5 };
  for (auto& i : vec)
  {
    boost::property_tree::ptree child;
    child.put("",i);
    children.push_back(std::make_pair("",child));
  }
  config.put_child("vec",children);
  
  for (auto& v : config.get_child("vec"))
  {
    if (v.second.empty() && config.get<string>(v.first).empty())
    {
      std::cout << v.second.get_value<int>() << ", ";
    }
  }
  cout << endl;
  
  children.clear();
  vec = { 2,3,4,5 };
  for (auto& i : vec)
  {
    boost::property_tree::ptree child;
    child.put("",i);
    children.push_back(std::make_pair("",child));
  }
  
  config.put_child("vec",children);


	// Print out changes values
	cout << "### After: " << endl;
	test.print();
	cout << "Config: " << endl;
	cout << config;

	// Read and write config
	cout << "### Read and write test with NEW_PARAM: " << endl;
	config.put("NEW_PARAM","FOO");
	config.save("sample.cfg");
	config.load("sample.cfg");
	cout << config;

  config.load("json.cfg");
  cout << config;
  config.save("json_test.cfg");
*/
	return 0;
}

