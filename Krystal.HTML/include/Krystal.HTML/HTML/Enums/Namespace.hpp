#pragma once

#include "Krystal.HTML/Infra/Namespaces.hpp"
#include "Krystal.HTML/Infra/StringAlgorithms.hpp"
#include "Krystal.HTML/DOM/Types/DOMString.hpp"
#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
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
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::Namespace, 8uz);

namespace Krys::HTML
{
  KRYS_NODISCARD constexpr Namespace ParseNamespace(DOMStringView name) noexcept
  {
    if (name == DOMStringAtom::Empty())
    {
      return Namespace::None;
    }

    if (StringAlgorithms::ASCIICaseInsensitiveMatch(name, Namespaces::HTML.View()))
    {
      return Namespace::HTML;
    }

    if (StringAlgorithms::ASCIICaseInsensitiveMatch(name, Namespaces::SVG.View()))
    {
      return Namespace::SVG;
    }

    if (StringAlgorithms::ASCIICaseInsensitiveMatch(name, Namespaces::MathML.View()))
    {
      return Namespace::MathML;
    }

    if (StringAlgorithms::ASCIICaseInsensitiveMatch(name, Namespaces::XML.View()))
    {
      return Namespace::XML;
    }

    if (StringAlgorithms::ASCIICaseInsensitiveMatch(name, Namespaces::XMLNS.View()))
    {
      return Namespace::XMLNS;
    }

    if (StringAlgorithms::ASCIICaseInsensitiveMatch(name, Namespaces::XLink.View()))
    {
      return Namespace::XLink;
    }

    return Namespace::Unknown;
  }
}