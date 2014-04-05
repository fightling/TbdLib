#include <QApplication>
#include <tbd/qt/ColorButton.h>

int main(int ac, char* av[])
{
  /// Non-silent mode: Start gui
  QApplication _a(ac, av);

  tbd::qt::ColorButton _button;

  _button.setText("Test");
  _button.show();

  return _a.exec();
}
