#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  /// @brief Various flags used for optimizations in the Document class.
  enum class DocumentFlags : uint16
  {
    None = 0,

    /// @brief Indicates the type of the document is 'xml'.
    /// @see https://dom.spec.whatwg.org/#concept-document-type
    IsXMLDocument = 1 << 1,

    /// @brief Indicates the type of the document is 'html'.
    /// @see https://dom.spec.whatwg.org/#concept-document-type
    IsHTMLDocument = 1 << 2,

    /// @brief Indicates the type of the document is 'xhtml'.
    IsXHTMLDocument = 1 << 3,

    /// @brief Indicates the type of the document is 'svg'.
    IsSVGDocument = 1 << 4,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(Krys::HTML::DocumentFlags, 5uz);