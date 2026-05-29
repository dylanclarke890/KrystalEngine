#pragma once

#include "Krystal.HTML/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.HTML/HTML/ElementInternals.hpp"
#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  // TODO(impl): CONTENT-ATTRIBUTE-REFLECTION - USVString is also one of the possible types but we currently
  // don't support it.
  // TODO(impl): CONTENT-ATTRIBUTE-REFLECTION - FrozenArray<T>? is also one of the possible types but we
  // currently don't support it.
  template <typename T>
  concept IDLReflectionType = OneOf<DOMString, Maybe<DOMString>, bool, int32, uint32, double, DOMTokenList>;

  template <typename T>
  concept IDLReflectionTarget = OneOf<HTMLElement, ElementInternals>;

  /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#reflect
  class ContentAttributeReflection
  {
  public:
    template <IDLReflectionType TValue, IDLReflectionTarget Target>
    static TValue Reflect(Target &target, DOMStringAtom name) noexcept
    {
      if constexpr (SameType<TValue, DOMString>)
      {
        auto *element = GetElement(target);
        auto contentAttribute = GetContentAttribute(target, name);
      }
    }

    template <IDLReflectionType TValue, IDLReflectionTarget Target>
    static void Reflect(Target &target, DOMStringAtom name, DOMString &&value) noexcept
    {
      auto *element = GetElement(target);
    }

  private:
#pragma region HTMLElement Reflected Target

    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#get-the-element
    KRYS_NODISCARD static RawPtr<HTMLElement> GetElement(HTMLElement &element) noexcept
    {
      return &element;
    }

    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#get-the-content-attribute
    KRYS_NODISCARD static Maybe<DOMString> GetContentAttribute(HTMLElement &element,
                                                               DOMStringAtom name) noexcept
    {
      auto attribute = ElementAlgorithms::GetAttributeByNamespace(DOMStringAtom::Null(), name, element);
      if (attribute == nullptr)
      {
        return Null;
      }

      return attribute->Value();
    }

    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#set-the-content-attribute
    static void SetContentAttribute(HTMLElement &element, DOMStringAtom name, DOMString &&value) noexcept
    {
      ElementAlgorithms::SetAttributeValue(element, name, Krys::Move(value));
    }

    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#delete-the-content-attribute
    static void DeleteContentAttribute(HTMLElement &element, DOMStringAtom name) noexcept
    {
      ElementAlgorithms::RemoveAttributeByNamespace(DOMStringAtom::Null(), name, element);
    }

#pragma endregion

#pragma region ElementInternals Reflected Target

    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#get-the-element
    KRYS_NODISCARD static RawPtr<HTMLElement> GetElement(ElementInternals &internals) noexcept
    {
      return internals.TargetElement();
    }

    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#get-the-content-attribute
    KRYS_NODISCARD static Maybe<DOMString> GetContentAttribute(ElementInternals &internals,
                                                               DOMStringAtom name) noexcept
    {
      // TODO(impl): ELEMENT-INTERNALS
      return Null;
    }

    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#set-the-content-attribute
    static void SetContentAttribute(ElementInternals &internals, DOMStringAtom name,
                                    DOMString &&value) noexcept
    {
      // TODO(impl): ELEMENT-INTERNALS
    }

    /// @see https://html.spec.whatwg.org/multipage/common-dom-interfaces.html#delete-the-content-attribute
    static void DeleteContentAttribute(ElementInternals &internals, DOMStringAtom name) noexcept
    {
      // TODO(impl): ELEMENT-INTERNALS
    }

#pragma endregion
  };
}