#include "Krystal.UI/Document.hpp"

namespace Krys::UI
{
  Document::Document() : _layoutConfig(Layout::ConfigCreate())
  {
    _body = CreateElement<Element>();
  }

  Document::~Document()
  {
    Layout::ConfigDestroy(_layoutConfig);
  }
}