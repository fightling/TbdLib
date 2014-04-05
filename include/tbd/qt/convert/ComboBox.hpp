#pragma once

#include <boost/algorithm/string/split.hpp>
#include <tbd/qt/Converter.hpp>
#include <QComboBox>

namespace tbd
{
  namespace qt
  {
    namespace convert
    {
      class ComboBox : 
        public Converter,
        private Registrar<ComboBox>
      {
      public:
        TBD_CONVERTER("ComboBox")
        ComboBox(std::string const& _name, const parameters_type& _value) : 
          Converter(_name,_value) {}

        inline QWidget* makeWidget(ParameterForm* _form, const parameters_type& _p) const
        {
          QComboBox* _comboBox = new QComboBox();
          auto&& _stringItems = _p.get_optional<std::string>("items");
          auto&& _min = _p.get_optional<int>("min");
          auto&& _max = _p.get_optional<int>("max");
          if (_stringItems)
          {
            std::vector<std::string> _items;
            boost::algorithm::split(_items,_stringItems.get(),boost::is_any_of(","),boost::token_compress_on);
          
            for (auto& _item : _items)
            {
              _comboBox->addItem(QString().fromStdString(_item));
            }
          } else
          if (_min && _max)
          {
            for (int i = _min.get(); i <= _max.get(); ++i)
            {
              _comboBox->addItem(QString("%1").arg(i));
            }
          }  
          ParameterForm::connect(_comboBox, SIGNAL(currentIndexChanged(int)), _form, SLOT(update()));
          updateWidget(_comboBox);
          return _comboBox;
        }
        
        inline void updateWidget(QWidget* _widget) const
        {
          QComboBox* _cb = static_cast<QComboBox*>(_widget);
          _cb->setCurrentIndex(parameters().get<int>(""));
        }

        inline void toParameter(QWidget const* _widget, parameters_type& _p) const
        {
          if (!_widget) return;
          QComboBox const* _cb = static_cast<QComboBox const*>(_widget);
          _p.put(name(),_cb->currentIndex());
        }
      };
    }
  }
}


