#pragma once

#ifndef STYLE_INTERPOLATION_GENERATED_INCLUDE_TRAP
  #error "Please do not include this file anywhere except from generated code."
#endif

#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleProperties+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleProperties+SettersInlines.hpp"
#include "Krystal.Booey/Style/InterpolationContext.hpp"
#include "Krystal.Booey/Style/InterpolationFunctions.hpp"
#include "Krystal.Booey/Style/InterpolationWrapperBase.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <algorithm>

namespace krys::boo::style::interpolation
{
  // MARK: - Base Wrappers

  template <typename T, typename TGetter = T>
  class WrapperWithGetter : public WrapperBase
  {
  private:
    TGetter (RenderStyleProperties::*_getter)() const noexcept;

  public:
    WrapperWithGetter(css::PropertyId property,
                      TGetter (RenderStyleProperties::*getter)() const noexcept) noexcept
        : WrapperBase(property), _getter(getter)
    {
    }

    KRYS_NODISCARD TGetter Value(const RenderStyle &style) const noexcept
    {
      return (style.*_getter)();
    }

    KRYS_NODISCARD bool Equals(const RenderStyle &a, const RenderStyle &b) const noexcept override
    {
      if (&a == &b)
      {
        return true;
      }
      return Value(a) == Value(b);
    }
  };

  template <typename T, typename TGetter = T, typename TSetter = T>
  class Wrapper : public WrapperWithGetter<T, TGetter>
  {
  protected:
    void (RenderStyleProperties::*_setter)(TSetter) noexcept;

  public:
    Wrapper(css::PropertyId property, TGetter (RenderStyleProperties::*getter)() const noexcept,
            void (RenderStyleProperties::*setter)(TSetter) noexcept) noexcept
        : WrapperWithGetter<T, TGetter>(property, getter), _setter(setter)
    {
    }

    void Interpolate(RenderStyle &destination, const RenderStyle &from, const RenderStyle &to,
                     const Context &context) const noexcept override
    {
      (destination.*_setter)(BlendFunc(this->Value(from), this->Value(to), context));
    }
  };

  // Deduction guide for getter/setters that return and take values.
  template <typename T, typename TGetterStyle, typename TSetterStyle>
  Wrapper(css::PropertyId, T (TGetterStyle::*getter)() const noexcept,
          void (TSetterStyle::*setter)(T) noexcept) -> Wrapper<T, T, T>;

  // Deduction guide for getter/setters that return const references and take r-value references.
  template <typename T, typename TGetterStyle, typename TSetterStyle>
  Wrapper(css::PropertyId, const T &(TGetterStyle::*getter)() const noexcept,
          void (TSetterStyle::*setter)(T &&) noexcept) -> Wrapper<T, const T &, T &&>;

  // MARK: - Typed Wrappers

  template <typename T, typename TGetter = T, typename TSetter = T>
  class StyleTypeWrapper : public WrapperBase
  {
  private:
    TGetter (RenderStyleProperties::*_getter)() const noexcept;
    void (RenderStyleProperties::*_setter)(TSetter) noexcept;

  public:
    StyleTypeWrapper(css::PropertyId property, TGetter (RenderStyleProperties::*getter)() const noexcept,
                     void (RenderStyleProperties::*setter)(TSetter) noexcept) noexcept
        : WrapperBase(property), _getter(getter), _setter(setter)
    {
    }

    KRYS_NODISCARD bool Equals(const RenderStyle &from, const RenderStyle &to) const noexcept override
    {
      if (&from == &to)
      {
        return true;
      }
      return style::EqualsForBlending(this->Value(from), this->Value(to), from, to);
    }

    KRYS_NODISCARD bool CanInterpolate(const RenderStyle &from, const RenderStyle &to,
                                       CompositeOperation operation) const noexcept override
    {
      return style::CanBlend(this->Value(from), this->Value(to), from, to, operation);
    }

    KRYS_NODISCARD bool
      RequiresInterpolationForAccumulativeIteration(const RenderStyle &from,
                                                    const RenderStyle &to) const noexcept override
    {
      return style::RequiresInterpolationForAccumulativeIteration(this->Value(from), this->Value(to), from,
                                                                  to);
    }

    void Interpolate(RenderStyle &destination, const RenderStyle &from, const RenderStyle &to,
                     const Context &context) const noexcept override
    {
      (destination.*_setter)(style::Blend(this->Value(from), this->Value(to), from, to, context));
    }

  private:
    KRYS_NODISCARD TGetter Value(const RenderStyle &style) const noexcept
    {
      return (style.*_getter)();
    }
  };

  // Deduction guide for getter/setters that return and take values.
  template <typename T, typename TGetterStyle, typename TSetterStyle>
  StyleTypeWrapper(css::PropertyId, T (TGetterStyle::*getter)() const noexcept,
                   void (TSetterStyle::*setter)(T) noexcept) -> StyleTypeWrapper<T, T, T>;

  // Deduction guide for getter/setters that return const references and take r-value references.
  template <typename T, typename TGetterStyle, typename TSetterStyle>
  StyleTypeWrapper(css::PropertyId, const T &(TGetterStyle::*getter)() const noexcept,
                   void (TSetterStyle::*setter)(T &&) noexcept) -> StyleTypeWrapper<T, const T &, T &&>;

  // Deduction guide for getter/setters that return values and take r-value references.
  template <typename T, typename TGetterStyle, typename TSetterStyle>
  StyleTypeWrapper(css::PropertyId, T (TGetterStyle::*getter)() const noexcept,
                   void (TSetterStyle::*setter)(T &&) noexcept) -> StyleTypeWrapper<T, T, T &&>;

  template <typename T>
  class VisitedAffectedStyleTypeWrapper : public WrapperBase
  {
  protected:
    StyleTypeWrapper<T, const T &, T &&> _wrapper;
    StyleTypeWrapper<T, const T &, T &&> _visitedWrapper;

  public:
    VisitedAffectedStyleTypeWrapper(css::PropertyId property,
                                    const T &(RenderStyleProperties::*getter)() const noexcept,
                                    void (RenderStyleProperties::*setter)(T &&) noexcept,
                                    const T &(RenderStyleProperties::*visitedGetter)() const noexcept,
                                    void (RenderStyleProperties::*visitedSetter)(T &&) noexcept) noexcept
        : WrapperBase(property), _wrapper(property, getter, setter),
          _visitedWrapper(property, visitedGetter, visitedSetter)
    {
    }

    KRYS_NODISCARD bool Equals(const RenderStyle &a, const RenderStyle &b) const noexcept override
    {
      return _wrapper.Equals(a, b) && _visitedWrapper.Equals(a, b);
    }

    KRYS_NODISCARD bool
      RequiresInterpolationForAccumulativeIteration(const RenderStyle &a,
                                                    const RenderStyle &b) const noexcept override
    {
      return _wrapper.RequiresInterpolationForAccumulativeIteration(a, b)
             && _visitedWrapper.RequiresInterpolationForAccumulativeIteration(a, b);
    }

    void Interpolate(RenderStyle &destination, const RenderStyle &from, const RenderStyle &to,
                     const Context &context) const noexcept override
    {
      _wrapper.Interpolate(destination, from, to, context);
      _visitedWrapper.Interpolate(destination, from, to, context);
    }
  };

  // Deduction guide for getter/setters that return const references and take r-value references.
  template <typename T, typename TGetterStyle, typename TSetterStyle, typename TVisitedGetterStyle,
            typename TVisitedSetterStyle>
  VisitedAffectedStyleTypeWrapper(css::PropertyId, const T &(TGetterStyle::*getter)() const noexcept,
                                  void (TSetterStyle::*setter)(T &&) noexcept,
                                  const T &(TVisitedGetterStyle::*visitedGetter)() const noexcept,
                                  void (TVisitedSetterStyle::*visitedSetter)(T &&) noexcept)
    -> VisitedAffectedStyleTypeWrapper<T>;

  // MARK: - Discrete Wrappers

  template <typename T, typename TGetter = T, typename TSetter = T>
  class DiscreteWrapper : public WrapperWithGetter<T, TGetter>
  {
  private:
    void (RenderStyleProperties::*_setter)(TSetter) noexcept;

  public:
    DiscreteWrapper(css::PropertyId property, TGetter (RenderStyleProperties::*getter)() const noexcept,
                    void (RenderStyleProperties::*setter)(TSetter) noexcept) noexcept
        : WrapperWithGetter<T, TGetter>(property, getter), _setter(setter)
    {
    }

    KRYS_NODISCARD bool CanInterpolate(const RenderStyle &, const RenderStyle &,
                                       CompositeOperation) const noexcept final
    {
      return false;
    }

    void Interpolate(RenderStyle &destination, const RenderStyle &from, const RenderStyle &to,
                     const Context &context) const noexcept override
    {
      krys_debug_assert(!context.Progress || context.Progress == 1.0);
      (destination.*_setter)(T {this->Value(context.Progress ? to : from)});
    }
  };

  // Deduction guide for getter/setters that return and take values.
  template <typename T, typename TGetterStyle, typename TSetterStyle>
  DiscreteWrapper(css::PropertyId, T (TGetterStyle::*getter)() const noexcept,
                  void (TSetterStyle::*setter)(T) noexcept) -> DiscreteWrapper<T, T, T>;

  // Deduction guide for getter/setters that return const references and take r-value references.
  template <typename T, typename TGetterStyle, typename TSetterStyle>
  DiscreteWrapper(css::PropertyId, const T &(TGetterStyle::*getter)() const noexcept,
                  void (TSetterStyle::*setter)(T &&) noexcept) -> DiscreteWrapper<T, const T &, T &&>;

  // Deduction guide for getter/setters that return values and take r-value references.
  template <typename T, typename TGetterStyle, typename TSetterStyle>
  DiscreteWrapper(css::PropertyId, T (TGetterStyle::*getter)() const noexcept,
                  void (TSetterStyle::*setter)(T &&) noexcept) -> DiscreteWrapper<T, T, T &&>;

  template <typename T>
  class NonNormalizedDiscreteWrapper final : public Wrapper<T>
  {
  public:
    NonNormalizedDiscreteWrapper(css::PropertyId property,
                                 T (RenderStyleProperties::*getter)() const noexcept,
                                 void (RenderStyleProperties::*setter)(T) noexcept) noexcept
        : Wrapper<T>(property, getter, setter)
    {
    }

    KRYS_NODISCARD bool CanInterpolate(const RenderStyle &, const RenderStyle &,
                                       CompositeOperation) const noexcept final
    {
      return false;
    }
  };

  // MARK: - Font Property Wrappers

  class FontSizeWrapper final : public Wrapper<float>
  {
  public:
    FontSizeWrapper() noexcept
        : Wrapper<float>(css::PropertyId::FontSize, &RenderStyleProperties::ComputedFontSize,
                         &RenderStyleProperties::SetFontSize)
    {
    }

    KRYS_NODISCARD bool Equals(const RenderStyle &a, const RenderStyle &b) const noexcept final
    {
      return a.SpecifiedFontSize() == b.SpecifiedFontSize();
    }
  };

  // MARK: - Color Property Wrappers

  class ColorWrapper final : public WrapperWithGetter<const boo::Color &>
  {
  private:
    void (RenderStyleProperties::*_setter)(boo::Color &&) noexcept;

  public:
    ColorWrapper(css::PropertyId property,
                 const boo::Color &(RenderStyleProperties::*getter)() const noexcept,
                 void (RenderStyleProperties::*setter)(boo::Color &&) noexcept) noexcept
        : WrapperWithGetter<const boo::Color &>(property, getter), _setter(setter)
    {
    }

    void Interpolate(RenderStyle &destination, const RenderStyle &from, const RenderStyle &to,
                     const Context &context) const noexcept override
    {
      (destination.*_setter)(BlendFunc(Value(from), Value(to), context));
    }
  };

  class VisitedAffectedColorWrapper final : public WrapperBase
  {
  protected:
    ColorWrapper _wrapper;
    ColorWrapper _visitedWrapper;

  public:
    VisitedAffectedColorWrapper(css::PropertyId property,
                                const boo::Color &(RenderStyleProperties::*getter)() const noexcept,
                                void (RenderStyleProperties::*setter)(boo::Color &&) noexcept,
                                const boo::Color &(RenderStyleProperties::*visitedGetter)() const noexcept,
                                void (RenderStyleProperties::*visitedSetter)(boo::Color &&) noexcept) noexcept
        : WrapperBase(property), _wrapper(property, getter, setter),
          _visitedWrapper(property, visitedGetter, visitedSetter)
    {
    }

    KRYS_NODISCARD bool Equals(const RenderStyle &a, const RenderStyle &b) const noexcept final
    {
      return _wrapper.Equals(a, b) && _visitedWrapper.Equals(a, b);
    }

    KRYS_NODISCARD bool
      RequiresInterpolationForAccumulativeIteration(const RenderStyle &,
                                                    const RenderStyle &) const noexcept final
    {
      return true;
    }

    void Interpolate(RenderStyle &destination, const RenderStyle &from, const RenderStyle &to,
                     const Context &context) const noexcept final
    {
      _wrapper.Interpolate(destination, from, to, context);
      _visitedWrapper.Interpolate(destination, from, to, context);
    }
  };

  class CaretColorWrapper final : public VisitedAffectedStyleTypeWrapper<style::Color>
  {
  public:
    CaretColorWrapper() noexcept
        : VisitedAffectedStyleTypeWrapper<style::Color>(
            css::PropertyId::CaretColor, &RenderStyleProperties::CaretColor,
            &RenderStyleProperties::SetCaretColor, &RenderStyleProperties::VisitedLinkCaretColor,
            &RenderStyleProperties::SetVisitedLinkCaretColor)
    {
    }

    KRYS_NODISCARD bool Equals(const RenderStyle &a, const RenderStyle &b) const noexcept final
    {
      return a.HasAutoCaretColor() == b.HasAutoCaretColor()
             && a.HasVisitedLinkAutoCaretColor() == b.HasVisitedLinkAutoCaretColor()
             && VisitedAffectedStyleTypeWrapper<style::Color>::Equals(a, b);
    }

    KRYS_NODISCARD bool CanInterpolate(const RenderStyle &from, const RenderStyle &to,
                                       CompositeOperation) const noexcept final
    {
      return CanInterpolateCaretColor(from, to, false) || CanInterpolateCaretColor(from, to, true);
    }

    void Interpolate(RenderStyle &destination, const RenderStyle &from, const RenderStyle &to,
                     const Context &context) const noexcept final
    {
      if (CanInterpolateCaretColor(from, to, false))
      {
        _wrapper.Interpolate(destination, from, to, context);
      }
      else
      {
        const auto &blendingRenderStyle = context.Progress < 0.5 ? from : to;
        if (blendingRenderStyle.HasAutoCaretColor())
        {
          destination.SetHasAutoCaretColor();
        }
        else
        {
          destination.SetCaretColor(style::Color {blendingRenderStyle.CaretColor()});
        }
      }

      if (CanInterpolateCaretColor(from, to, true))
      {
        _visitedWrapper.Interpolate(destination, from, to, context);
      }
      else
      {
        const auto &blendingRenderStyle = context.Progress < 0.5 ? from : to;
        if (blendingRenderStyle.HasVisitedLinkAutoCaretColor())
        {
          destination.SetHasVisitedLinkAutoCaretColor();
        }
        else
        {
          destination.SetVisitedLinkCaretColor(style::Color {blendingRenderStyle.VisitedLinkCaretColor()});
        }
      }
    }

  private:
    KRYS_NODISCARD static bool CanInterpolateCaretColor(const RenderStyle &from, const RenderStyle &to,
                                                        bool visited) noexcept
    {
      if (visited)
      {
        return !from.HasVisitedLinkAutoCaretColor() && !to.HasVisitedLinkAutoCaretColor();
      }
      return !from.HasAutoCaretColor() && !to.HasAutoCaretColor();
    }
  };

  // MARK: - Other Custom Wrappers

  class CounterWrapper final : public WrapperBase
  {
  public:
    explicit CounterWrapper(css::PropertyId property) noexcept : WrapperBase(property)
    {
      krys_debug_assert(property == css::PropertyId::CounterIncrement
                        || property == css::PropertyId::CounterReset
                        || property == css::PropertyId::CounterSet);
    }

    KRYS_NODISCARD bool CanInterpolate(const RenderStyle &, const RenderStyle &,
                                       CompositeOperation) const noexcept final
    {
      return false;
    }

    KRYS_NODISCARD bool Equals(const RenderStyle &a, const RenderStyle &b) const noexcept final
    {
      const auto &mapA = a.GetCounterDirectives().Map;
      const auto &mapB = b.GetCounterDirectives().Map;
      if (mapA.size() != mapB.size())
      {
        return false;
      }
      for (const auto &[key, aDirective] : mapA)
      {
        auto it = mapB.find(key);
        if (it == mapB.end())
        {
          return false;
        }
        const auto &bDirective = it->second;
        if ((Property() == css::PropertyId::CounterIncrement
             && aDirective.IncrementValue != bDirective.IncrementValue)
            || (Property() == css::PropertyId::CounterReset && aDirective.ResetValue != bDirective.ResetValue)
            || (Property() == css::PropertyId::CounterSet && aDirective.SetValue != bDirective.SetValue))
        {
          return false;
        }
      }
      return true;
    }

    void Interpolate(RenderStyle &destination, const RenderStyle &from, const RenderStyle &to,
                     const Context &context) const noexcept final
    {
      krys_debug_assert(context.IsDiscrete);
      krys_debug_assert(!context.Progress || context.Progress == 1);

      // Clear all existing values in the existing set of directives.
      for (auto &[key, directive] : destination.AccessCounterDirectives().Map)
      {
        Field(directive) = null;
      }

      const auto &style = context.Progress ? to : from;
      auto &targetDirectives = destination.AccessCounterDirectives().Map;
      for (const auto &[key, directive] : style.GetCounterDirectives().Map)
      {
        Field(targetDirectives[key]) = Field(directive);
      }
    }

  private:
    KRYS_NODISCARD Maybe<int32> &Field(CounterDirectives &directives) const noexcept
    {
      if (Property() == css::PropertyId::CounterIncrement)
      {
        return directives.IncrementValue;
      }
      if (Property() == css::PropertyId::CounterReset)
      {
        return directives.ResetValue;
      }
      return directives.SetValue;
    }

    KRYS_NODISCARD const Maybe<int32> &Field(const CounterDirectives &directives) const noexcept
    {
      return Field(const_cast<CounterDirectives &>(directives));
    }
  };

  class VisibilityWrapper final : public Wrapper<Visibility>
  {
  public:
    VisibilityWrapper() noexcept
        : Wrapper(css::PropertyId::Visibility, &RenderStyleProperties::GetVisibility,
                  &RenderStyleProperties::SetVisibility)
    {
    }

    KRYS_NODISCARD bool CanInterpolate(const RenderStyle &from, const RenderStyle &to,
                                       CompositeOperation) const noexcept final
    {
      // https://drafts.csswg.org/web-animations-1/#animating-visibility
      // If neither value is visible, then discrete animation is used.
      return Value(from) == Visibility::Visible || Value(to) == Visibility::Visible;
    }
  };

  // MARK: - CoordinatedValueList Wrappers

  // Wrapper base class for an animatable property in a CoordinatedValueList
  template <typename TListValue>
  class CoordinatedValueListPropertyWrapperBase
  {
  private:
    css::PropertyId _property;

  public:
    explicit CoordinatedValueListPropertyWrapperBase(css::PropertyId property) noexcept : _property(property)
    {
    }
    virtual ~CoordinatedValueListPropertyWrapperBase() noexcept = default;

    KRYS_NODISCARD css::PropertyId Property() const noexcept
    {
      return _property;
    }

    KRYS_NODISCARD virtual bool Equals(const TListValue &, const TListValue &) const noexcept = 0;
    virtual void Interpolate(TListValue &, const TListValue &, const TListValue &,
                             const Context &) const noexcept = 0;
    KRYS_NODISCARD virtual bool CanInterpolate(const TListValue &, const TListValue &) const noexcept
    {
      return true;
    }
  };

  template <typename TStyle, typename TListValue>
  class CoordinatedValueListPropertyStyleTypeWrapper final
      : public CoordinatedValueListPropertyWrapperBase<TListValue>
  {
  private:
    const TStyle &(TListValue::*_getter)() const noexcept;
    void (TListValue::*_setter)(TStyle &&) noexcept;

  public:
    CoordinatedValueListPropertyStyleTypeWrapper(css::PropertyId property,
                                                 const TStyle &(TListValue::*getter)() const noexcept,
                                                 void (TListValue::*setter)(TStyle &&) noexcept) noexcept
        : CoordinatedValueListPropertyWrapperBase<TListValue>(property), _getter(getter), _setter(setter)
    {
    }

    KRYS_NODISCARD bool Equals(const TListValue &from, const TListValue &to) const noexcept override
    {
      if (&from == &to)
      {
        return true;
      }
      return style::EqualsForBlending(Value(from), Value(to));
    }

    KRYS_NODISCARD bool CanInterpolate(const TListValue &from, const TListValue &to) const noexcept final
    {
      return style::CanBlend(Value(from), Value(to));
    }

    void Interpolate(TListValue &destination, const TListValue &from, const TListValue &to,
                     const Context &context) const noexcept final
    {
      (destination.*_setter)(style::Blend(Value(from), Value(to), context));
    }

  private:
    KRYS_NODISCARD const TStyle &Value(const TListValue &value) const noexcept
    {
      return (value.*_getter)();
    }
  };

  template <typename T, typename TListValue, typename TGetter = T, typename TSetter = T>
  class DiscreteCoordinatedValueListPropertyWrapper final
      : public CoordinatedValueListPropertyWrapperBase<TListValue>
  {
  private:
    TGetter (TListValue::*_getter)() const noexcept;
    void (TListValue::*_setter)(TSetter) noexcept;

  public:
    DiscreteCoordinatedValueListPropertyWrapper(css::PropertyId property,
                                                TGetter (TListValue::*getter)() const noexcept,
                                                void (TListValue::*setter)(TSetter) noexcept) noexcept
        : CoordinatedValueListPropertyWrapperBase<TListValue>(property), _getter(getter), _setter(setter)
    {
    }

    KRYS_NODISCARD bool Equals(const TListValue &a, const TListValue &b) const noexcept final
    {
      return Value(a) == Value(b);
    }

    KRYS_NODISCARD bool CanInterpolate(const TListValue &, const TListValue &) const noexcept final
    {
      return false;
    }

    void Interpolate(TListValue &destination, const TListValue &from, const TListValue &to,
                     const Context &context) const noexcept final
    {
      krys_debug_assert(!context.Progress || context.Progress == 1.0);
      (destination.*_setter)(T {context.Progress ? Value(to) : Value(from)});
    }

  private:
    KRYS_NODISCARD TGetter Value(const TListValue &list) const noexcept
    {
      return (list.*_getter)();
    }
  };

  // Deduction guide for getter/setters that return and take values.
  template <typename T, typename TListValue>
  DiscreteCoordinatedValueListPropertyWrapper(css::PropertyId, T (TListValue::*getter)() const noexcept,
                                              void (TListValue::*setter)(T) noexcept)
    -> DiscreteCoordinatedValueListPropertyWrapper<T, TListValue, T, T>;

  // Deduction guide for getter/setters that return const references and take r-value references.
  template <typename T, typename TListValue>
  DiscreteCoordinatedValueListPropertyWrapper(css::PropertyId,
                                              const T &(TListValue::*getter)() const noexcept,
                                              void (TListValue::*setter)(T &&) noexcept)
    -> DiscreteCoordinatedValueListPropertyWrapper<T, TListValue, const T &, T &&>;

  // Deduction guide for getter/setters that return values and take r-value references.
  template <typename T, typename TListValue>
  DiscreteCoordinatedValueListPropertyWrapper(css::PropertyId, T (TListValue::*getter)() const noexcept,
                                              void (TListValue::*setter)(T &&) noexcept)
    -> DiscreteCoordinatedValueListPropertyWrapper<T, TListValue, T, T &&>;

  template <typename T, typename TRepeatedValueWrapper>
  class CoordinatedValueListPropertyWrapper final : public WrapperBase
  {
  public:
    using ListType = T;
    using ListValue = typename ListType::value_type;

    using ListGetter = const ListType &(RenderStyleBase::*)() const noexcept;
    using ListAccessor = ListType &(RenderStyleBase::*)() noexcept;
    using ListSetter = void (RenderStyleBase::*)(ListType &&) noexcept;

  private:
    ListGetter _listGetter;
    ListAccessor _listAccessor;
    ListSetter _listSetter;
    TRepeatedValueWrapper _repeatedValueWrapper;

  public:
    CoordinatedValueListPropertyWrapper(css::PropertyId property, ListGetter getter, ListAccessor accessor,
                                        ListSetter setter,
                                        TRepeatedValueWrapper repeatedValueWrapper) noexcept
        : WrapperBase(property), _listGetter(getter), _listAccessor(accessor), _listSetter(setter),
          _repeatedValueWrapper(krys::move(repeatedValueWrapper))
    {
    }

    KRYS_NODISCARD bool Equals(const RenderStyle &from, const RenderStyle &to) const noexcept final
    {
      if (&from == &to)
      {
        return true;
      }

      const auto &fromList = (from.*_listGetter)();
      const auto &toList = (to.*_listGetter)();

      auto numberOfValues = std::min(fromList.ComputedLength(), toList.ComputedLength());
      for (size_t i = 0; i < numberOfValues; ++i)
      {
        if (!_repeatedValueWrapper.Equals(fromList[i], toList[i]))
        {
          return false;
        }
      }

      return true;
    }

    KRYS_NODISCARD bool CanInterpolate(const RenderStyle &from, const RenderStyle &to,
                                       CompositeOperation) const noexcept final
    {
      const auto &fromList = (from.*_listGetter)();
      const auto &toList = (to.*_listGetter)();

      auto numberOfValues = std::min(fromList.ComputedLength(), toList.ComputedLength());
      for (size_t i = 0; i < numberOfValues; ++i)
      {
        const auto &fromValue = fromList[i];
        const auto &toValue = toList[i];

        // First check if the owner values allow interpolation.
        if (!style::CanBlend(fromValue, toValue))
        {
          return false;
        }

        // Then check if the individual property values allow interpolation.
        if (!_repeatedValueWrapper.CanInterpolate(fromValue, toValue))
        {
          return false;
        }
      }

      return true;
    }

    void Interpolate(RenderStyle &destination, const RenderStyle &from, const RenderStyle &to,
                     const Context &context) const noexcept final
    {
      const auto *fromList = &(from.*_listGetter)();
      const auto *toList = &(to.*_listGetter)();
      auto &destinationList = (destination.*_listAccessor)();

      if (context.IsDiscrete)
      {
        krys_debug_assert(!context.Progress || context.Progress == 1.0);
        const auto *list = context.Progress ? toList : fromList;
        fromList = list;
        toList = list;
      }

      auto numberOfDestinationValues = destinationList.ComputedLength();
      auto numberOfValues = std::min(fromList->ComputedLength(), toList->ComputedLength());

      for (size_t i = 0; i < numberOfValues; ++i)
      {
        if (i >= numberOfDestinationValues)
        {
          destinationList.Append(ListValue {});
        }

        _repeatedValueWrapper.Interpolate(destinationList[i], (*fromList)[i], (*toList)[i], context);
      }

      destinationList.PrepareForUse();
    }
  };

  // MARK: - Shorthand Wrapper

  class ShorthandWrapper final : public WrapperBase
  {
  private:
    List<WrapperBase *> _longhandWrappers;

  public:
    ShorthandWrapper(css::PropertyId property, List<WrapperBase *> &&longhandWrappers) noexcept
        : WrapperBase(property), _longhandWrappers(krys::move(longhandWrappers))
    {
    }

    KRYS_NODISCARD bool Equals(const RenderStyle &a, const RenderStyle &b) const noexcept final
    {
      if (&a == &b)
      {
        return true;
      }

      return std::ranges::all_of(_longhandWrappers,
                                 [&](WrapperBase *wrapper) { return wrapper->Equals(a, b); });
    }

    void Interpolate(RenderStyle &destination, const RenderStyle &from, const RenderStyle &to,
                     const Context &context) const noexcept final
    {
      for (auto *wrapper : _longhandWrappers)
      {
        wrapper->Interpolate(destination, from, to, context);
      }
    }
  };
}
