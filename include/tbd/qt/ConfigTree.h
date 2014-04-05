#pragma once

#include <QTreeView>
#include <QStandardItemModel>
#include <memory>
#include <tbd/config.h>

namespace tbd
{
  namespace qt
  {
    class ConfigTree : public QTreeView
    {
      Q_OBJECT
    public:
      ConfigTree(QWidget* _parent = nullptr);
      ConfigTree(Config const*, QWidget* _parent = nullptr);

      void setConfig(Config const* _config);

      TBD_PROPERTY_RO(Config const*,config)
    private:
      void generate(QStandardItem* _root, boost::property_tree::ptree const&);

      std::unique_ptr<QStandardItemModel> model_;
    };
  }
}

