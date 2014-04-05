#pragma once

#include <tbd/qt/Converter.hpp>
#include <QSpinBox>

namespace tbd
{
  namespace qt
  {
    namespace convert
    {
      class SpinBox : 
        public Converter,
        private Registrar<SpinBox>
      {
      public:
        TBD_CONVERTER("SpinBox")
        SpinBox(std::string const& _name, const parameters_type& _value) : 
          Converter(_name,_value) {}

        inline QWidget* makeWidget(ParameterForm* _form, const parameters_type& _p) const
        {
          QWidget* _spinBox = new QSpinBox();
          /// Set infinite range
          QSpinBox* _sp = static_cast<QSpinBox*>(_spinBox);
          
          auto&& _min = _p.get("min",std::numeric_limits<int>::min());
          auto&& _max = _p.get("max",std::numeric_limits<int>::max());
          _sp->setRange(_min,_max);
          ParameterForm::connect(_spinBox, SIGNAL(editingFinished()), _form, SLOT(update()));
          ParameterForm::connect(_spinBox, SIGNAL(valueChanged(int)), _form, SLOT(update()));
          updateWidget(_spinBox);
          return _spinBox;
        }
        
        inline void updateWidget(QWidget* _widget) const
        {
          QSpinBox* _sp = static_cast<QSpinBox*>(_widget);
          _sp->setValue(parameters().get<int>(""));
        }

        inline void toParameter(QWidget const* _widget, parameters_type& _p) const
        {
          if (!_widget) return;
          QSpinBox const* _spinBox = static_cast<QSpinBox const*>(_widget);
          _p.put(name(),_spinBox->value());
        }
      };
    }
  }
}

