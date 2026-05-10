#pragma once

#include "Krystal.Lib/Pointers/WeakPtr.hpp"

namespace Krys::HTML
{
  class Document;

  class DOMImplementation
  {
    friend class Document;

  private:
    WeakRef<Document> _document;

  private:
    DOMImplementation(Document &document) noexcept : _document(CreateWeakRef(document))
    {
    }

  public:
  };
}