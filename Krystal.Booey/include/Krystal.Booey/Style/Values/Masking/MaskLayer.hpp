#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/BackgroundSize.hpp"
#include "Krystal.Booey/Style/Values/Backgrounds/RepeatStyle.hpp"
#include "Krystal.Booey/Style/Values/Images/ImageOrNone.hpp"
#include "Krystal.Booey/Style/Values/Masking/MaskMode.hpp"
#include "Krystal.Booey/Style/Values/Primitives/CoordinatedValueListValue.hpp"
#include "Krystal.Booey/Style/Values/Primitives/Position.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include <tuple>

namespace krys::boo::style
{
  // macro(ownerType, property, type, lowercaseName, uppercaseName)

  // clang-format off
#define KRYS_FOR_EACH_MASK_LAYER_REFERENCE(macro)                                \
  macro(MaskLayer, MaskImage, ImageOrNone, image, Image)                         \
  macro(MaskLayer, WebkitMaskPositionX, PositionX, positionX, PositionX)         \
  macro(MaskLayer, WebkitMaskPositionY, PositionY, positionY, PositionY)         \
  macro(MaskLayer, MaskSize, BackgroundSize, size, Size)                         \
  macro(MaskLayer, MaskRepeat, RepeatStyle, repeat, Repeat)

#define KRYS_FOR_EACH_MASK_LAYER_ENUM(macro)                                     \
  macro(MaskLayer, MaskClip, FillBox, clip, Clip)                                \
  macro(MaskLayer, MaskOrigin, FillBox, origin, Origin)                          \
  macro(MaskLayer, MaskComposite, CompositeOperator, composite, Composite)       \
  macro(MaskLayer, MaskMode, MaskMode, maskMode, MaskMode)

#define KRYS_FOR_EACH_MASK_LAYER_SHORTHAND(macro) \
  macro(MaskLayer, MaskPosition, Position, position, Position)

#define KRYS_FOR_EACH_MASK_LAYER_PROPERTY(macro) \
  KRYS_FOR_EACH_MASK_LAYER_REFERENCE(macro)      \
  KRYS_FOR_EACH_MASK_LAYER_ENUM(macro)
  // clang-format on

  /// @brief One layer of an element's mask: the `mask-*` values at one index.
  /// @note The generated `FillRepeat()` setter hides that type inside the class.
  /// FIXME: `hasOpaqueImage` comes with the renderer (it asks the image whether it's known to be opaque).
  struct MaskLayer
  {
  private:
    ImageOrNone _image;
    PositionX _positionX;
    PositionY _positionY;
    BackgroundSize _size;
    RepeatStyle _repeat;
    uint8 _clip : FillBoxBitWidth;
    uint8 _origin : FillBoxBitWidth;
    uint8 _composite : 4;
    uint8 _maskMode : 2;
    mutable uint8 _clipMax : FillBoxBitWidth; // maximum _clip value from this to bottom layer
    KRYS_FOR_EACH_MASK_LAYER_PROPERTY(KRYS_DECLARE_COORDINATED_VALUE_LIST_IS_SET_AND_IS_FILLED_MEMBERS)

  public:
    MaskLayer() noexcept;
    MaskLayer(css::keywords::None) noexcept;
    MaskLayer(ImageOrNone &&) noexcept;
    MaskLayer(RefPtr<StyleImage> &&) noexcept;

    KRYS_NODISCARD const ImageOrNone &GetImage() const noexcept
    {
      return _image;
    }
    KRYS_NODISCARD const PositionX &GetPositionX() const noexcept
    {
      return _positionX;
    }
    KRYS_NODISCARD const PositionY &GetPositionY() const noexcept
    {
      return _positionY;
    }
    KRYS_NODISCARD const BackgroundSize &GetSize() const noexcept
    {
      return _size;
    }
    KRYS_NODISCARD const RepeatStyle &GetRepeat() const noexcept
    {
      return _repeat;
    }
    KRYS_NODISCARD FillBox GetClip() const noexcept
    {
      return static_cast<FillBox>(_clip);
    }
    KRYS_NODISCARD FillBox GetOrigin() const noexcept
    {
      return static_cast<FillBox>(_origin);
    }
    KRYS_NODISCARD CompositeOperator GetComposite() const noexcept
    {
      return static_cast<CompositeOperator>(_composite);
    }
    KRYS_NODISCARD MaskMode GetMaskMode() const noexcept
    {
      return static_cast<MaskMode>(_maskMode);
    }

    KRYS_NODISCARD constexpr static FillAttachment GetAttachment() noexcept
    {
      return FillAttachment::ScrollBackground;
    }
    KRYS_NODISCARD constexpr static BlendMode GetBlendMode() noexcept
    {
      return BlendMode::Normal;
    }

    // https://drafts.fxtf.org/css-masking/#the-mask-composite
    // If there is no further mask layer, the compositing operator must be ignored.
    KRYS_NODISCARD CompositeOperator CompositeForPainting(bool isLastLayer) const noexcept
    {
      if (isLastLayer)
      {
        return CompositeOperator::SourceOver;
      }
      return GetComposite();
    }

    KRYS_NODISCARD static ImageOrNone InitialImage() noexcept
    {
      return css::keywords::None {};
    }
    KRYS_NODISCARD static PositionX InitialPositionX() noexcept
    {
      using namespace css::Literals;
      return 0_css_percentage;
    }
    KRYS_NODISCARD static PositionY InitialPositionY() noexcept
    {
      using namespace css::Literals;
      return 0_css_percentage;
    }
    KRYS_NODISCARD static BackgroundSize InitialSize() noexcept
    {
      return css::keywords::Auto {};
    }
    KRYS_NODISCARD constexpr static RepeatStyle InitialRepeat() noexcept
    {
      return {.Values {boo::FillRepeat::Repeat, boo::FillRepeat::Repeat}};
    }
    KRYS_NODISCARD constexpr static FillBox InitialClip() noexcept
    {
      return FillBox::BorderBox;
    }
    KRYS_NODISCARD constexpr static FillBox InitialOrigin() noexcept
    {
      return FillBox::BorderBox;
    }
    KRYS_NODISCARD constexpr static CompositeOperator InitialComposite() noexcept
    {
      return CompositeOperator::SourceOver;
    }
    KRYS_NODISCARD constexpr static MaskMode InitialMaskMode() noexcept
    {
      return MaskMode::MatchSource;
    }

    KRYS_NODISCARD bool HasImage() const noexcept
    {
      return _image.IsImage();
    }
    KRYS_NODISCARD bool HasRepeatXY() const noexcept
    {
      return GetRepeat() == boo::FillRepeat::Repeat;
    }

    KRYS_NODISCARD bool ClipOccludesNextLayers() const noexcept
    {
      return _clip == _clipMax;
    }
    void SetClipMax(FillBox clipMax) const noexcept
    {
      _clipMax = static_cast<uint8>(clipMax);
    }

    KRYS_NODISCARD bool operator==(const MaskLayer &) const noexcept;

    KRYS_FOR_EACH_MASK_LAYER_REFERENCE(KRYS_DECLARE_COORDINATED_VALUE_LIST_GETTER_AND_SETTERS_REFERENCE)
    KRYS_FOR_EACH_MASK_LAYER_ENUM(KRYS_DECLARE_COORDINATED_VALUE_LIST_GETTER_AND_SETTERS_ENUM)

    // Support for the `mask-position` shorthand.
    KRYS_NODISCARD static Position InitialPosition() noexcept
    {
      return {InitialPositionX(), InitialPositionY()};
    }
    KRYS_NODISCARD Position GetPosition() const noexcept
    {
      return {_positionX, _positionY};
    }
    void SetPosition(Position &&position) noexcept
    {
      SetPositionX(krys::move(position.X));
      SetPositionY(krys::move(position.Y));
    }
    void FillPosition(Position &&position) noexcept
    {
      FillPositionX(krys::move(position.X));
      FillPositionY(krys::move(position.Y));
    }
    void ClearPosition() noexcept
    {
      ClearPositionX();
      ClearPositionY();
    }
    KRYS_NODISCARD bool IsPositionUnset() const noexcept
    {
      return IsPositionXUnset() && IsPositionYUnset();
    }
    KRYS_NODISCARD bool IsPositionSet() const noexcept
    {
      return IsPositionXSet() || IsPositionYSet();
    }
    KRYS_NODISCARD bool IsPositionFilled() const noexcept
    {
      return IsPositionXFilled() || IsPositionYFilled();
    }

    // CoordinatedValueList interface.
    constexpr static auto ComputedValueUsesUsedValues = true;
    constexpr static auto BaseProperty = css::PropertyNameConstant<css::PropertyId::MaskImage> {};
    constexpr static auto Properties =
      std::tuple {KRYS_FOR_EACH_MASK_LAYER_PROPERTY(KRYS_DECLARE_COORDINATED_VALUE_LIST_PROPERTY)};

    KRYS_NODISCARD static MaskLayer Clone(const MaskLayer &other) noexcept
    {
      return other;
    }

    KRYS_NODISCARD bool IsInitial() const noexcept
    {
      return _image.IsNone();
    }

  private:
    // Needed by macros to access members.
    KRYS_NODISCARD MaskLayer &Data() noexcept
    {
      return *this;
    }
    KRYS_NODISCARD const MaskLayer &Data() const noexcept
    {
      return *this;
    }
  };

  KRYS_FOR_EACH_MASK_LAYER_REFERENCE(KRYS_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_REFERENCE)
  KRYS_FOR_EACH_MASK_LAYER_ENUM(KRYS_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_ENUM)
  KRYS_FOR_EACH_MASK_LAYER_SHORTHAND(KRYS_DECLARE_COORDINATED_VALUE_LIST_PROPERTY_ACCESSOR_SHORTHAND)

  // MARK: - Blending

  template <>
  struct Blending<MaskLayer>
  {
    KRYS_NODISCARD bool CanBlend(const MaskLayer &, const MaskLayer &) noexcept;
  };

#undef KRYS_FOR_EACH_MASK_LAYER_REFERENCE
#undef KRYS_FOR_EACH_MASK_LAYER_ENUM
#undef KRYS_FOR_EACH_MASK_LAYER_SHORTHAND
#undef KRYS_FOR_EACH_MASK_LAYER_PROPERTY
}
