/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER “AS IS” AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/TypedOM/Transform/CSSMatrixComponent.hpp"
#include "Krystal.Booey/CSS/DOMMatrix/DOMMatrix.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSUnitValue.hpp"
#include "Krystal.Booey/CSS/Values/FunctionValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueListBuilder.hpp"

namespace krys::boo::css
{
  Ref<CSSMatrixComponent> CSSMatrixComponent::Create(const DOMMatrixReadOnly &matrix,
                                                     CSSMatrixComponentOptions options) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-cssmatrixcomponent-cssmatrixcomponent
    // 1. Let this be a new CSSMatrixComponent object with its matrix internal slot set to matrix.
    // 2. If options was passed and has a is2D field, set this's is2D internal slot to the value of that
    // field.
    // 3. Otherwise, set this's is2D internal slot to the value of matrix's is2D attribute.
    // 4. Return this.
    return AdoptRef(*new CSSMatrixComponent(matrix, options.Is2D.value_or(matrix.GetIs2D())));
  }

  dom::ExceptionOr<Ref<CSSMatrixComponent>> CSSMatrixComponent::Create(const FunctionValue &function,
                                                                       dom::Document &document) noexcept
  {
    auto reified = ReifyArguments(function, document);
    if (reified.HasException())
    {
      return reified.ReleaseException();
    }
    List<double> entries;
    for (const auto &argument : reified.ReleaseValue())
    {
      const auto *unitValue = DynamicDowncast<CSSUnitValue>(*argument);
      if (!unitValue)
      {
        return dom::Exception {dom::ExceptionCode::TypeError};
      }
      entries.push_back(unitValue->Value());
    }

    const auto &e = entries;
    if (function.Name() == ValueId::Matrix && e.size() == 6)
    {
      auto matrix = DOMMatrixReadOnly::Create(TransformationMatrix(e[0], e[1], e[2], e[3], e[4], e[5]),
                                              DOMMatrixReadOnly::Is2D::Yes);
      return Create(*matrix);
    }
    if (function.Name() == ValueId::Matrix3d && e.size() == 16)
    {
      auto matrix =
        DOMMatrixReadOnly::Create(TransformationMatrix(e[0], e[1], e[2], e[3], e[4], e[5], e[6], e[7], e[8],
                                                       e[9], e[10], e[11], e[12], e[13], e[14], e[15]),
                                  DOMMatrixReadOnly::Is2D::No);
      return Create(*matrix);
    }
    return dom::Exception {dom::ExceptionCode::TypeError};
  }

  CSSMatrixComponent::CSSMatrixComponent(const DOMMatrixReadOnly &matrix, bool is2D) noexcept
      : CSSTransformComponent(is2D), _matrix(matrix.CloneAsDOMMatrix())
  {
  }

  CSSMatrixComponent::~CSSMatrixComponent() noexcept = default;

  void CSSMatrixComponent::SetMatrix(Ref<DOMMatrix> matrix) noexcept
  {
    _matrix = krys::move(matrix);
  }

  void CSSMatrixComponent::Serialise(StringBuilder &builder) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#serialize-a-cssmatrixcomponent
    // The matrix's serialisation, as a 2D one if this is 2D.
    const auto &m = *_matrix;
    if (Is2D())
    {
      builder.Append(u8"matrix(", m.A(), u8", ", m.B(), u8", ", m.C(), u8", ", m.D(), u8", ", m.E(), u8", ",
                     m.F(), u8')');
      return;
    }
    builder.Append(u8"matrix3d(", m.M11(), u8", ", m.M12(), u8", ", m.M13(), u8", ", m.M14(), u8", ", m.M21(),
                   u8", ", m.M22(), u8", ", m.M23(), u8", ", m.M24(), u8", ");
    builder.Append(m.M31(), u8", ", m.M32(), u8", ", m.M33(), u8", ", m.M34(), u8", ", m.M41(), u8", ",
                   m.M42(), u8", ", m.M43(), u8", ", m.M44(), u8')');
  }

  dom::ExceptionOr<Ref<DOMMatrix>> CSSMatrixComponent::ToMatrix() const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-csstransformcomponent-tomatrix
    if (!Is2D())
    {
      return _matrix->CloneAsDOMMatrix();
    }
    // Flattened to 2D.
    const auto &m = *_matrix;
    return DOMMatrix::Create(TransformationMatrix(m.A(), m.B(), m.C(), m.D(), m.E(), m.F()),
                             DOMMatrixReadOnly::Is2D::Yes);
  }

  RefPtr<Value> CSSMatrixComponent::ToCSSValue() const noexcept
  {
    const auto &m = *_matrix;
    ValueListBuilder arguments;
    if (Is2D())
    {
      for (double entry : {m.A(), m.B(), m.C(), m.D(), m.E(), m.F()})
      {
        arguments.push_back(PrimitiveValue::Create(entry));
      }
      return FunctionValue::Create(ValueId::Matrix, krys::move(arguments));
    }
    for (double entry : {m.M11(), m.M12(), m.M13(), m.M14(), m.M21(), m.M22(), m.M23(), m.M24(), m.M31(),
                         m.M32(), m.M33(), m.M34(), m.M41(), m.M42(), m.M43(), m.M44()})
    {
      arguments.push_back(PrimitiveValue::Create(entry));
    }
    return FunctionValue::Create(ValueId::Matrix3d, krys::move(arguments));
  }
}
