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

//#define TEST_THREADS //uncommment to test with threads

#include <vector>
#include <iostream>

#include "tbd/config.h" 

using namespace std;

#define NUM_ELEMENTS 10

class Foo : public tbd::ConfigurableObject
{
	// Preprocessor macros to declare read-only properties which are read from config
	TBD_PROPERTY_CFG(float,bar1,3.1415f)
	TBD_PROPERTY_CFG(string,bar2,"Test")
  TBD_PROPERTY_CFG_ARRAY(std::vector<int>,bar3,1,2,3)

public:
	Foo(tbd::Config* _config = NULL) : ConfigurableObject("Foo",_config) {}
	
	void print()
	{
		cout << bar1_path() << ": " << bar1() << ", default: " << bar1_def() << endl;
		cout << bar2_path() << ": " << bar2() << ", default: " << bar2_def() << endl;
	}
};

class Bar : public tbd::ModifyableObject
{
  TBD_PROPERTY_MODIFY_CFG(float,foo1,42)
  TBD_PROPERTY_MODIFY_CFG_REF(string,foo2,"Foo")
  TBD_PROPERTY_MODIFY_CFG_ARRAY(std::vector<int>,foo3,3,5,7)

public:
  Bar() : tbd::ModifyableObject("Bar") {}

};


int main(int ac, char* av[])
{
	// Instantiate an object of class 'Foo'
	tbd::Config config;
	Foo test(&config);

	// Print out the initial default values
	cout << "### Before: " << endl;
	test.print();
	cout << "Config: " << endl;
	cout << config;



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

	return 0;
}

