#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::html
{
  template <DerivedFrom<HTMLElement> TElement>
  struct EnumeratedAttributeDefinition
  {
  };
}