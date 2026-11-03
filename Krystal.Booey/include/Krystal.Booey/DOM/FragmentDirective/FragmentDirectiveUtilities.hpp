#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::dom
{
  class ContainerNode;
  class Node;

  /// @brief A text directive, decoded: the text it matches (`StartText`, or from `StartText` to `EndText`),
  /// and the text that must come before and after it.
  /// @see https://wicg.github.io/scroll-to-text-fragment/#text-directive
  struct ParsedTextDirective
  {
    utf8_string Prefix;
    utf8_string StartText;
    utf8_string EndText;
    utf8_string Suffix;

    KRYS_NODISCARD bool operator==(const ParsedTextDirective &) const noexcept = default;
  };

  /// @brief What finding text directives' text shares.
  class FragmentDirectiveUtilities
  {
  public:
    /// @brief The nearest inclusive ancestor of @p node that's block-level, or the document.
    /// @see https://wicg.github.io/scroll-to-text-fragment/#nearest-block-ancestor
    KRYS_NODISCARD static ContainerNode &NearestBlockAncestor(Node &node) noexcept;
  };
}
