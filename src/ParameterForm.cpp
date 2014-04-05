#include <tbd/qt/ParameterForm.h>
#include <tbd/qt/Converter.hpp>
#include <tbd/boolean.h>
#include <tbd/serialize.h>

#include <set>

namespace tbd
{
  namespace qt
  {
    ParameterForm::ParameterForm(
      element_type* _element,
      QWidget* _parent) :
      QFormLayout(_parent),
      element_(_element)
    {
      typeIdDelegate_ = standardTypeIdDelegates();
      setVerticalSpacing(1);
      generate();
    }
    
    ParameterForm::ParameterForm( 
          element_type* _element,
          typeid_delegate_map const& _typeIdDelegate,
          QWidget* _parent) : 
      QFormLayout(_parent),
      element_(_element),
      typeIdDelegate_(_typeIdDelegate)
    {
      setVerticalSpacing(1);
      generate();
    }

    ParameterForm::~ParameterForm() {}

    void ParameterForm::clear()
    {
      labels_.clear();
      widgets_.clear();
      converters_.clear();
    }

    void ParameterForm::generate()
    {
      if (!element()) return;
      clear();

      tbd::Config _config;
      element()->save("",_config);
      for (auto& _child : _config)
      {
        std::string _name(_child.first);
        converters_[_name].reset(nullptr);
        widgets_[_name].reset(nullptr);
        labels_[_name].reset(nullptr);
      }
      update();
    }

    void ParameterForm::update()
    {
      std::cout << "Updated!!!" << std::endl;
     
      tbd::Config _addParams;
      tbd::Config _config;
      element()->save("",_config);
      element()->additionalParameters(_addParams);
 
      tbd::Config _parameters;
      for (auto& _child : _addParams)
      {
        ConfigPath _name(_child.first);
        std::string _nameStr = _name.dump();
        
        if (!converters_.count(_nameStr) || !widgets_.count(_nameStr)) continue;
        if (!_addParams.exists(_name / "type")) continue; 

        auto&& _labelName = _child.second.get("label",_nameStr);

        std::string _typeId = _child.second.get<std::string>("type");
        if (typeIdDelegate_.count(_typeId))
          _typeId = typeIdDelegate_[_typeId];

        auto& _conv = converters_[_nameStr];
        if (!_conv)
        {
          _conv = std::move(convert::Registry::create(_typeId,_nameStr,_config.get_child(_name)));
          if (!_conv) continue;
          
          auto _widget = _conv->makeWidget(this,_addParams.get_child(_name)); 
          if (!_widget) continue;
          widgets_[_nameStr].reset(_widget);
          labels_[_nameStr].reset(new QLabel(QString().fromStdString(_labelName)));
          this->addRow(labels_[_nameStr].get(),_widget);
        } else
        {
          _conv->toParameter(widgets_[_nameStr].get(),_parameters);
        }
      }

      if (element()->load("",_parameters))
      {
        emit valueChanged();
      }

      _addParams.clear();
      element()->additionalParameters(_addParams);

      std::set<std::string> _paramNames, _usedNames;
      for (auto& _child : _config)
      {
        std::string _name(_child.first);
        _paramNames.insert(_name);
      }
      
      for (auto& _child : _addParams)
      {
        std::string _name(_child.first);
        _usedNames.insert(_name);
      }

      bool _reload = false;
      using tbd::boolean::operator-;
      auto&& _names = _paramNames - _usedNames;
      for (auto& _name : _paramNames)
      {
        if (labels_[_name]) labels_[_name]->show();
        if (widgets_[_name]) widgets_[_name]->show();
      }
      if (!_names.empty())
      {
        _reload =true;
        
        for (auto& _name : _names)
        {
          if (labels_[_name]) labels_[_name]->hide();
          if (widgets_[_name]) widgets_[_name]->hide();
        }
      }
    }
  }
}
