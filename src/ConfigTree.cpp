#include <tbd/qt/ConfigTree.h>

#include <QHeaderView>

namespace tbd
{
  namespace qt
  {
    ConfigTree::ConfigTree(QWidget* _parent) : 
      QTreeView(_parent), 
      config_(nullptr),
      model_(new QStandardItemModel(this))
    {
      setModel(model_.get());
    }
    
    ConfigTree::ConfigTree(Config const* _config, QWidget* _parent) : 
      QTreeView(_parent), 
      model_(new QStandardItemModel(this))
    {
      setModel(model_.get());
      setConfig(_config);
    }

    void ConfigTree::setConfig(Config const* _config)
    {
      config_=_config;
      model_->clear();
      model_->setColumnCount(2);
      model_->setHeaderData(0,Qt::Horizontal,"Key");
      model_->setHeaderData(1,Qt::Horizontal,"Value");
      this->header()->setResizeMode(1, QHeaderView::Stretch);
      this->header()->setCascadingSectionResizes(true);
      this->header()->setResizeMode(QHeaderView::ResizeToContents);
      
      generate(model_->invisibleRootItem(),*_config);
      expandAll();
    }

    void ConfigTree::generate(QStandardItem* _root, boost::property_tree::ptree const& _tree)
    {
      for (const auto& _v : _tree.get_child("") )
      {
        const auto& _subtree = _v.second;
        auto _nodeStr = _tree.get<std::string>(_v.first);
        QList<QStandardItem*> _row;
        auto _item = new QStandardItem(QString().fromStdString(_v.first));
        _item->setEditable(false);
        _row << _item;
        
        if (_subtree.empty())
        {
          _item = new QStandardItem(QString().fromStdString(
                _nodeStr.empty() ? 
                _v.second.get_value<std::string>() :
                _nodeStr));
          _item->setEditable(false);
          _row << _item;
        }
        _root->appendRow(_row);
        if (!_subtree.empty())
          generate(_item,_subtree);
      }
    }
  }
}
