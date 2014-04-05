#pragma once

#include <QPushButton>
#include <QColor>
#include <tbd/property.h>

namespace tbd
{
  namespace qt
  {
    class ColorButton : public QPushButton
    {
      Q_OBJECT
    public:
      typedef QColor color_type;

      ColorButton(QWidget* _parent = nullptr);
      ColorButton(const color_type& _color, QWidget* _parent = nullptr);

      void color(const color_type& _color);

      TBD_PROPERTY_REF_RO(color_type,color)

    public slots:
      void click();
    };
  }
}
