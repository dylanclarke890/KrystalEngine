#pragma once

#include "Krystal.Booey/DOM/FragmentDirectiveUtilities.hpp"
#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  class Document;

  /// @brief Finds the text text directives indicate in a document.
  class FragmentDirectiveRangeFinder
  {
  public:
    /// @brief The range of the first text in @p document that @p parsedTextDirective matches, if any.
    /// @see https://wicg.github.io/scroll-to-text-fragment/#find-a-range-from-a-text-directive
    KRYS_NODISCARD static Maybe<SimpleRange>
      FindRangeFromTextDirective(const ParsedTextDirective &parsedTextDirective, Document &document) noexcept;

    /// @brief The ranges @p parsedTextDirectives match in @p document, leaving out those that match nothing.
    KRYS_NODISCARD static List<SimpleRange>
      FindRangesFromTextDirectives(const List<ParsedTextDirective> &parsedTextDirectives,
                                   Document &document) noexcept;
  };
}
