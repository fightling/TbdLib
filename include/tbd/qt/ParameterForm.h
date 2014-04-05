#pragma once

#include <map>
#include <memory>
#include <QObject>
#include <QFormLayout>
#include <QLabel>
#include <tbd/property.h>

namespace tbd
{
  struct SerializationInterface;

  namespace qt
  {
    class Converter;

    class ParameterForm : public QFormLayout
    {
      Q_OBJECT
    public:
      typedef SerializationInterface element_type;
      typedef QWidget widget_type;
      typedef QFormLayout inherited_type;
      typedef Converter converter_type;
      typedef std::map<std::string,std::string> typeid_delegate_map;

      ParameterForm( 
          element_type* _element,
          QWidget* _parent = nullptr);
      
      ParameterForm( 
          element_type* _element,
          typeid_delegate_map const& _typeIdDelegate,
          QWidget* _parent = nullptr);
      virtual ~ParameterForm();

      void clear();
      
      inline static typeid_delegate_map const& standardTypeIdDelegates()
      {
        static typeid_delegate_map _typeIdDelegate;
        if (_typeIdDelegate.empty())
        {
          _typeIdDelegate = {
            { "int", "SpinBox", },
            { "bool", "CheckBox", },
            { "float", "DoubleSpinBox", },
            { "double", "DoubleSpinBox", },
            { "std::string", "LineEdit", },
            { "list", "ComboBox", },
            { "slider", "Slider" }
          };
        }
        return _typeIdDelegate;
      }
 
      TBD_PROPERTY_RO(element_type*,element)
      TBD_PROPERTY_REF(typeid_delegate_map,typeIdDelegate)
    signals:
      void valueChanged();

    public slots:
      void update();
    private:
      //std::pair<QLabel*,widget_type*> addWidget(FORM* _form, const PARAMETER_RULE& _rule);
      typedef std::map<std::string,std::unique_ptr<widget_type>> widgets_type;


      void generate();

      std::map<std::string,std::unique_ptr<QLabel>> labels_;
      widgets_type widgets_;
      std::map<std::string,std::unique_ptr<converter_type>> converters_;
    };
  }
}

