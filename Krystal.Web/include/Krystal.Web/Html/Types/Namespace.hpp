#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Web/Dom/Types/DOMString.hpp"
#include "Krystal.Web/Infra/Namespaces.hpp"
#include "Krystal.Web/Infra/StringAlgorithms.hpp"

namespace krys::web::html
{
  enum class Namespace : uint8
  {
    Unknown,
    None,
    HTML,
    SVG,
    MathML,
    XLink,
    XML,
    XMLNS,
  };

  consteval Namespace MaxEnumValue(Namespace) noexcept
  {
    return Namespace::XMLNS;
  }

  KRYS_NODISCARD constexpr Namespace ParseNamespace(DOMStringView name) noexcept
  {
    if (name == DOMStringAtom::Empty())
    {
      return Namespace::None;
    }

    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(name, infra::namespaces::HTML))
    {
      return Namespace::HTML;
    }

    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(name, infra::namespaces::SVG))
    {
      return Namespace::SVG;
    }

    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(name, infra::namespaces::MathML))
    {
      return Namespace::MathML;
    }

    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(name, infra::namespaces::XML))
    {
      return Namespace::XML;
    }

    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(name, infra::namespaces::XMLNS))
    {
      return Namespace::XMLNS;
    }

    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(name, infra::namespaces::XLink))
    {
      return Namespace::XLink;
    }

    return Namespace::Unknown;
  }
}
