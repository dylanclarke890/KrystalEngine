#pragma once

#include "Krystal.UI/Layout/Api/Forward.hpp"
#include "Krystal.UI/Styles/Enums/Errata.hpp"
#include "Krystal.UI/Styles/Enums/ExperimentalFeature.hpp"

namespace Krys::UI
{
  /// @brief Allocates a set of configuration options. The configuration may be applied to multiple nodes
  /// (i.e. a single global config), or can be applied more granularly per-node.
  ConfigRef ConfigCreate();

  /// @brief Frees the associated configuration.
  void ConfigDestroy(ConfigRef config);

  /// @brief Returns the default config values.
  ConfigConstRef ConfigGetDefault();

  /// @brief The engine by default creates new nodes with style defaults different from flexbox on web (e.g.
  /// `FlexDirection::Column` and `PositionType::Relative`). `UseWebDefaults` instructs the engine to instead
  /// use a default style consistent with the web.
  void ConfigSetUseWebDefaults(ConfigRef config, bool enabled);

  /// @brief Whether the configuration is set to use web defaults.
  bool ConfigGetUseWebDefaults(ConfigConstRef config);

  /// @brief The engine will by default round final layout positions and dimensions to the nearest point.
  /// `pointScaleFactor` controls the density of the grid used for layout rounding (e.g. to round to the
  /// closest display pixel). May be set to 0.0f to avoid rounding the layout results.
  void ConfigSetPointScaleFactor(ConfigRef config, float pixelsInPoint);

  /// @brief  Get the currently set point scale factor.
  float ConfigGetPointScaleFactor(ConfigConstRef config);

  /// @brief Configures how tje egmome balances W3C conformance vs compatibility with layouts created against
  /// earlier versions of the engine.
  ///
  /// By default the engine will prioritize W3C conformance. `Errata` may be set to ask the engine to produce
  /// specific incorrect behaviors. E.g. `ConfigSetErrata(config, Errata::StretchFlexBasis)`.
  ///
  /// Errata is a bitmask, and multiple errata may be set at once. Predefined constants exist for convenience:
  /// 1. Errata::None: No errata
  /// 2. Errata::Classic: Match layout behaviors of Yoga 1.x
  /// 3. Errata::All: Match layout behaviors of Yoga 1.x, including `UseLegacyStretchBehaviour`
  void ConfigSetErrata(ConfigRef config, Errata errata);

  /// @brief Get the currently set errata.
  Errata ConfigGetErrata(ConfigConstRef config);

  /// @brief Sets an arbitrary context pointer on the config which may be read from during callbacks.
  void ConfigSetContext(ConfigRef config, void *context);

  /// @brief Gets the currently set context.
  void *ConfigGetContext(ConfigConstRef config);

  /// @brief Enable an experimental/unsupported feature in Yoga.
  void ConfigSetExperimentalFeatureEnabled(ConfigRef config, ExperimentalFeature feature,
                                           bool enabled);

  /// @brief Whether an experimental feature is set.
  bool ConfigIsExperimentalFeatureEnabled(ConfigConstRef config, ExperimentalFeature feature);

  /// @brief Sets a callback, called during layout, to create a new mutable node if the engine must write to
  /// it and its owner is not its parent observed during layout.
  void ConfigSetCloneNodeFunc(ConfigRef config, CloneNodeFunc callback);
}