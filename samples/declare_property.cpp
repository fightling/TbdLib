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

#include "tbd/property.h" 

using namespace std;

#define NUM_ELEMENTS 10

class Foo
{
	// A simple bool value
	TBD_PROPERTY(bool,bar1);
	
	// A string
	TBD_PROPERTY(string,bar2);
	
	// Returns the reference to the member variable
	TBD_PROPERTY_REF(vector<int>,vecbar1);

	// Read-only member variable
	TBD_PROPERTY_RO(vector<int>,vecbar2);
	
public:
	void generate_vecbar2()
	{
		for (size_t i = 0; i < NUM_ELEMENTS; i++)
			vecbar2_.push_back(i);
	}
};


int main(int ac, char* av[])
{
	// Instantiate an object of class 'Foo'
	Foo test;

	// Set and output bar1 and bar2
	test.bar1(true); cout << "bar1: " << test.bar1() << endl;
	test.bar2("Test");  cout << "bar2: " << test.bar2() << endl;
	
	// Generate a vector of ints and give it to test.vecbar1
	vector<int> vecbar1(NUM_ELEMENTS);
	test.vecbar1(vecbar1);

	// Change some values
	test.vecbar1()[3] = 42;
	cout << "vecbar1: ";
	for (int i = 0; i < test.vecbar1().size(); i++)  // Output
		cout << test.vecbar1()[i] << ", ";
	cout << endl;

	// Generate pointer variables
	test.generate_vecbar2();
	vector<int> vecbar2 = test.vecbar2();
	cout << "vecbar2: ";
	for (size_t i = 0; i < vecbar2.size(); i++) 
		cout << vecbar2[i] << ", ";
	cout << endl;

	return 0;
}

