#include "Krystal.UI/Document.hpp"

namespace Krys::UI
{
  Document::Document() : _layoutConfig(ConfigCreate())
  {
    _body = CreateElement<Element>();
  }

  Document::~Document()
  {
    ConfigDestroy(_layoutConfig);
  }
}