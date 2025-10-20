#include "Krystal.UI/Document.hpp"

namespace Krys::UI
{
  Document::Document() : _layoutConfig(Layout::ConfigCreate())
  {
    _body = CreateElement<Element>();

    auto &body = GetBody();
    body.SetWidth(100.0_pct);
    body.SetHeight(100.0_pct);
    body.SetBorderColour(Gfx::Colours::Transparent);
    body.SetBorderWidth(Layout::UnitValue::Zero);
    body.SetWrap(Wrap::Wrap);
  }

  Document::~Document()
  {
    Layout::ConfigDestroy(_layoutConfig);
  }
}