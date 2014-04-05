#pragma once

#include <tbd/qt/convert.hpp>
#include <QCheckBox>

namespace tbd
{
  namespace qt
  {
    namespace convert
    {
      class CheckBox : 
        public Converter,
        private Registrar<CheckBox>
      {
      public:
        TBD_CONVERTER("CheckBox")
        CheckBox(std::string const& _name, const parameters_type& _value) : 
          Converter(_name,_value) {}

        inline QWidget* makeWidget(ParameterForm* _form, const parameters_type&) const
        {
          QWidget* _checkBox = new QCheckBox();
          ParameterForm::connect(_checkBox, SIGNAL(clicked()), _form, SLOT(update()));
          updateWidget(_checkBox);
          return _checkBox;
        }
        
        inline void updateWidget(QWidget* _widget) const
        {
          QCheckBox* _chk = static_cast<QCheckBox*>(_widget);
          _chk->setChecked(parameters().get<bool>("",false));  
        }

        inline void toParameter(QWidget const* _widget, parameters_type& _p) const
        { 
          if (!_widget) return;
          QCheckBox const* _spinBox = static_cast<QCheckBox const*>(_widget);
          _p.put(name(),_spinBox->isChecked());
        }
      };
    }
  }
}


