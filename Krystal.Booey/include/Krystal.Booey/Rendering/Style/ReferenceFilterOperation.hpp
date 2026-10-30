#pragma once

#include "Krystal.Booey/Graphics/Filters/FilterOperation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/URL.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"

namespace krys::boo::style
{
  /// @brief A `url(#filter)` reference to an SVG filter element.
  /// @note FIXME: `loadExternalDocumentIfNeeded` and `cachedSVGDocumentReference` (filters in other
  /// documents).
  class ReferenceFilterOperation final : public FilterOperation
  {
  private:
    URL _url;
    StringAtom _fragment;

  public:
    KRYS_NODISCARD static Ref<ReferenceFilterOperation> Create(URL url, StringAtom fragment) noexcept
    {
      return AdoptRef(*new ReferenceFilterOperation(krys::move(url), krys::move(fragment)));
    }

    ~ReferenceFilterOperation() noexcept override;

    KRYS_NODISCARD Ref<FilterOperation> Clone() const noexcept final
    {
      // Reference filters cannot be cloned.
      krys_unreachable();
    }

    KRYS_NODISCARD bool AffectsOpacity() const noexcept override
    {
      return true;
    }
    KRYS_NODISCARD bool MovesPixels() const noexcept override
    {
      return true;
    }
    // FIXME: This only needs to return true for graphs that include ConvolveMatrix, DisplacementMap,
    // Morphology and possibly Lighting. https://bugs.webkit.org/show_bug.cgi?id=171753
    KRYS_NODISCARD bool ShouldBeRestrictedBySecurityOrigin() const noexcept override
    {
      return true;
    }

    KRYS_NODISCARD const URL &Url() const noexcept
    {
      return _url;
    }
    KRYS_NODISCARD const StringAtom &Fragment() const noexcept
    {
      return _fragment;
    }

  private:
    ReferenceFilterOperation(URL &&, StringAtom &&) noexcept;

    KRYS_NODISCARD bool Equals(const FilterOperation &) const noexcept override;
    KRYS_NODISCARD bool IsIdentity() const noexcept override;
    KRYS_NODISCARD IntOutsets Outsets() const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_OPERATION(krys::boo::style::ReferenceFilterOperation,
                                             GetType() == krys::boo::FilterOperation::Type::Reference)
