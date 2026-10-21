#include "Krystal.Booey/Rendering/CSSFilterRenderer.hpp"
#include "Krystal.Booey/Graphics/Filters/FEColorMatrix.hpp"
#include "Krystal.Booey/Graphics/Filters/FEDropShadow.hpp"
#include "Krystal.Booey/Graphics/Filters/FEGaussianBlur.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterOperations.hpp"
#include "Krystal.Booey/Graphics/Filters/SourceGraphic.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/DropShadowFilterOperationWithStyleColor.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/Filter.hpp"
#include "Krystal.Core/Colour/ColourMatrix.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  namespace
  {
    /// @brief The value of a `filter` property's list, or of a list of filter operations.
    KRYS_NODISCARD const FilterOperation &OperationOf(const style::FilterValue &value) noexcept
    {
      return value.Get();
    }
    KRYS_NODISCARD const FilterOperation &OperationOf(const Ref<FilterOperation> &operation) noexcept
    {
      return *operation;
    }

    /// @brief A matrix's rows as `feColorMatrix` values.
    KRYS_NODISCARD List<float> ValuesOf(const ColourMatrix<5, 4> &matrix) noexcept
    {
      const auto &data = matrix.data();
      return List<float>(data.begin(), data.end());
    }
    template <size_t ColumnCount, size_t RowCount>
    KRYS_NODISCARD List<float> ValuesOf(const ColourMatrix<ColumnCount, RowCount> &matrix) noexcept
    {
      return ValuesOf(ColourMatrix<5, 4>(matrix));
    }

    KRYS_NODISCARD RefPtr<FilterEffect> CreateBlurEffect(const BlurFilterOperation &blurOperation) noexcept
    {
      float stdDeviation = blurOperation.StdDeviation();
      return FEGaussianBlur::Create(stdDeviation, stdDeviation, EdgeModeType::None);
    }

    KRYS_NODISCARD RefPtr<FilterEffect>
      CreateBrightnessEffect(const BasicComponentTransferFilterOperation &componentTransferOperation) noexcept
    {
      auto amount = static_cast<float>(componentTransferOperation.Amount());
      return FEColorMatrix::Create(ColorMatrixType::FECOLORMATRIX_TYPE_MATRIX,
                                   ValuesOf(BrightnessColourMatrix(amount)));
    }

    KRYS_NODISCARD RefPtr<FilterEffect>
      CreateContrastEffect(const BasicComponentTransferFilterOperation &componentTransferOperation) noexcept
    {
      auto amount = static_cast<float>(componentTransferOperation.Amount());
      return FEColorMatrix::Create(ColorMatrixType::FECOLORMATRIX_TYPE_MATRIX,
                                   ValuesOf(ContrastColourMatrix(amount)));
    }

    KRYS_NODISCARD RefPtr<FilterEffect>
      CreateDropShadowEffect(const DropShadowFilterOperation &dropShadowOperation) noexcept
    {
      auto std = static_cast<float>(dropShadowOperation.StdDeviation());
      return FEDropShadow::Create(std, std, static_cast<float>(dropShadowOperation.X()),
                                  static_cast<float>(dropShadowOperation.Y()), dropShadowOperation.GetColor(),
                                  1);
    }

    KRYS_NODISCARD RefPtr<FilterEffect>
      CreateDropShadowEffect(const style::DropShadowFilterOperationWithStyleColor &dropShadowOperation,
                             const RenderStyle &style) noexcept
    {
      auto std = static_cast<float>(dropShadowOperation.StdDeviation());
      return FEDropShadow::Create(std, std, static_cast<float>(dropShadowOperation.X()),
                                  static_cast<float>(dropShadowOperation.Y()),
                                  style.ColorResolvingCurrentColor(dropShadowOperation.StyleColor()), 1);
    }

    KRYS_NODISCARD RefPtr<FilterEffect>
      CreateGrayScaleEffect(const BasicColorMatrixFilterOperation &colorMatrixOperation) noexcept
    {
      auto grayscaleMatrix = GrayscaleColourMatrix(static_cast<float>(colorMatrixOperation.Amount()));
      List<float> inputParameters {
        grayscaleMatrix.at(0, 0),
        grayscaleMatrix.at(0, 1),
        grayscaleMatrix.at(0, 2),
        0,
        0,
        grayscaleMatrix.at(1, 0),
        grayscaleMatrix.at(1, 1),
        grayscaleMatrix.at(1, 2),
        0,
        0,
        grayscaleMatrix.at(2, 0),
        grayscaleMatrix.at(2, 1),
        grayscaleMatrix.at(2, 2),
        0,
        0,
        0,
        0,
        0,
        1,
        0,
      };

      return FEColorMatrix::Create(ColorMatrixType::FECOLORMATRIX_TYPE_MATRIX, krys::move(inputParameters));
    }

    KRYS_NODISCARD RefPtr<FilterEffect>
      CreateHueRotateEffect(const BasicColorMatrixFilterOperation &colorMatrixOperation) noexcept
    {
      List<float> inputParameters {static_cast<float>(colorMatrixOperation.Amount())};
      return FEColorMatrix::Create(ColorMatrixType::FECOLORMATRIX_TYPE_HUEROTATE,
                                   krys::move(inputParameters));
    }

    KRYS_NODISCARD RefPtr<FilterEffect>
      CreateInvertEffect(const BasicComponentTransferFilterOperation &componentTransferOperation) noexcept
    {
      auto amount = static_cast<float>(componentTransferOperation.Amount());
      return FEColorMatrix::Create(ColorMatrixType::FECOLORMATRIX_TYPE_MATRIX,
                                   ValuesOf(InvertColourMatrix(amount)));
    }

    KRYS_NODISCARD RefPtr<FilterEffect>
      CreateOpacityEffect(const BasicComponentTransferFilterOperation &componentTransferOperation) noexcept
    {
      auto amount = static_cast<float>(componentTransferOperation.Amount());
      return FEColorMatrix::Create(ColorMatrixType::FECOLORMATRIX_TYPE_MATRIX,
                                   ValuesOf(OpacityColourMatrix(amount)));
    }

    KRYS_NODISCARD RefPtr<FilterEffect>
      CreateSaturateEffect(const BasicColorMatrixFilterOperation &colorMatrixOperation) noexcept
    {
      List<float> inputParameters {static_cast<float>(colorMatrixOperation.Amount())};
      return FEColorMatrix::Create(ColorMatrixType::FECOLORMATRIX_TYPE_SATURATE, krys::move(inputParameters));
    }

    KRYS_NODISCARD RefPtr<FilterEffect>
      CreateSepiaEffect(const BasicColorMatrixFilterOperation &colorMatrixOperation) noexcept
    {
      auto sepiaMatrix = SepiaColourMatrix(static_cast<float>(colorMatrixOperation.Amount()));
      List<float> inputParameters {
        sepiaMatrix.at(0, 0),
        sepiaMatrix.at(0, 1),
        sepiaMatrix.at(0, 2),
        0,
        0,
        sepiaMatrix.at(1, 0),
        sepiaMatrix.at(1, 1),
        sepiaMatrix.at(1, 2),
        0,
        0,
        sepiaMatrix.at(2, 0),
        sepiaMatrix.at(2, 1),
        sepiaMatrix.at(2, 2),
        0,
        0,
        0,
        0,
        0,
        1,
        0,
      };

      return FEColorMatrix::Create(ColorMatrixType::FECOLORMATRIX_TYPE_MATRIX, krys::move(inputParameters));
    }
  }

  template <typename TFilter>
  RefPtr<CSSFilterRenderer> CSSFilterRenderer::CreateGeneric(
    RenderElement &renderer, const TFilter &filter, const FilterGeometry &geometry,
    EnumSet<FilterRenderingMode> preferredRenderingModes, const GraphicsContext &destinationContext) noexcept
  {
    bool hasFilterThatMovesPixels = filter.HasFilterThatMovesPixels();
    bool hasFilterThatShouldBeRestrictedBySecurityOrigin =
      filter.HasFilterThatShouldBeRestrictedBySecurityOrigin();

    auto filterRenderer = AdoptRef(*new CSSFilterRenderer(geometry, hasFilterThatMovesPixels,
                                                          hasFilterThatShouldBeRestrictedBySecurityOrigin));

    if (!filterRenderer->BuildFilterFunctions(renderer, filter, preferredRenderingModes, destinationContext))
    {
      return nullptr;
    }

    filterRenderer->SetFilterRenderingModes(preferredRenderingModes);
    // Without a mode the page prefers (there's no software rendering to fall back on), the filter can't be
    // applied.
    if (filterRenderer->FilterRenderingModes().IsEmpty())
    {
      return nullptr;
    }

    return filterRenderer;
  }

  RefPtr<CSSFilterRenderer> CSSFilterRenderer::Create(RenderElement &renderer, const style::Filter &filter,
                                                      const FilterGeometry &geometry,
                                                      EnumSet<FilterRenderingMode> preferredRenderingModes,
                                                      const GraphicsContext &destinationContext) noexcept
  {
    return CreateGeneric(renderer, filter, geometry, preferredRenderingModes, destinationContext);
  }

  RefPtr<CSSFilterRenderer> CSSFilterRenderer::Create(RenderElement &renderer,
                                                      const FilterOperations &operations,
                                                      const FilterGeometry &geometry,
                                                      EnumSet<FilterRenderingMode> preferredRenderingModes,
                                                      const GraphicsContext &destinationContext) noexcept
  {
    return CreateGeneric(renderer, operations, geometry, preferredRenderingModes, destinationContext);
  }

  Ref<CSSFilterRenderer>
    CSSFilterRenderer::Create(List<Ref<FilterFunction>> &&functions, const FilterGeometry &geometry,
                              EnumSet<FilterRenderingMode> preferredRenderingModes) noexcept
  {
    auto filter = AdoptRef(*new CSSFilterRenderer(krys::move(functions), geometry));
    // Setting filter rendering modes cannot be moved to the constructor because it ends up
    // calling supportedFilterRenderingModes() which is a virtual function.
    filter->SetFilterRenderingModes(preferredRenderingModes);
    return filter;
  }

  CSSFilterRenderer::CSSFilterRenderer(const FilterGeometry &geometry, bool hasFilterThatMovesPixels,
                                       bool hasFilterThatShouldBeRestrictedBySecurityOrigin) noexcept
      : Filter(Filter::Type::CSSFilterRenderer, geometry),
        _hasFilterThatMovesPixels(hasFilterThatMovesPixels),
        _hasFilterThatShouldBeRestrictedBySecurityOrigin(hasFilterThatShouldBeRestrictedBySecurityOrigin)
  {
  }

  CSSFilterRenderer::CSSFilterRenderer(List<Ref<FilterFunction>> &&functions,
                                       const FilterGeometry &geometry) noexcept
      : Filter(Type::CSSFilterRenderer, geometry), _functions(krys::move(functions))
  {
    ClampFilterRegionIfNeeded();
  }

  RefPtr<FilterFunction> CSSFilterRenderer::BuildFilterFunction(RenderElement &renderer,
                                                                const FilterOperation &operation,
                                                                EnumSet<FilterRenderingMode>,
                                                                const GraphicsContext &) noexcept
  {
    switch (operation.GetType())
    {
      case FilterOperation::Type::AppleInvertLightness:
        krys_debug_assert(false); // AppleInvertLightness is only used in -apple-color-filter.
        break;

      case FilterOperation::Type::Blur: return CreateBlurEffect(Downcast<BlurFilterOperation>(operation));

      case FilterOperation::Type::Brightness:
        return CreateBrightnessEffect(Downcast<BasicComponentTransferFilterOperation>(operation));

      case FilterOperation::Type::Contrast:
        return CreateContrastEffect(Downcast<BasicComponentTransferFilterOperation>(operation));

      case FilterOperation::Type::DropShadow:
        return CreateDropShadowEffect(Downcast<DropShadowFilterOperation>(operation));

      case FilterOperation::Type::DropShadowWithStyleColor:
        return CreateDropShadowEffect(Downcast<style::DropShadowFilterOperationWithStyleColor>(operation),
                                      renderer.Style());

      case FilterOperation::Type::Grayscale:
        return CreateGrayScaleEffect(Downcast<BasicColorMatrixFilterOperation>(operation));

      case FilterOperation::Type::HueRotate:
        return CreateHueRotateEffect(Downcast<BasicColorMatrixFilterOperation>(operation));

      case FilterOperation::Type::Invert:
        return CreateInvertEffect(Downcast<BasicComponentTransferFilterOperation>(operation));

      case FilterOperation::Type::Opacity:
        return CreateOpacityEffect(Downcast<BasicComponentTransferFilterOperation>(operation));

      case FilterOperation::Type::Saturate:
        return CreateSaturateEffect(Downcast<BasicColorMatrixFilterOperation>(operation));

      case FilterOperation::Type::Sepia:
        return CreateSepiaEffect(Downcast<BasicColorMatrixFilterOperation>(operation));

      case FilterOperation::Type::Reference:
        // FIXME: Reference filters (createReferenceFilter, SVGFilterRenderer), once there's SVG.
        break;

      default: break;
    }

    return nullptr;
  }

  template <typename TFilter>
  bool CSSFilterRenderer::BuildFilterFunctions(RenderElement &renderer, const TFilter &filter,
                                               EnumSet<FilterRenderingMode> preferredRenderingModes,
                                               const GraphicsContext &destinationContext) noexcept
  {
    for (const auto &value : filter)
    {
      auto function =
        BuildFilterFunction(renderer, OperationOf(value), preferredRenderingModes, destinationContext);
      if (!function)
      {
        continue;
      }

      if (_functions.empty())
      {
        _functions.push_back(SourceGraphic::Create());
      }

      _functions.push_back(krys::move(function));
    }

    // If we didn't make any effects, tell our caller we are not valid.
    if (_functions.empty())
    {
      return false;
    }

    _functions.shrink_to_fit();
    return true;
  }

  FilterEffectVector CSSFilterRenderer::EffectsOfType(FilterFunction::Type filterType) const noexcept
  {
    FilterEffectVector effects;

    for (const auto &function : _functions)
    {
      if (function->FilterType() == filterType)
      {
        effects.push_back(ShareRef(Downcast<FilterEffect>(*function)));
        continue;
      }

      // FIXME: The effects of reference filters (SVGFilterRenderer), once there's SVG.
    }

    return effects;
  }

  EnumSet<FilterRenderingMode> CSSFilterRenderer::SupportedFilterRenderingModes(
    EnumSet<FilterRenderingMode> preferredFilterRenderingModes) const noexcept
  {
    EnumSet<FilterRenderingMode> modes = AllFilterRenderingModes;

    for (const auto &function : _functions)
    {
      modes = modes & function->SupportedFilterRenderingModes(preferredFilterRenderingModes);
    }

    return modes;
  }

  FilterStyleVector CSSFilterRenderer::CreateFilterStyles(GraphicsContext &context,
                                                          const FilterStyle &sourceStyle) const noexcept
  {
    krys_debug_assert(FilterRenderingModes().Contains(FilterRenderingMode::GraphicsContext));

    FilterStyleVector styles;
    FilterStyle lastStyle = sourceStyle;

    for (const auto &function : _functions)
    {
      if (function->FilterType() == FilterEffect::Type::SourceGraphic)
      {
        continue;
      }

      auto result = function->CreateFilterStyles(context, *this, lastStyle);
      if (result.empty())
      {
        return {};
      }

      lastStyle = result.back();
      styles.insert(styles.end(), result.begin(), result.end());
    }

    return styles;
  }

  void CSSFilterRenderer::SetFilterRegion(const FloatRect &filterRegion) noexcept
  {
    Filter::SetFilterRegion(filterRegion);
    ClampFilterRegionIfNeeded();
  }

  bool CSSFilterRenderer::IsIdentity(RenderElement &, const style::Filter &filter) noexcept
  {
    if (filter.HasFilterThatShouldBeRestrictedBySecurityOrigin())
    {
      return false;
    }

    for (const auto &value : filter)
    {
      const auto &operation = value.Get();
      // FIXME: A reference filter is the identity when its SVG filter is (isIdentityReferenceFilter), once
      // there's SVG; until then it applies nothing.
      if (operation.GetType() == FilterOperation::Type::Reference)
      {
        continue;
      }

      if (!operation.IsIdentity())
      {
        return false;
      }
    }

    return true;
  }

  bool CSSFilterRenderer::IsIdentity(RenderElement &, const FilterOperations &operations) noexcept
  {
    if (operations.HasFilterThatShouldBeRestrictedBySecurityOrigin())
    {
      return false;
    }

    for (const auto &operation : operations)
    {
      // FIXME: Reference filters, as above.
      if (operation->GetType() == FilterOperation::Type::Reference)
      {
        continue;
      }

      if (!operation->IsIdentity())
      {
        return false;
      }
    }

    return true;
  }

  IntOutsets CSSFilterRenderer::CalculateOutsets(RenderElement &, const style::Filter &filter,
                                                 const FloatRect &) noexcept
  {
    IntOutsets outsets;

    for (const auto &value : filter)
    {
      // FIXME: A reference filter's outsets (calculateReferenceFilterOutsets), once there's SVG.
      outsets += value.Get().Outsets();
    }

    return outsets;
  }

  IntOutsets CSSFilterRenderer::CalculateOutsets(RenderElement &, const FilterOperations &operations,
                                                 const FloatRect &) noexcept
  {
    IntOutsets outsets;

    for (const auto &operation : operations)
    {
      // FIXME: A reference filter's outsets, as above.
      outsets += operation->Outsets();
    }

    return outsets;
  }

  TextStream &CSSFilterRenderer::ExternalRepresentation(TextStream &ts,
                                                        FilterRepresentation representation) const noexcept
  {
    int32 level = 0;

    for (auto it = _functions.rbegin(), end = _functions.rend(); it != end; ++it)
    {
      const auto &function = *it;

      // SourceAlpha is a built-in effect. No need to say SourceGraphic is its input.
      if (function->FilterType() == FilterEffect::Type::SourceAlpha)
      {
        ++it;
      }

      TextStream::IndentScope indentScope(ts, level++);
      function->ExternalRepresentation(ts, representation);
    }

    return ts;
  }
}
