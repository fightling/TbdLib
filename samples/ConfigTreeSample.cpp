#include <QApplication>
#include <tbd/qt/ConfigTree.h>

int main(int ac, char* av[])
{
  /// Non-silent mode: Start gui
  QApplication _a(ac, av);

  tbd::Config _cfg("json.cfg");
  tbd::qt::ConfigTree _tree(&_cfg);

  _tree.show();

  return _a.exec();
}
