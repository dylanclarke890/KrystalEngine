#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Document;
  class Element;

  /// @brief Determines whether the links of a document are visited, for `:visited` and the link styles.
  // FIXME: There's no visited link store (it comes with pages and history), so only links to the document
  // itself (an empty `href`) are visited, and the style invalidation of links whose state changes is missing.
  class VisitedLinkState final
  {
    KRYS_NON_COPY_MOVABLE(VisitedLinkState);

  private:
    WeakPtr<Document> _document;

  public:
    explicit VisitedLinkState(Document &document) noexcept;

    KRYS_NODISCARD InsideLink DetermineLinkState(const Element &element) const noexcept;

  private:
    KRYS_NODISCARD InsideLink DetermineLinkStateSlowCase(const Element &element) const noexcept;
  };
}
