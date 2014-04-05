#include <tbd/qt/ColorButton.h>

#include <QColorDialog>
#include <sstream>

namespace tbd
{
  namespace qt
  {
    ColorButton::ColorButton(QWidget* _parent) :
      QPushButton(_parent),
      color_("#000000")
    {
      this->setFlat(true);
      ColorButton::connect(this, SIGNAL(clicked()), this, SLOT(click()));
    }

    ColorButton::ColorButton(const color_type& _color, QWidget* _parent) :
      QPushButton(_parent),
      color_(_color)
    {
      this->setFlat(true);
    }

    void ColorButton::color(const color_type& _color)
    {
      color_ = _color;
      this->setStyleSheet("* { background-color: "+color_.name()+"}");
    }

    void ColorButton::click()
    {
      this->setStyleSheet("");
      QColor _qcolor = QColorDialog::getColor(color_, this);
      if (_qcolor.isValid())
      {
        color(_qcolor);
      }
      else
      {
        color(color_);
      }
    }
  }
}
