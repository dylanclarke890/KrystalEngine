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

#include "Krystal.Booey/CSS/TypedOM/Transform/CSSTransformValue.hpp"
#include "Krystal.Booey/CSS/DOMMatrix/DOMMatrix.hpp"
#include "Krystal.Booey/CSS/TypedOM/Transform/CSSMatrixComponent.hpp"
#include "Krystal.Booey/CSS/TypedOM/Transform/CSSPerspective.hpp"
#include "Krystal.Booey/CSS/TypedOM/Transform/CSSRotate.hpp"
#include "Krystal.Booey/CSS/TypedOM/Transform/CSSScale.hpp"
#include "Krystal.Booey/CSS/TypedOM/Transform/CSSSkew.hpp"
#include "Krystal.Booey/CSS/TypedOM/Transform/CSSSkewX.hpp"
#include "Krystal.Booey/CSS/TypedOM/Transform/CSSSkewY.hpp"
#include "Krystal.Booey/CSS/TypedOM/Transform/CSSTranslate.hpp"
#include "Krystal.Booey/CSS/Values/FunctionValue.hpp"
#include "Krystal.Booey/CSS/Values/TransformListValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueListBuilder.hpp"

#include <algorithm>

namespace krys::boo::css
{
  namespace
  {
    template <typename T>
    KRYS_NODISCARD dom::ExceptionOr<Ref<CSSTransformComponent>>
      UpcastComponent(dom::ExceptionOr<Ref<T>> &&component) noexcept
    {
      if (component.HasException())
      {
        return component.ReleaseException();
      }
      return Ref<CSSTransformComponent>(component.ReleaseValue());
    }

    /// @brief Reifies a transform function as the transform component of its kind.
    /// @see https://drafts.css-houdini.org/css-typed-om/#reify-a-transform-list
    KRYS_NODISCARD dom::ExceptionOr<Ref<CSSTransformComponent>>
      CreateTransformComponent(const FunctionValue &function, dom::Document &document) noexcept
    {
      switch (function.Name())
      {
        case ValueId::TranslateX:
        case ValueId::TranslateY:
        case ValueId::TranslateZ:
        case ValueId::Translate:
        case ValueId::Translate3d:
        {
          return UpcastComponent(CSSTranslate::Create(function, document));
        }
        case ValueId::ScaleX:
        case ValueId::ScaleY:
        case ValueId::ScaleZ:
        case ValueId::Scale:
        case ValueId::Scale3d:
        {
          return UpcastComponent(CSSScale::Create(function, document));
        }
        case ValueId::RotateX:
        case ValueId::RotateY:
        case ValueId::RotateZ:
        case ValueId::Rotate:
        case ValueId::Rotate3d:
        {
          return UpcastComponent(CSSRotate::Create(function, document));
        }
        case ValueId::SkewX:
        {
          return UpcastComponent(CSSSkewX::Create(function, document));
        }
        case ValueId::SkewY:
        {
          return UpcastComponent(CSSSkewY::Create(function, document));
        }
        case ValueId::Skew:
        {
          return UpcastComponent(CSSSkew::Create(function, document));
        }
        case ValueId::Perspective:
        {
          return UpcastComponent(CSSPerspective::Create(function, document));
        }
        case ValueId::Matrix:
        case ValueId::Matrix3d:
        {
          return UpcastComponent(CSSMatrixComponent::Create(function, document));
        }
        default:
        {
          return dom::Exception {dom::ExceptionCode::TypeError};
        }
      }
    }
  }

  dom::ExceptionOr<Ref<CSSTransformValue>> CSSTransformValue::Create(const TransformListValue &list,
                                                                     dom::Document &document) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#reify-a-transform-list
    List<Ref<CSSTransformComponent>> components;
    for (const auto &value : list)
    {
      const auto *function = DynamicDowncast<FunctionValue>(value);
      if (!function)
      {
        return dom::Exception {dom::ExceptionCode::TypeError};
      }
      auto component = CreateTransformComponent(*function, document);
      if (component.HasException())
      {
        return component.ReleaseException();
      }
      components.push_back(component.ReleaseValue());
    }
    return AdoptRef(*new CSSTransformValue(krys::move(components)));
  }

  dom::ExceptionOr<Ref<CSSTransformValue>>
    CSSTransformValue::Create(List<Ref<CSSTransformComponent>> transforms) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-csstransformvalue-csstransformvalue
    // 1. If transforms is empty, throw a TypeError.
    if (transforms.empty())
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }
    // 2. Let this be a new CSSTransformValue whose values to iterate over is transforms.
    // 3. Return this.
    return AdoptRef(*new CSSTransformValue(krys::move(transforms)));
  }

  CSSTransformValue::CSSTransformValue(List<Ref<CSSTransformComponent>> transforms) noexcept
      : _components(krys::move(transforms))
  {
  }

  CSSTransformValue::~CSSTransformValue() noexcept = default;

  RefPtr<CSSTransformComponent> CSSTransformValue::Item(size_t index) const noexcept
  {
    if (index >= _components.size())
    {
      return nullptr;
    }
    return _components[index];
  }

  dom::ExceptionOr<Ref<CSSTransformComponent>>
    CSSTransformValue::SetItem(size_t index, Ref<CSSTransformComponent> component) noexcept
  {
    if (index > _components.size())
    {
      return dom::Exception {dom::ExceptionCode::RangeError};
    }
    if (index == _components.size())
    {
      _components.push_back(krys::move(component));
    }
    else
    {
      _components[index] = krys::move(component);
    }
    return _components[index];
  }

  bool CSSTransformValue::Is2D() const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-csstransformvalue-is2d
    return std::ranges::all_of(_components, [](const auto &component) { return component->Is2D(); });
  }

  dom::ExceptionOr<Ref<DOMMatrix>> CSSTransformValue::ToMatrix() const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-csstransformvalue-tomatrix
    // 1. Let matrix be a new DOMMatrix, initialized to the identity matrix, with its is2D internal slot set
    // to
    //    true.
    TransformationMatrix matrix;
    auto is2D = DOMMatrixReadOnly::Is2D::Yes;
    // 2. For each transform in this's values to iterate over:
    for (const auto &component : _components)
    {
      // 2.1. Let transformMatrix be the result of calling transform's toMatrix() method.
      auto componentMatrix = component->ToMatrix();
      if (componentMatrix.HasException())
      {
        return componentMatrix.ReleaseException();
      }
      const auto &transformMatrix = componentMatrix.Value();
      // 2.2. Set matrix to the result of multiplying matrix and the matrix represented by transformMatrix.
      if (!transformMatrix->GetIs2D())
      {
        is2D = DOMMatrixReadOnly::Is2D::No;
      }
      matrix.Multiply(transformMatrix->GetTransformationMatrix());
    }
    // 3. Return matrix.
    return DOMMatrix::Create(matrix, is2D);
  }

  void CSSTransformValue::Serialise(StringBuilder &builder, EnumSet<SerialisationArguments>) const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#serialize-a-csstransformvalue
    bool first = true;
    for (const auto &component : _components)
    {
      if (!first)
      {
        builder.Append(u8' ');
      }
      first = false;
      component->Serialise(builder);
    }
  }

  RefPtr<Value> CSSTransformValue::ToCSSValue() const noexcept
  {
    ValueListBuilder builder;
    for (const auto &component : _components)
    {
      if (auto value = component->ToCSSValue())
      {
        builder.push_back(Ref<Value>(krys::move(value)));
      }
    }
    return TransformListValue::Create(krys::move(builder));
  }
}
