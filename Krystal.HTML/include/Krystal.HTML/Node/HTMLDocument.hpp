#pragma once

#include "Krystal.HTML/Node/Document.hpp"

namespace Krys::HTML
{
  class HTMLDocument : public Document
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLDocument);
  };
}