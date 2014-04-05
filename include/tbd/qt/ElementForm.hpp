#pragma once

#include <QObject>
#include <QLabel>
#include <tomo/tool/ChainInterface.hpp>
#include "convert.hpp"

namespace tomo
{
  class GLWidget;

  template<typename ELEMENT, typename CONVERTER>
  class ElementForm
  {
  public:
    typedef QWidget widget_type;
    typedef GLWidget viewer_type;
    typedef ELEMENT element_type;
    typedef CONVERTER converter_type;
    typedef std::pair<std::unique_ptr<widget_type>,tbd::ParameterRule> widgetpair_type;
    typedef std::vector<widgetpair_type> widgetlist_type;

    ElementForm(
        element_type* _element, 
        viewer_type* _viewer = nullptr,
        const std::string& _title = std::string()) : 
      element_(_element),
      viewer_(_viewer),
      title_(_title)
    {}

    virtual ~ElementForm() {}
    
    void clear()
    {
      labels_.clear();
      widgets_.clear();
    }

    virtual void generate() = 0;

    void update()
    {
      base::Element::tokenmap_type _tokenMap;
      for (auto& _widget : widgets_)
      {
        if (_widget.first->isHidden()) continue;
        auto&& _token = converter_type().toToken(_widget.first.get(),_widget.second);
        if (!_token) continue;
        _tokenMap.insert(tbd::splitToken(_token.get()));
      }
      element()->parse(_tokenMap);
      if (viewer()) viewer()->update();
    }

    void deleteHidden()  
    {
      std::remove_if(labels_.begin(),labels_.end(),[this](const std::unique_ptr<QLabel>& _label)
      {
        return _label->isHidden();
      });
      std::remove_if(widgets_.begin(),widgets_.end(),[this](const widgetpair_type& _pair)
      {
        return _pair.first->isHidden();
      });
    }

    template<typename FORM, typename PARAMETER_RULE>
    std::pair<QLabel*,widget_type*> addWidget(FORM* _form, const PARAMETER_RULE& _rule)
    {
      typedef std::pair<QLabel*,widget_type*> label_widget_type;

      std::unique_ptr<QWidget> _widget(converter_type().toWidget(_form,_rule));
      if (!_widget) return label_widget_type(nullptr,nullptr); 
      labels_.emplace_back(new QLabel(QString().fromStdString(_rule.name())));
      
      widgets_.emplace_back(std::move(_widget),_rule);
      return label_widget_type(
        labels_.back().get(),
        widgets_.back().first.get());
    }

    TBD_PROPERTY_RO(element_type*,element)
    TBD_PROPERTY(viewer_type*,viewer)
    TBD_PROPERTY_REF(std::string,title)
    TBD_PROPERTY_REF(widgetlist_type,widgets)
    TBD_PROPERTY_REF(std::vector<std::unique_ptr<QLabel>>,labels)
  private:
  };
}

