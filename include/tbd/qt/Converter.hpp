#pragma once

#include <memory>
#include <tbd/classregister.h>
#include <tbd/config.h>
#include <tbd/property.h>
#include <QWidget>

namespace tbd
{
  namespace qt
  {
    /// Interface to convert a widget to parameter and vice-versa
    class Converter
    {
    public:
      typedef boost::property_tree::ptree parameters_type;
      typedef std::string token_type;
      typedef QWidget widget_type;
      typedef std::unique_ptr<widget_type> widget_ptr_type;

      Converter(
            const token_type& _name, 
            const parameters_type& _parameters) :
          name_(_name),
          parameters_(_parameters) {}

        virtual QWidget* makeWidget(ParameterForm* _form, 
            const parameters_type& _p) const = 0;
        virtual void updateWidget(QWidget* _widget) const = 0;
        virtual void toParameter(QWidget const*,parameters_type& _p) const = 0;

        TBD_PROPERTY_REF_RO(token_type,name)
        TBD_PROPERTY_REF_RO(parameters_type,parameters)
      };

    namespace convert
    {
      typedef tbd::ClassRegistry<std::string,Converter,const std::string&,const Converter::parameters_type&> Registry;
      template<typename T>
      using Registrar = tbd::ClassRegistrar<Registry,T>;
    }
  }
}

#define TBD_CONVERTER(type_id)\
    static std::string typeId() { return type_id; }\
    inline virtual std::string getTypeId() const { return typeId(); }


