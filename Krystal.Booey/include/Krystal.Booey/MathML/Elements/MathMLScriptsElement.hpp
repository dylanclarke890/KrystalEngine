/*
 * Copyright (C) 2016 Igalia S.L. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/MathML/Elements/MathMLRowElement.hpp"

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
