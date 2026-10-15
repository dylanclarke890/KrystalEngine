#pragma once

#include "Krystal.Booey/Graphics/Transforms/TransformOperation.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Functions/TransformFunctionBase.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Functions/TransformFunctionWrapper.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // Any <transform-function>.
  // https://www.w3.org/TR/css-transforms-1/#typedef-transform-function
  struct TransformFunction : TransformFunctionWrapper<TransformFunctionBase>
  {
    using TransformFunctionWrapper<TransformFunctionBase>::TransformFunctionWrapper;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<TransformFunction>
  {
    KRYS_NODISCARD TransformFunction operator()(BuilderState &, const css::Value &) noexcept;
  };
  template <>
  struct CSSValueCreation<TransformFunction>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const TransformFunction &) noexcept;
  };
  template <>
  struct CSSValueCreation<TransformationMatrix>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const TransformationMatrix &) noexcept;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<TransformFunction>
  {
    void operator()(css::CSSOMString &, const css::SerialisationContext &, const RenderStyle &,
                    const TransformFunction &) noexcept;
  };
  template <>
  struct Serialiser<TransformationMatrix>
  {
    void operator()(css::CSSOMString &, const css::SerialisationContext &, const RenderStyle &,
                    const TransformationMatrix &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<TransformFunction>
  {
    KRYS_NODISCARD TransformFunction Blend(const TransformFunction &, const TransformFunction &,
                                           const BlendingContext &) noexcept;
  };

  // MARK: - Platform

  template <>
  struct ToPlatform<TransformFunction>
  {
    KRYS_NODISCARD Ref<TransformOperation> operator()(const TransformFunction &, const FloatSize &) noexcept;
  };
}
