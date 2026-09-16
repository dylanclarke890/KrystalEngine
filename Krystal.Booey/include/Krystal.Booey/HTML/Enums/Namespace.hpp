#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Core/Enum.hpp"

namespace krys::boo::html
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

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::html::Namespace, 8uz);

namespace krys::boo::html
{
  KRYS_NODISCARD constexpr Namespace ParseNamespace(dom::DOMStringView name) noexcept
  {
    if (name == dom::DOMStringAtom::Empty())
    {
      return Namespace::None;
    }

    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(name, infra::Namespaces::HTML))
    {
      return Namespace::HTML;
    }

    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(name, infra::Namespaces::SVG))
    {
      return Namespace::SVG;
    }

    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(name, infra::Namespaces::MathML))
    {
      return Namespace::MathML;
    }

    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(name, infra::Namespaces::XML))
    {
      return Namespace::XML;
    }

    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(name, infra::Namespaces::XMLNS))
    {
      return Namespace::XMLNS;
    }

    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(name, infra::Namespaces::XLink))
    {
      return Namespace::XLink;
    }

    return Namespace::Unknown;
  }
}