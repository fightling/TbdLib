#include <tbd/simple_serialization.h>

#include <iostream>
#include <tbd/config.h>



class Person : public tbd::Serializer<Person>
{
  TBD_PARAMETER_LIST
  (
    (std::string) name,
    (int) age
  )
public:
  typedef tbd::Serializer<Person> serializer_type;

  Person() : serializer_type(*this),
    name_("Alice"),
    age_(23) {}

  Person(const std::string& _name, int _age) :
    serializer_type(*this),
    name_(_name),
    age_(_age) {}

  void setName(std::string const& _name) 
  {
    name_ = _name;
  }
};
namespace tbd
{
  template<>
  struct TypeInfo<Person>
  {
    typedef char const* token_type;
    token_type operator()()
    {
      return "P";
    }
  };
}

class Couple : public tbd::Serializer<Couple>
{
  TBD_PARAMETER_LIST
  (
    (Person) a,
    (Person) b,
    (bool) married
  )
public:
  typedef tbd::Serializer<Couple> serializer_type;

  Couple() : serializer_type(*this),
    married_(true) {}

  Person& a() { return a_; }
  Person& b() { return b_; }
};





void personTest()
{
  Person p("Tom", 82);
  //print_fields(p);

  tbd::Config _cfg;
  p.save(tbd::ConfigPath("test"),_cfg);
  std::cout << _cfg;

  _cfg.put("test.name","Bob");
  _cfg.put("test.age","42");
  std::cout << _cfg;

  Couple _c;
  _c.a().setName("Romeo");
  _c.b().setName("Julia");
  _c.save("couple",_cfg);
  std::cout << _cfg;

  p.load(tbd::ConfigPath("test"),_cfg);

  auto _parameterList = _c.typeInfo();
  for (auto& _parameter : _parameterList)
    std::cout << _parameter.first << ":" << _parameter.second << std::endl;

  std::cout << _parameterList[std::string("married").c_str()] << std::endl;
}

#include <tbd/serialize_unique_ptr.h>


class Interface : public virtual tbd::SerializationInterface
{
public:
  virtual std::string getTypeId() const = 0;
  virtual std::string print() const = 0;
};

typedef tbd::ClassRegistry<std::string,Interface> Registry;

template<typename T>
using Registrar = tbd::ClassRegistrar<Registry,T>;

class Foo : 
  public Interface,
  public tbd::Serializer<Foo>,
  public Registrar<Foo>
{
  TBD_PARAMETER_LIST
  (
    (int) foo1,
    (int) foo2
  )
public:
  static std::string typeId() { return "Foo"; }
  std::string getTypeId() const { return typeId(); }
  
  Foo() : 
    tbd::Serializer<Foo>(*this),
    foo1_(42),
    foo2_(42) {}

  std::string print() const 
  {
    return std::to_string(foo1_) + " foo " + std::to_string(foo2_);
  }
};

class Bar : 
  public Interface,
  public tbd::Serializer<Bar>,
  public Registrar<Bar>
{
  TBD_PARAMETER_LIST
  (
    (int) bar1,
    (int) bar2
  )
public:
  static std::string typeId() { return "Bar"; }
  std::string getTypeId() const { return typeId(); }

  Bar() : 
    tbd::Serializer<Bar>(*this),
    bar1_(0),
    bar2_(0) {}

  std::string print() const 
  {
    return std::to_string(bar1_) + " bar " + std::to_string(bar2_);
  }
};

class FooBar : 
  public tbd::Serializer<FooBar>
{
  TBD_PARAMETER_LIST
  (
    (std::unique_ptr<Interface>) foobar,
    (std::vector<std::unique_ptr<Interface>>) foobars
  )
public:
  FooBar() : 
    tbd::Serializer<FooBar>(*this),
    foobar_(nullptr) {}
};


void unique_ptr_test()
{
  std::cout << "Num classes: " << Registry::classes().size() << std::endl;
  for (auto _class : Registry::classes())
  {
    std::cout << _class.first << std::endl;
  }

  using tbd::Config;
  using tbd::ConfigPath;
  Config _cfg;
  _cfg.put(ConfigPath("foobars") / "number",0);
  _cfg.put(ConfigPath("foobar") / "typeid","Bar");
  _cfg.put(ConfigPath("foobar") / "Bar" / "bar1",1);
  _cfg.put(ConfigPath("foobar") / "Bar" / "bar2",2);

  FooBar _foobar;
  _foobar.load(ConfigPath(),_cfg);  
  std::cout << _foobar.foobar()->print();
  std::cout << _cfg;
  _cfg.clear(); 
  _cfg.put(ConfigPath("foobars") / "number",1);
  _cfg.put(ConfigPath("foobar") / "typeid","Bar");
  _cfg.put(ConfigPath("foobar") / "Bar" / "bar1",1);
  _cfg.put(ConfigPath("foobar") / "Bar" / "bar2",2);
  _cfg.put(ConfigPath("foobars") / "0" / "typeid","Foo");
  _cfg.put(ConfigPath("foobars") / "0" / "Foo" / "foo1",3);
  _cfg.put(ConfigPath("foobars") / "0" / "Foo" / "foo2",31);
  _foobar.load(ConfigPath(),_cfg);  
  std::cout << _foobar.foobars()[0]->print();
  
  std::cout << _cfg;
  _cfg.save("unique_ptr_test.cfg");

}



int main(int argc, char** argv)
{
//  personTest();
  unique_ptr_test();
//  configurableTest();
//  subobjectTest();

  return 0;
}

