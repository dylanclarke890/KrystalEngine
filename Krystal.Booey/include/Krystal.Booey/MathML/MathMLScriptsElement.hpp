#pragma once

#include "Krystal.Booey/MathML/MathMLRowElement.hpp"

namespace krys::boo::mathml
{
  /// @brief The script elements: `<msub>`, `<msup>`, `<msubsup>` and `<mmultiscripts>`, and the base of the
  /// under and over elements.
  /// @see https://w3c.github.io/mathml-core/#script-and-limit-schemata
  class MathMLScriptsElement : public MathMLRowElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(MathMLScriptsElement);

  public:
    enum class ScriptType : uint8
    {
      Sub,
      Super,
      SubSup,
      Multiscripts,
      Under,
      Over,
      UnderOver,
    };

  private:
    const boo::mathml::MathMLScriptsElement::ScriptType _scriptType;

  public:
    MathMLScriptsElement(dom::Document &document, ScriptType scriptType) noexcept
        : MathMLScriptsElement(document, scriptType, MathMLElementInterface::Scripts)
    {
    }

    KRYS_NODISCARD boo::mathml::MathMLScriptsElement::ScriptType GetScriptType() const noexcept
    {
      return _scriptType;
    }

  protected:
    MathMLScriptsElement(dom::Document &document, ScriptType scriptType,
                         MathMLElementInterface interface) noexcept
        : MathMLRowElement(document, interface), _scriptType(scriptType)
    {
    }

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::mathml::MathMLScriptsElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::mathml::MathMLElement &element) noexcept
  {
    return element.Interface() == krys::boo::mathml::MathMLElementInterface::Scripts
           || element.Interface() == krys::boo::mathml::MathMLElementInterface::UnderOver;
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    auto *element = krys::DynamicDowncast<krys::boo::mathml::MathMLElement>(node);
    return element && IsType(*element);
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
