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

#include "Krystal.Web/Css/TypedOM/Transform/CSSTransformValue.hpp"
#include "Krystal.Core/Geometry/TransformationMatrix.hpp"
#include "Krystal.Core/Maths/Matrix.hpp"
#include "Krystal.Core/Maths/Transform.hpp"
#include "Krystal.Web/Css/Geometry/DOMMatrix.hpp"
#include "Krystal.Web/Css/TypedOM/Transform/CSSMatrixComponent.hpp"
#include "Krystal.Web/Css/TypedOM/Transform/CSSPerspective.hpp"
#include "Krystal.Web/Css/TypedOM/Transform/CSSRotate.hpp"
#include "Krystal.Web/Css/TypedOM/Transform/CSSScale.hpp"
#include "Krystal.Web/Css/TypedOM/Transform/CSSSkew.hpp"
#include "Krystal.Web/Css/TypedOM/Transform/CSSSkewX.hpp"
#include "Krystal.Web/Css/TypedOM/Transform/CSSSkewY.hpp"
#include "Krystal.Web/Css/TypedOM/Transform/CSSTranslate.hpp"
#include "Krystal.Web/Css/Values/FunctionValue.hpp"
#include "Krystal.Web/Css/Values/TransformListValue.hpp"
#include "Krystal.Web/Css/Values/ValueListBuilder.hpp"
#include "Krystal.Web/Style/Values/Transforms/Perspective.hpp"
#include "Krystal.Web/Style/Values/Transforms/Rotate.hpp"
#include "Krystal.Web/Style/Values/Transforms/Scale.hpp"
#include "Krystal.Web/Style/Values/Transforms/Translate.hpp"

#include <algorithm>

namespace krys::web
{
  namespace
  {
    template <typename T>
    KRYS_NODISCARD ExceptionOr<Ref<CSSTransformComponent>>
      UpcastComponent(ExceptionOr<Ref<T>> component) noexcept
    {
      if (component.HasException())
      {
        return component.ReleaseException();
      }
      return Ref<CSSTransformComponent>(component.ReleaseValue());
    }

    /// @brief Reifies a transform function as the transform component of its kind.
    /// @see https://drafts.css-houdini.org/css-typed-om/#reify-a-transform-list
    KRYS_NODISCARD ExceptionOr<Ref<CSSTransformComponent>>
      CreateTransformComponent(const css::FunctionValue &function, Document &document) noexcept
    {
      switch (function.Name())
      {
        case css::ValueId::TranslateX:
        case css::ValueId::TranslateY:
        case css::ValueId::TranslateZ:
        case css::ValueId::Translate:
        case css::ValueId::Translate3d:
        {
          return UpcastComponent(CSSTranslate::Create(function, document));
        }
        case css::ValueId::ScaleX:
        case css::ValueId::ScaleY:
        case css::ValueId::ScaleZ:
        case css::ValueId::Scale:
        case css::ValueId::Scale3d:
        {
          return UpcastComponent(CSSScale::Create(function, document));
        }
        case css::ValueId::RotateX:
        case css::ValueId::RotateY:
        case css::ValueId::RotateZ:
        case css::ValueId::Rotate:
        case css::ValueId::Rotate3d:
        {
          return UpcastComponent(CSSRotate::Create(function, document));
        }
        case css::ValueId::SkewX:
        {
          return UpcastComponent(CSSSkewX::Create(function, document));
        }
        case css::ValueId::SkewY:
        {
          return UpcastComponent(CSSSkewY::Create(function, document));
        }
        case css::ValueId::Skew:
        {
          return UpcastComponent(CSSSkew::Create(function, document));
        }
        case css::ValueId::Perspective:
        {
          return UpcastComponent(CSSPerspective::Create(function, document));
        }
        case css::ValueId::Matrix:
        case css::ValueId::Matrix3d:
        {
          return UpcastComponent(CSSMatrixComponent::Create(function, document));
        }
        default:
        {
          return Exception {ExceptionCode::TypeError};
        }
      }
    }
  }

  ExceptionOr<Ref<CSSTransformValue>> CSSTransformValue::Create(const css::TransformListValue &list,
                                                                Document &document) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#reify-a-transform-list
    List<Ref<CSSTransformComponent>> components;
    for (const auto &value : list)
    {
      const auto *function = DynamicDowncast<css::FunctionValue>(value);
      if (!function)
      {
        return Exception {ExceptionCode::TypeError};
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

  ExceptionOr<Ref<CSSTransformValue>>
    CSSTransformValue::Create(List<Ref<CSSTransformComponent>> transforms) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-csstransformvalue-csstransformvalue
    // 1. If transforms is empty, throw a TypeError.
    if (transforms.empty())
    {
      return Exception {ExceptionCode::TypeError};
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

  ExceptionOr<Ref<CSSTransformComponent>>
    CSSTransformValue::SetItem(size_t index, Ref<CSSTransformComponent> component) noexcept
  {
    if (index > _components.size())
    {
      return Exception {ExceptionCode::RangeError};
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

  ExceptionOr<Ref<DOMMatrix>> CSSTransformValue::ToMatrix() const noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-csstransformvalue-tomatrix
    // 1. Let matrix be a new DOMMatrix, initialized to the identity matrix, with its is2D internal slot set
    //    to true.
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

  RefPtr<css::Value> CSSTransformValue::ToCSSValue() const noexcept
  {
    css::ValueListBuilder builder;
    for (const auto &component : _components)
    {
      if (auto value = component->ToCSSValue())
      {
        builder.push_back(Ref<css::Value>(krys::move(value)));
      }
    }
    return css::TransformListValue::Create(krys::move(builder));
  }
}
