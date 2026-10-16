#pragma once

#include "Krystal.Booey/Graphics/FontGenericFamilies.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  /// @brief The settings values that style resolution reads.
  struct SettingsValues
  {
    int32 DefaultFontSize {16};
    int32 DefaultFixedFontSize {13};
    int32 MinimumFontSize {0};
    int32 MinimumLogicalFontSize {0};
    bool EvaluationTimeZoomEnabled {false};
    /// @brief Whether characters no family has try the pictograph family before the registered fonts.
    bool FontFallbackPrefersPictographs {false};
    /// @brief The families the generic families resolve to; none by default, so they use the bundled font.
    boo::FontGenericFamilies FontGenericFamilies;
  };

  /// @brief The host's settings for a document: the features without a specification it turns on, the
  /// embedder features and the values style resolution reads. Standard features are always on.
  /// @note Documents share the default settings unless the host gives them their own, and documents a
  /// document creates share its settings. Changing the settings doesn't reparse style sheets that were
  /// already parsed.
  struct Settings : public RefCounted<Settings>
  {
    // Experimental features: these have no specification yet, or are WebKit's own.

    /// @brief `color-layers()`.
    bool CSSColorLayersEnabled {false};
    /// @brief `transform-style: separated`.
    bool CSSTransformStyleSeparatedEnabled {false};
    /// @brief The `::thumb` and `::track` pseudo-elements.
    bool ThumbAndTrackPseudoElementsEnabled {false};
    /// @brief The `spring()` easing function.
    bool SpringTimingFunctionEnabled {false};

    // Embedder features.

    /// @brief `-apple-color-filter`.
    bool ColorFilterEnabled {false};
    /// @brief The system `appearance` values (`default-button` and friends).
    bool UseSystemAppearance {false};

    SettingsValues Values;

    KRYS_NODISCARD static Ref<Settings> Create() noexcept;

    /// @brief The settings documents share unless the host gives them their own.
    KRYS_NODISCARD static Settings &Default() noexcept;
  };
}
