#pragma once

#include <tbd/qt/Converter.hpp>
#include <QDoubleSpinBox>

namespace tbd
{
  namespace qt
  {
    namespace convert
    {
      class DoubleSpinBox : 
        public Converter,
        private Registrar<DoubleSpinBox>
      {
      public:
        TBD_CONVERTER("DoubleSpinBox")
        DoubleSpinBox(std::string const& _name, const parameters_type& _value) : 
          Converter(_name,_value) {}

        inline QWidget* makeWidget(ParameterForm* _form, const parameters_type& _p) const
        {
          QDoubleSpinBox * _sp = new QDoubleSpinBox();
          
          /// Set infinite range
          auto&& _min = _p.get("min",std::numeric_limits<float>::min());
          auto&& _max = _p.get("max",std::numeric_limits<float>::max());
          auto&& _step = _p.get("step",float(0.1));
          _sp->setRange(_min,_max);
          _sp->setSingleStep(_step);
          ParameterForm::connect(_sp, SIGNAL(editingFinished()), _form, SLOT(update()));
          ParameterForm::connect(_sp, SIGNAL(valueChanged(double)), _form, SLOT(update()));
          updateWidget(_sp);
          return _sp;
        }
        
        inline void updateWidget(QWidget* _widget) const
        {
          QDoubleSpinBox* _sp = static_cast<QDoubleSpinBox*>(_widget);
          _sp->setValue(parameters().get<float>(""));
        }

        inline void toParameter(QWidget const* _widget, parameters_type& _p) const
        {
          if (!_widget) return;
          QDoubleSpinBox const* _spinBox = static_cast<QDoubleSpinBox const*>(_widget);
          _p.put(name(),_spinBox->value());
        }
      };
    }
  }
}


