#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::html
{
  constexpr int32 UninitializedDescriptor = -1;
  constexpr float DefaultDensityValue = 1.0f;

  /// @brief The descriptors of one image candidate: a density (`x`), or a width (`w`) and an optional
  /// height (`h`).
  class DescriptorParsingResult
  {
  private:
    float _density {UninitializedDescriptor};
    int32 _resourceWidth {UninitializedDescriptor};
    int32 _resourceHeight {UninitializedDescriptor};

  public:
    KRYS_NODISCARD bool HasDensity() const noexcept
    {
      return _density >= 0;
    }
    KRYS_NODISCARD bool HasWidth() const noexcept
    {
      return _resourceWidth >= 0;
    }
    KRYS_NODISCARD bool HasHeight() const noexcept
    {
      return _resourceHeight >= 0;
    }

    KRYS_NODISCARD float Density() const noexcept
    {
      krys_debug_assert(HasDensity());
      return _density;
    }
    KRYS_NODISCARD uint32 ResourceWidth() const noexcept
    {
      krys_debug_assert(HasWidth());
      return static_cast<uint32>(_resourceWidth);
    }
    KRYS_NODISCARD uint32 ResourceHeight() const noexcept
    {
      krys_debug_assert(HasHeight());
      return static_cast<uint32>(_resourceHeight);
    }

    void SetResourceWidth(int32 width) noexcept
    {
      krys_debug_assert(width >= 0);
      _resourceWidth = width;
    }
    void SetResourceHeight(int32 height) noexcept
    {
      krys_debug_assert(height >= 0);
      _resourceHeight = height;
    }
    void SetDensity(float density) noexcept
    {
      krys_debug_assert(density >= 0);
      _density = density;
    }
  };

  /// @brief An image candidate from `srcset` or `src`.
  /// @see https://html.spec.whatwg.org/#image-candidate-string
  struct ImageCandidate
  {
    enum class OriginAttribute : bool
    {
      Srcset,
      Src
    };

    dom::DOMString String;
    float Density {DefaultDensityValue};
    int32 ResourceWidth {UninitializedDescriptor};
    OriginAttribute Origin {OriginAttribute::Srcset};

    ImageCandidate() noexcept = default;
    ImageCandidate(dom::DOMString source, const DescriptorParsingResult &result,
                   OriginAttribute origin) noexcept
        : String(krys::move(source)),
          Density(result.HasDensity() ? result.Density() : UninitializedDescriptor),
          ResourceWidth(result.HasWidth() ? static_cast<int32>(result.ResourceWidth())
                                          : UninitializedDescriptor),
          Origin(origin)
    {
    }

    KRYS_NODISCARD bool SrcOrigin() const noexcept
    {
      return Origin == OriginAttribute::Src;
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return String.empty();
    }
  };

  /// @brief Picks the candidate from @p srcAttribute and @p srcsetAttribute that best fits the device scale
  /// factor, with `w` descriptors resolved against @p sourceSize.
  /// @param srcAttribute The `src` attribute, or null if it's absent.
  /// @param srcsetAttribute The `srcset` attribute, or null if it's absent.
  /// @see https://html.spec.whatwg.org/#select-an-image-source
  KRYS_NODISCARD ImageCandidate BestFitSourceForImageAttributes(
    float deviceScaleFactor, Maybe<dom::DOMStringView> srcAttribute,
    Maybe<dom::DOMStringView> srcsetAttribute, float sourceSize,
    const Func<bool(const ImageCandidate &)> &shouldIgnoreCandidateCallback = {}) noexcept;

  /// @see https://html.spec.whatwg.org/#parse-a-srcset-attribute
  KRYS_NODISCARD List<ImageCandidate>
    ParseImageCandidatesFromSrcsetAttribute(dom::DOMStringView attribute) noexcept;
}
