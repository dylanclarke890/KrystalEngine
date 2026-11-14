/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/Queries/MediaQueryFeatures.hpp"
#include "Krystal.Booey/CSS/Queries/GenericMediaQueryEvaluator.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Threading/NeverDestroyed.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::css::mq::Features
{
  using Dependency = MediaQueryDynamicDependency;

  struct BooleanSchema : public FeatureSchema
  {
    using ValueFunction = Func<bool(const FeatureEvaluationContext &)>;

    BooleanSchema(const CSSOMStringAtom &name, MediaQueryDynamicDependency dependencies,
                  ValueFunction &&valueFunction) noexcept
        : FeatureSchema(name, FeatureSchema::Type::Discrete, FeatureSchema::ValueType::Integer, dependencies),
          _valueFunction(krys::move(valueFunction))
    {
    }

    // FeatureSchema conformance

    KRYS_NODISCARD EvaluationResult Evaluate(const Feature &feature,
                                             const FeatureEvaluationContext &context) const noexcept override
    {
      return EvaluateBooleanFeature(feature, _valueFunction(context), context.ConversionData);
    }

  private:
    ValueFunction _valueFunction;
  };

  struct IntegerSchema : public FeatureSchema
  {
    using ValueFunction = Func<int32(const FeatureEvaluationContext &)>;

    IntegerSchema(const CSSOMStringAtom &name, MediaQueryDynamicDependency dependencies,
                  ValueFunction &&valueFunction) noexcept
        : FeatureSchema(name, FeatureSchema::Type::Range, FeatureSchema::ValueType::Integer, dependencies),
          _valueFunction(krys::move(valueFunction))
    {
    }

    // FeatureSchema conformance

    KRYS_NODISCARD EvaluationResult Evaluate(const Feature &feature,
                                             const FeatureEvaluationContext &context) const noexcept override
    {
      return EvaluateIntegerFeature(feature, _valueFunction(context), context.ConversionData);
    }

  private:
    ValueFunction _valueFunction;
  };

  struct LengthSchema : public FeatureSchema
  {
    using ValueFunction = Func<LayoutUnit(const FeatureEvaluationContext &)>;

    LengthSchema(const CSSOMStringAtom &name, MediaQueryDynamicDependency dependencies,
                 ValueFunction &&valueFunction) noexcept
        : FeatureSchema(name, FeatureSchema::Type::Range, FeatureSchema::ValueType::Length, dependencies),
          _valueFunction(krys::move(valueFunction))
    {
    }

    // FeatureSchema conformance

    KRYS_NODISCARD EvaluationResult Evaluate(const Feature &feature,
                                             const FeatureEvaluationContext &context) const noexcept override
    {
      return EvaluateLengthFeature(feature, _valueFunction(context), context.ConversionData);
    }

  private:
    ValueFunction _valueFunction;
  };

  struct RatioSchema : public FeatureSchema
  {
    using ValueFunction = Func<FloatSize(const FeatureEvaluationContext &)>;

    RatioSchema(const CSSOMStringAtom &name, MediaQueryDynamicDependency dependencies,
                ValueFunction &&valueFunction) noexcept
        : FeatureSchema(name, FeatureSchema::Type::Range, FeatureSchema::ValueType::Ratio, dependencies),
          _valueFunction(krys::move(valueFunction))
    {
    }

    // FeatureSchema conformance

    KRYS_NODISCARD EvaluationResult Evaluate(const Feature &feature,
                                             const FeatureEvaluationContext &context) const noexcept override
    {
      return EvaluateRatioFeature(feature, _valueFunction(context), context.ConversionData);
    }

  private:
    ValueFunction _valueFunction;
  };

  struct ResolutionSchema : public FeatureSchema
  {
    using ValueFunction = Func<float(const FeatureEvaluationContext &)>;

    ResolutionSchema(const CSSOMStringAtom &name, MediaQueryDynamicDependency dependencies,
                     ValueFunction &&valueFunction) noexcept
        : FeatureSchema(name, FeatureSchema::Type::Range, FeatureSchema::ValueType::Resolution, dependencies),
          _valueFunction(krys::move(valueFunction))
    {
    }

    // FeatureSchema conformance

    KRYS_NODISCARD EvaluationResult Evaluate(const Feature &feature,
                                             const FeatureEvaluationContext &context) const noexcept override
    {
      return EvaluateResolutionFeature(feature, _valueFunction(context), context.ConversionData);
    }

  private:
    ValueFunction _valueFunction;
  };

  using MatchingIdentifiers = SmallList<ValueId, 1>;

  struct IdentifierSchema : public FeatureSchema
  {
    using ValueFunction = Func<MatchingIdentifiers(const FeatureEvaluationContext &)>;

    IdentifierSchema(const CSSOMStringAtom &name, FixedVector<ValueId> &&valueIdentifiers,
                     MediaQueryDynamicDependency dependencies, ValueFunction &&valueFunction) noexcept
        : FeatureSchema(name, FeatureSchema::Type::Discrete, FeatureSchema::ValueType::Identifier,
                        dependencies, krys::move(valueIdentifiers)),
          _valueFunction(krys::move(valueFunction))
    {
    }

    KRYS_NODISCARD EvaluationResult Evaluate(const Feature &feature,
                                             const FeatureEvaluationContext &context) const noexcept override
    {
      auto valueIds = _valueFunction(context);
      for (auto valueId : valueIds)
      {
        krys_debug_assert(ValueIdentifiers.contains(valueId));
        if (EvaluateIdentifierFeature(feature, valueId, context.ConversionData) == EvaluationResult::True)
        {
          return EvaluationResult::True;
        }
      }
      return EvaluationResult::False;
    }

  private:
    ValueFunction _valueFunction;
  };

  /// @brief The document's page, if its frame has one.
  KRYS_NODISCARD static const Page *PageOf(const FeatureEvaluationContext &context) noexcept
  {
    return context.Document->GetPage();
  }

  /// @brief The chrome client of the document's page, which answers for the screen, the pointing devices
  /// and the user's preferences; a frame without a page gets an empty chrome client's answers.
  KRYS_NODISCARD static const ChromeClient &ChromeOf(const FeatureEvaluationContext &context) noexcept
  {
    if (auto *page = PageOf(context))
    {
      return page->GetChromeClient();
    }
    static MainThreadNeverDestroyed<EmptyChromeClient> emptyChromeClient;
    return emptyChromeClient;
  }

  KRYS_NODISCARD static const LocalFrameView &ViewOf(const FeatureEvaluationContext &context) noexcept
  {
    auto *view = context.Document->View();
    krys_assert_msg(view, "media features are only evaluated for a document with a view");
    return *view;
  }

  KRYS_NODISCARD static float DeviceScaleFactor(const FeatureEvaluationContext &context) noexcept
  {
    auto &mediaType = ViewOf(context).MediaType();

    if (mediaType == ScreenAtom())
    {
      auto *page = PageOf(context);
      return page ? page->DeviceScaleFactor() : 1.f;
    }
    if (mediaType == PrintAtom())
    {
      // The resolution of images while printing should not depend on the dpi
      // of the screen. Until we support proper ways of querying this info
      // we use 300px which is considered minimum for current printers.
      return 3.125; // 300dpi / 96dpi;
    }
    return 0;
  }

  KRYS_NODISCARD static MatchingIdentifiers
    IdentifiersForPointerCharacteristics(PointerCharacteristics pointerCharacteristics) noexcept
  {
    MatchingIdentifiers identifiers;
    if (HasFlag(pointerCharacteristics, PointerCharacteristics::Fine))
    {
      identifiers.push_back(ValueId::Fine);
    }
    if (HasFlag(pointerCharacteristics, PointerCharacteristics::Coarse))
    {
      identifiers.push_back(ValueId::Coarse);
    }
    if (identifiers.empty())
    {
      identifiers.push_back(ValueId::None);
    }
    return identifiers;
  }

  // MARK: - Singleton readonly instances of FeatureSchemas

  KRYS_NODISCARD static const IdentifierSchema &AnyHoverFeatureSchema() noexcept
  {
    static MainThreadNeverDestroyed<IdentifierSchema> schema {
      u8"any-hover", FixedVector {ValueId::None, ValueId::Hover}, Dependency::None, [](auto &context)
      {
        bool isSupported = ChromeOf(context).HoverSupportedByAnyAvailablePointingDevice();
        return MatchingIdentifiers {isSupported ? ValueId::Hover : ValueId::None};
      }};
    return schema;
  }

  KRYS_NODISCARD static const IdentifierSchema &AnyPointerFeatureSchema() noexcept
  {
    static MainThreadNeverDestroyed<IdentifierSchema> schema {
      u8"any-pointer", FixedVector {ValueId::None, ValueId::Fine, ValueId::Coarse}, Dependency::None,
      [](auto &context)
      { return IdentifiersForPointerCharacteristics(ChromeOf(context).AllAvailablePointingDevices()); }};
    return schema;
  }

  KRYS_NODISCARD static const RatioSchema &AspectRatioFeatureSchema() noexcept
  {
    static MainThreadNeverDestroyed<RatioSchema> schema {
      u8"aspect-ratio", Dependency::Viewport, [](auto &context)
      {
        auto &view = ViewOf(context);
        return FloatSize(static_cast<float>(view.LayoutWidth()), static_cast<float>(view.LayoutHeight()));
      }};
    return schema;
  }

  KRYS_NODISCARD static const IntegerSchema &ColorFeatureSchema() noexcept
  {
    static MainThreadNeverDestroyed<IntegerSchema> schema {
      u8"color", Dependency::None, [](auto &context) { return ChromeOf(context).ScreenDepthPerComponent(); }};
    return schema;
  }

  KRYS_NODISCARD static const IdentifierSchema &ColorGamutFeatureSchema() noexcept
  {
    static MainThreadNeverDestroyed<IdentifierSchema> schema {
      u8"color-gamut", FixedVector {ValueId::SRGB, ValueId::P3, ValueId::Rec2020}, Dependency::None,
      [](auto &context)
      {
        MatchingIdentifiers identifiers {ValueId::SRGB};
        if (ChromeOf(context).ScreenSupportsExtendedColor())
        {
          identifiers.push_back(ValueId::P3);
        }
        if (ChromeOf(context).ScreenSupportsRec2020Color())
        {
          identifiers.push_back(ValueId::Rec2020);
        }
        return identifiers;
      }};
    return schema;
  }

  KRYS_NODISCARD static const IntegerSchema &ColorIndexFeatureSchema() noexcept
  {
    static MainThreadNeverDestroyed<IntegerSchema> schema {u8"color-index", Dependency::None,
                                                           [](auto &) { return 0; }};
    return schema;
  }

  KRYS_NODISCARD static const RatioSchema &DeviceAspectRatioFeatureSchema() noexcept
  {
    static MainThreadNeverDestroyed<RatioSchema> schema {u8"device-aspect-ratio", Dependency::None,
                                                         [](auto &context)
                                                         { return ChromeOf(context).ScreenSize(); }};
    return schema;
  }

  KRYS_NODISCARD static const LengthSchema &DeviceHeightFeatureSchema() noexcept
  {
    static MainThreadNeverDestroyed<LengthSchema> schema {
      u8"device-height", Dependency::None,
      [](auto &context) { return LayoutUnit {ChromeOf(context).ScreenSize().Height()}; }};
    return schema;
  }

  KRYS_NODISCARD static const LengthSchema &DeviceWidthFeatureSchema() noexcept
  {
    static MainThreadNeverDestroyed<LengthSchema> schema {
      u8"device-width", Dependency::None,
      [](auto &context) { return LayoutUnit {ChromeOf(context).ScreenSize().Width()}; }};
    return schema;
  }

  KRYS_NODISCARD static const IdentifierSchema &DynamicRangeFeatureSchema() noexcept
  {
    static MainThreadNeverDestroyed<IdentifierSchema> schema {
      u8"dynamic-range", FixedVector {ValueId::Standard, ValueId::High}, Dependency::None, [](auto &context)
      {
        MatchingIdentifiers identifiers {ValueId::Standard};
        if (ChromeOf(context).ScreenSupportsHighDynamicRange())
        {
          identifiers.push_back(ValueId::High);
        }
        return identifiers;
      }};
    return schema;
  }

  KRYS_NODISCARD static const IdentifierSchema &ForcedColorsFeatureSchema() noexcept
  {
    static MainThreadNeverDestroyed<IdentifierSchema> schema {
      u8"forced-colors", FixedVector {ValueId::None, ValueId::Active}, Dependency::None,
      [](auto &) { return MatchingIdentifiers {ValueId::None}; }};
    return schema;
  }

  KRYS_NODISCARD static const BooleanSchema &GridFeatureSchema() noexcept
  {
    static MainThreadNeverDestroyed<BooleanSchema> schema {u8"grid", Dependency::None,
                                                           [](auto &) { return false; }};
    return schema;
  }

  KRYS_NODISCARD static const LengthSchema &HeightFeatureSchema() noexcept
  {
    static MainThreadNeverDestroyed<LengthSchema> schema {
      u8"height", Dependency::Viewport, [](auto &context)
      {
        auto height = ViewOf(context).LayoutHeight();
        if (auto *renderView = context.Document->GetRenderView())
        {
          height = AdjustForAbsoluteZoom(height, renderView->Style());
        }
        return LayoutUnit {height};
      }};
    return schema;
  }

  KRYS_NODISCARD static const IdentifierSchema &HoverFeatureSchema() noexcept
  {
    static MainThreadNeverDestroyed<IdentifierSchema> schema {
      u8"hover", FixedVector {ValueId::None, ValueId::Hover}, Dependency::None, [](auto &context)
      {
        bool isSupported = ChromeOf(context).HoverSupportedByPrimaryPointingDevice();
        return MatchingIdentifiers {isSupported ? ValueId::Hover : ValueId::None};
      }};
    return schema;
  }

  KRYS_NODISCARD static const IdentifierSchema &InvertedColorsFeatureSchema() noexcept
  {
    static MainThreadNeverDestroyed<IdentifierSchema> schema {
      u8"inverted-colors", FixedVector {ValueId::None, ValueId::Inverted}, Dependency::Accessibility,
      [](auto &context)
      {
        bool isInverted = ChromeOf(context).ColorsAreInverted();
        return MatchingIdentifiers {isInverted ? ValueId::Inverted : ValueId::None};
      }};
    return schema;
  }

  KRYS_NODISCARD static const IntegerSchema &MonochromeFeatureSchema() noexcept
  {
    static MainThreadNeverDestroyed<IntegerSchema> schema {
      u8"monochrome", Dependency::Accessibility, [](auto &context)
      {
        auto &chrome = ChromeOf(context);
        return chrome.ScreenIsMonochrome() ? chrome.ScreenDepthPerComponent() : 0;
      }};
    return schema;
  }

  KRYS_NODISCARD static const IdentifierSchema &OrientationFeatureSchema() noexcept
  {
    static MainThreadNeverDestroyed<IdentifierSchema> schema {
      u8"orientation", FixedVector {ValueId::Landscape, ValueId::Portrait}, Dependency::Viewport,
      [](auto &context)
      {
        auto layoutSize = ViewOf(context).GetLayoutSize();
        // Square viewport is portrait.
        bool isPortrait = layoutSize.Height() >= layoutSize.Width();
        return MatchingIdentifiers {isPortrait ? ValueId::Portrait : ValueId::Landscape};
      }};
    return schema;
  }

  KRYS_NODISCARD static const IdentifierSchema &PointerFeatureSchema() noexcept
  {
    static MainThreadNeverDestroyed<IdentifierSchema> schema {
      u8"pointer", FixedVector {ValueId::None, ValueId::Fine, ValueId::Coarse}, Dependency::None,
      [](auto &context)
      { return IdentifiersForPointerCharacteristics(ChromeOf(context).PrimaryPointingDevice()); }};
    return schema;
  }

  KRYS_NODISCARD static const IdentifierSchema &PrefersContrastFeatureSchema() noexcept
  {
    static MainThreadNeverDestroyed<IdentifierSchema> schema {
      u8"prefers-contrast",
      FixedVector {ValueId::NoPreference, ValueId::More, ValueId::Less, ValueId::Custom},
      Dependency::Accessibility, [](auto &context)
      {
        bool userPrefersContrast = ChromeOf(context).UserPrefersContrast();
        return MatchingIdentifiers {userPrefersContrast ? ValueId::More : ValueId::NoPreference};
      }};
    return schema;
  }

  KRYS_NODISCARD static const IdentifierSchema &PrefersReducedMotionFeatureSchema() noexcept
  {
    static MainThreadNeverDestroyed<IdentifierSchema> schema {
      u8"prefers-reduced-motion", FixedVector {ValueId::NoPreference, ValueId::Reduce},
      Dependency::Accessibility, [](auto &context)
      {
        bool userPrefersReducedMotion = ChromeOf(context).UserPrefersReducedMotion();
        return MatchingIdentifiers {userPrefersReducedMotion ? ValueId::Reduce : ValueId::NoPreference};
      }};
    return schema;
  }

  KRYS_NODISCARD static const ResolutionSchema &ResolutionFeatureSchema() noexcept
  {
    static MainThreadNeverDestroyed<ResolutionSchema> schema {
      u8"resolution", Dependency::None, [](auto &context) { return DeviceScaleFactor(context); }};
    return schema;
  }

  KRYS_NODISCARD static const IdentifierSchema &ScanFeatureSchema() noexcept
  {
    static MainThreadNeverDestroyed<IdentifierSchema> schema {
      u8"scan", FixedVector {ValueId::Interlace, ValueId::Progressive}, Dependency::None,
      [](auto &) { return MatchingIdentifiers {}; }};
    return schema;
  }

  KRYS_NODISCARD static const IdentifierSchema &ScriptingFeatureSchema() noexcept
  {
    static MainThreadNeverDestroyed<IdentifierSchema> schema {
      u8"scripting", FixedVector {ValueId::None, ValueId::InitialOnly, ValueId::Enabled}, Dependency::None,
      [](auto &)
      {
        // Scripting is always enabled: the application plays the part of scripts.
        return MatchingIdentifiers {ValueId::Enabled};
      }};
    return schema;
  }

  KRYS_NODISCARD static const IdentifierSchema &UpdateFeatureSchema() noexcept
  {
    static MainThreadNeverDestroyed<IdentifierSchema> schema {
      u8"update", FixedVector {ValueId::None, ValueId::Slow, ValueId::Fast}, Dependency::None,
      [](auto &context)
      {
        if (ViewOf(context).MediaType() == PrintAtom())
        {
          return MatchingIdentifiers {ValueId::None};
        }

        // FIXME: Potentially add a hook for ports to change this value.
        return MatchingIdentifiers {ValueId::Fast};
      }};
    return schema;
  }

  KRYS_NODISCARD static const LengthSchema &WidthFeatureSchema() noexcept
  {
    static MainThreadNeverDestroyed<LengthSchema> schema {
      u8"width", Dependency::Viewport, [](auto &context)
      {
        auto width = ViewOf(context).LayoutWidth();
        if (auto *renderView = context.Document->GetRenderView())
        {
          width = AdjustForAbsoluteZoom(width, renderView->Style());
        }
        return LayoutUnit {width};
      }};
    return schema;
  }

  KRYS_NODISCARD static const IdentifierSchema &DisplayModeFeatureSchema() noexcept
  {
    static MainThreadNeverDestroyed<IdentifierSchema> schema {
      u8"display-mode",
      FixedVector {ValueId::Fullscreen, ValueId::Standalone, ValueId::MinimalUi, ValueId::Browser},
      Dependency::None, [](auto &context)
      {
        auto identifier = [&]
        {
          auto *page = PageOf(context);
          switch (page ? page->DisplayMode() : ApplicationDisplayMode::Browser)
          {
            case ApplicationDisplayMode::Fullscreen: return ValueId::Fullscreen;
            case ApplicationDisplayMode::Standalone: return ValueId::Standalone;
            case ApplicationDisplayMode::MinimalUI:  return ValueId::MinimalUi;
            case ApplicationDisplayMode::Browser:    return ValueId::Browser;
          }
          krys_unreachable();
        }();

        return MatchingIdentifiers {identifier};
      }};
    return schema;
  }

  KRYS_NODISCARD static const IdentifierSchema &OverflowBlockFeatureSchema() noexcept
  {
    static MainThreadNeverDestroyed<IdentifierSchema> schema {
      u8"overflow-block", FixedVector {ValueId::None, ValueId::Scroll, ValueId::Paged}, Dependency::None,
      [](auto &context)
      {
        // FIXME: Match none when scrollEnabled is set to false by UIKit.
        auto &view = ViewOf(context);
        return MatchingIdentifiers {view.MediaType() == PrintAtom() ? ValueId::Paged : ValueId::Scroll};
      }};
    return schema;
  }

  KRYS_NODISCARD static const IdentifierSchema &OverflowInlineFeatureSchema() noexcept
  {
    static MainThreadNeverDestroyed<IdentifierSchema> schema {
      u8"overflow-inline", FixedVector {ValueId::None, ValueId::Scroll}, Dependency::None, [](auto &)
      {
        // FIXME: Match none when scrollEnabled is set to false by UIKit.
        return MatchingIdentifiers {ValueId::Scroll};
      }};
    return schema;
  }

  KRYS_NODISCARD static const IdentifierSchema &PrefersColorSchemeFeatureSchema() noexcept
  {
    static MainThreadNeverDestroyed<IdentifierSchema> schema {
      u8"prefers-color-scheme", FixedVector {ValueId::Light, ValueId::Dark}, Dependency::Appearance,
      [](auto &context)
      {
        auto *page = PageOf(context);
        bool useDarkAppearance = page && page->UseDarkAppearance();
        return MatchingIdentifiers {useDarkAppearance ? ValueId::Dark : ValueId::Light};
      }};
    return schema;
  }

  // MARK: - Type erased exposed schemas

  const FeatureSchema &AnyHover() noexcept
  {
    return AnyHoverFeatureSchema();
  }

  const FeatureSchema &AnyPointer() noexcept
  {
    return AnyPointerFeatureSchema();
  }

  const FeatureSchema &AspectRatio() noexcept
  {
    return AspectRatioFeatureSchema();
  }

  const FeatureSchema &Color() noexcept
  {
    return ColorFeatureSchema();
  }

  const FeatureSchema &ColorGamut() noexcept
  {
    return ColorGamutFeatureSchema();
  }

  const FeatureSchema &ColorIndex() noexcept
  {
    return ColorIndexFeatureSchema();
  }

  const FeatureSchema &DeviceAspectRatio() noexcept
  {
    return DeviceAspectRatioFeatureSchema();
  }

  const FeatureSchema &DeviceHeight() noexcept
  {
    return DeviceHeightFeatureSchema();
  }

  const FeatureSchema &DeviceWidth() noexcept
  {
    return DeviceWidthFeatureSchema();
  }

  const FeatureSchema &DynamicRange() noexcept
  {
    return DynamicRangeFeatureSchema();
  }

  const FeatureSchema &ForcedColors() noexcept
  {
    return ForcedColorsFeatureSchema();
  }

  const FeatureSchema &Grid() noexcept
  {
    return GridFeatureSchema();
  }

  const FeatureSchema &Height() noexcept
  {
    return HeightFeatureSchema();
  }

  const FeatureSchema &Hover() noexcept
  {
    return HoverFeatureSchema();
  }

  const FeatureSchema &InvertedColors() noexcept
  {
    return InvertedColorsFeatureSchema();
  }

  const FeatureSchema &Monochrome() noexcept
  {
    return MonochromeFeatureSchema();
  }

  const FeatureSchema &Orientation() noexcept
  {
    return OrientationFeatureSchema();
  }

  const FeatureSchema &Pointer() noexcept
  {
    return PointerFeatureSchema();
  }

  const FeatureSchema &PrefersContrast() noexcept
  {
    return PrefersContrastFeatureSchema();
  }

  const FeatureSchema &PrefersReducedMotion() noexcept
  {
    return PrefersReducedMotionFeatureSchema();
  }

  const FeatureSchema &Resolution() noexcept
  {
    return ResolutionFeatureSchema();
  }

  const FeatureSchema &Scan() noexcept
  {
    return ScanFeatureSchema();
  }

  const FeatureSchema &Scripting() noexcept
  {
    return ScriptingFeatureSchema();
  }

  const FeatureSchema &Update() noexcept
  {
    return UpdateFeatureSchema();
  }

  const FeatureSchema &Width() noexcept
  {
    return WidthFeatureSchema();
  }

  const FeatureSchema &DisplayMode() noexcept
  {
    return DisplayModeFeatureSchema();
  }

  const FeatureSchema &OverflowBlock() noexcept
  {
    return OverflowBlockFeatureSchema();
  }

  const FeatureSchema &OverflowInline() noexcept
  {
    return OverflowInlineFeatureSchema();
  }

  const FeatureSchema &PrefersColorScheme() noexcept
  {
    return PrefersColorSchemeFeatureSchema();
  }

  List<const FeatureSchema *> AllSchemas() noexcept
  {
    return {
      &AnyHover(),
      &AnyPointer(),
      &AspectRatio(),
      &Color(),
      &ColorGamut(),
      &ColorIndex(),
      &DeviceAspectRatio(),
      &DeviceHeight(),
      &DeviceWidth(),
      &DynamicRange(),
      &ForcedColors(),
      &Grid(),
      &Height(),
      &Hover(),
      &InvertedColors(),
      &Monochrome(),
      &OverflowBlock(),
      &OverflowInline(),
      &Orientation(),
      &Pointer(),
      &PrefersContrast(),
      &PrefersReducedMotion(),
      &Resolution(),
      &Scan(),
      &Scripting(),
      &Update(),
      &Width(),
      &DisplayMode(),
      &PrefersColorScheme(),
    };
  }
}
