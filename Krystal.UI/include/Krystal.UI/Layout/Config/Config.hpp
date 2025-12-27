#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.UI/Layout/Api/Forward.hpp"
#include "Krystal.UI/Styles/Enums/Errata.hpp"
#include "Krystal.UI/Styles/Enums/ExperimentalFeature.hpp"
#include <bitset>

namespace Krys::UI
{

  struct Config
  {
  private:
    bool _useWebDefaults : 1 = false;
    ExperimentalFeature _experimentalFeatures : BitCount<ExperimentalFeature>() = ExperimentalFeature::None;
    Errata _errata = Errata::None;
    uint16 _version = 0;
    float _pointScaleFactor = 1.0f;
    CloneNodeFunc _cloneNodeCallback {nullptr};
    void *_context = nullptr;

  public:
    Config() = default;

    void SetUseWebDefaults(bool useWebDefaults);
    bool UseWebDefaults() const;

    void SetExperimentalFeatureEnabled(ExperimentalFeature feature, bool enabled);
    KRYS_NODISCARD bool IsExperimentalFeatureEnabled(ExperimentalFeature feature) const;
    KRYS_NODISCARD ExperimentalFeature GetEnabledExperiments() const;

    void SetErrata(Errata errata);
    void AddErrata(Errata errata);
    void RemoveErrata(Errata errata);
    KRYS_NODISCARD Errata GetErrata() const;
    KRYS_NODISCARD bool HasErrata(Errata errata) const;

    void SetPointScaleFactor(float pointScaleFactor);
    KRYS_NODISCARD float GetPointScaleFactor() const;

    void SetContext(void *context);
    KRYS_NODISCARD void *GetContext() const;

    KRYS_NODISCARD uint16 GetVersion() const noexcept;

    void SetCloneNodeCallback(CloneNodeFunc cloneNode);
    KRYS_NODISCARD NodeRef CloneNode(NodeConstRef node, NodeConstRef owner, size_t childIndex) const;

    KRYS_NODISCARD static const Config &Defaults();
  };

  KRYS_NODISCARD bool ConfigUpdateInvalidatesLayout(const Config &oldConfig, const Config &newConfig);
}