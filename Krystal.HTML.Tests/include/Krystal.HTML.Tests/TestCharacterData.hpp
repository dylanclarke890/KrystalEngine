#pragma once

#include "Krystal.HTML/DOM/Text.hpp"

namespace Krys::Tests
{
  using namespace Krys::HTML;

  class TestCharacterData : public Text
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(TestCharacterData);

  public:
    TestCharacterData(Document &document, DOMString &&value = u8"") noexcept
        : Text(document, Krys::Move(value))
    {
    }
  };
}