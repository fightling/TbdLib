#include <QApplication>
#include <QGroupBox>
#include <tbd/qt/ParameterForm.h>
#include <tbd/qt/Converter.hpp>
#include <tbd/qt/convert.hpp>
#include <tbd/serialize.h>
#include <tbd/classregister.h>

class Person : public tbd::Serializer<Person>
{
  TBD_PARAMETER_LIST
  (
    (std::string) name,
    (int) age,
    (bool) married,
    (int) gender,
    (float) height,
    (int) years,
    (int) level
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

  void additionalParameters(tbd::Config& _config) const
  {
    using namespace tbd::parameter;
    make(*this,_config,"name",
    {
      {"max_length", "32"},
    });
    make(*this,_config,"age",
    {
      {"min", "0"},
      {"max", "99"}
    });
    make(*this,_config,"married");
    
    make(_config,"gender", 
      {    
        {"type", "list"},
        {"items", "male,female,intersex"}
      });
    
    make(_config,"level",
    {
      {"type", "slider"},
      {"min", "0"},
      {"max", "100"},
      {"step", "10"}
    });
  }

};

int main(int ac, char* av[])
{
  /// Non-silent mode: Start gui
  QApplication _a(ac, av);

  tbd::Config _addParams;
  Person _p("Bob",42);
  _p.additionalParameters(_addParams);
  std::cout << _addParams;

  for (auto& _class : tbd::qt::convert::Registry::classes())
    std::cout << _class.first << std::endl;

  QGroupBox _widget;
  _widget.setTitle("Test");
  _widget.show();
  _widget.setLayout(new tbd::qt::ParameterForm(&_p,&_widget));

  return _a.exec();
}
