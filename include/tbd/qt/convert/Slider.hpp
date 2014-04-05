#pragma once

#include <tbd/qt/Converter.hpp>
#include <QSlider>

namespace tbd
{
  namespace qt
  {
    namespace convert
    {
      class Slider : 
        public Converter,
        private Registrar<Slider>
      {
      public:
        TBD_CONVERTER("Slider")
        Slider(std::string const& _name, const parameters_type& _value) : 
          Converter(_name,_value) {}

        inline QWidget* makeWidget(ParameterForm* _form, const parameters_type& _p) const
        {
          /// Set infinite range
          QSlider * _slider = new QSlider(Qt::Horizontal); 
          auto&& _min = _p.get("min",0);
          auto&& _max = _p.get("max",9);
          auto&& _step = _p.get("step",1);
          _slider->setRange(_min,_max);
          _slider->setSingleStep(_step);
          ParameterForm::connect(_slider, SIGNAL(sliderMoved(int)), _form, SLOT(update()));
          ParameterForm::connect(_slider, SIGNAL(valueChanged(int)), _form, SLOT(update()));
          updateWidget(_slider);
          return _slider;
        }
        
        inline void updateWidget(QWidget* _widget) const
        {
          QSlider* _sl = static_cast<QSlider*>(_widget);
          _sl->setValue(parameters().get<int>(""));
        }

        inline void toParameter(QWidget const* _widget, parameters_type& _p) const
        {
          if (!_widget) return;
          QSlider const* _slider = static_cast<QSlider const*>(_widget);
          _p.put(name(),_slider->value());
        }
      };
    }
  }
}


