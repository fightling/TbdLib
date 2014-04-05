#pragma once

#include <tbd/qt/Converter.hpp>
#include <QLineEdit>

namespace tbd
{
  namespace qt
  {
    namespace convert
    {
      class LineEdit : 
        public Converter,
        private Registrar<LineEdit>
      {
      public:
        TBD_CONVERTER("LineEdit")

        LineEdit(std::string const& _name, const parameters_type& _value) : 
          Converter(_name,_value) {}

        inline QWidget* makeWidget(ParameterForm* _form, const parameters_type&) const
        {
          QWidget* _lineEdit = new QLineEdit(QString().fromStdString(parameters().get<std::string>("")));
          ParameterForm::connect(_lineEdit, SIGNAL(editingFinished()), _form, SLOT(update()));
          return _lineEdit;
        }

        inline void updateWidget(QWidget* _widget) const
        {
          QLineEdit* _lineEdit = static_cast<QLineEdit*>(_widget);
          _lineEdit->setText(QString().fromStdString(parameters().get<std::string>("")));
        }

        inline void toParameter(QWidget const* _widget, parameters_type& _p) const
        {
          if (!_widget) return;
          QLineEdit const* _lineEdit = static_cast<QLineEdit const*>(_widget);
          _p.put(name(),_lineEdit->text().toStdString());
        }
      };
    }
  }
}
