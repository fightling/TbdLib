#include <tbd/serialize.h>
#include <iostream>
#include <tbd/config.h>

struct Person :
    tbd::Serializer<Person>
{
  typedef tbd::Serializer<Person> serializer_type;

  Person() : serializer_type(*this),
    name_("Alice"),
    age_(23) {}

  Person(const std::string& _name, int _age) :
    serializer_type(*this),
    name_(_name),
    age_(_age) {}

private:
  TBD_PARAMETER_LIST
  (
    (std::string) name,
    (int) age
  )
};

void personTest()
{
  Person p("Tom", 82);
  //print_fields(p);
  std::cout << p << std::endl;

  std::istringstream _is("(name=John,age=42)");
  _is >> p;
  std::cout << p << std::endl;

  tbd::Config _cfg;
  p.save(tbd::ConfigPath("test"),_cfg);
  std::cout << _cfg;

  _cfg.put("test.name","Bob");
  std::cout << _cfg;

  p.load(tbd::ConfigPath("test"),_cfg);
  std::cout << p << std::endl;
  std::cout << Person() << std::endl;
}

template<typename T>
struct Handler
{
  Handler(T& _obj) :
    obj_(_obj) {}

  template<typename FIELD>
  void operator()(const FIELD& _f)
  {
    if (std::string(_f.name()) == "foo")
    {
      obj_.test();
    }
    else if (std::string(_f.name()) == "bar")
    {
      obj_.privateTest();
    }
  }

  T& obj_;
};

struct MyInterface
{
  virtual void fooBar() const = 0;
};

template<typename T>
class Abstract :
    public MyInterface,
    public tbd::Serializer<Abstract<T>,Handler>
{
  TBD_PARAMETER_LIST
  (
    (int) foo,
    (std::string) bar
  )
public:
  friend Handler<Abstract>;
  typedef tbd::Serializer<Abstract<T>,Handler> serializer_type;

  Abstract() :
    serializer_type(*this),
    foo_(23),
    bar_("lorem") {}

  Abstract(int _foo, const std::string& _bar) :
    serializer_type(*this),
    foo_(_foo),
    bar_(_bar) {}

  virtual T method() const = 0;

  void test()
  {
    std::cout << "Handler called test()" << foo() << " " << bar() << std::endl;
  }

private:
  void privateTest()
  {
    std::cout << "Reclaim your privacy: " << std::endl;
  }
};


struct Derived :
    tbd::SerializerWithBase<Derived,Abstract<int>>
{
  typedef Abstract<int> inherited_type;
  typedef tbd::SerializerWithBase<Derived,Abstract<int>> serializer_type;

  Derived() :
    serializer_type(*this),
    updated_(0) {}

  void fooBar() const
  {
    std::cout << this->foo() << std::endl;
  }
  int method() const
  {
    return 23;
  }

private:

  TBD_PARAMETER_LIST
  (
    (bool) updated
  )
};


void configurableTest()
{
  Derived _d;
  std::istringstream _is("(foo=42,bar=ipsum,updated=1)");
  _d.parse(_is);

  //_is >> _d;

  _d.print(std::cout);
  std::cout << std::endl;

  _d.print(std::cout); std::cout << std::endl;

  std::cout << "Get token map..." << std::endl;
  Derived::tokenmap_type _tokens;
  _d.tokenMap(_tokens);
  for (auto& _t : _tokens) 
    std::cout << _t.first << "=" << _t.second << std::endl;

  Derived::typed_tokenmap_type _typedTokens;
  _d.typedTokenMap(_typedTokens);
  for (auto& _t : _typedTokens) 
    std::cout << _t.second.second << " " << _t.first << " = " << _t.second.first << std::endl;

}


////////////////////////////////
// Subobject Test
#include <tbd/serialize_sub.h>

class A : public tbd::Serializer<A>
{
  TBD_PARAMETER_LIST
  (
    (int) a1,
    (int) a2
  )
public:

  A() : tbd::Serializer<A>(*this), a1_(10), a2_(20) {}

  static std::string typeId() { return "A"; }
};

class B : public tbd::Serializer<B>
{
  TBD_PARAMETER_LIST
  (
    (int) b1,
    (int) b2
  )
public:

  B() : tbd::Serializer<B>(*this), b1_(30), b2_(40) {}

  static std::string typeId() { return "B"; }
};

class C : 
  public tbd::SerializeSubobjects<C,A,B>
{
  TBD_PARAMETER_LIST
  (
    (int) c1,
    (int) c2
  )
public:

  C() : 
    //tbd::Reflect<C>(*this),
    tbd::SerializeSubobjects<C,A,B>(*this,a_,b_), c1_(50), c2_(60) {}

  static std::string typeId() { return "C"; }

private:
  A a_;
  B b_;
};

void subobjectTest()
{
  std::cout << "subobjectTest <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << std::endl;

  C c;
  c.print(std::cout);

  Derived::tokenmap_type _tokens;
  c.tokenMap(_tokens);
  for (auto& _t : _tokens) 
    std::cout << _t.first << "=" << _t.second << std::endl;

  Derived::typed_tokenmap_type _typedTokens;
 // c.typedTokenMap(_typedTokens);
  for (auto& _t : _typedTokens) 
    std::cout << _t.second.second << " " << _t.first << " = " << _t.second.first << std::endl;

  std::istringstream _is("(A.a1=100,B.b2=3000,c2=4711,b1=100)");
  c.parse(_is);
  c.print(std::cout);
}









int main(int argc, char** argv)
{
  personTest();
//  configurableTest();
//  subobjectTest();

  return 0;
}
