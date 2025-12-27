#pragma once

#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include <bitset>

#include "Krystal.UI/Layout/Api/Forward.hpp"
#include "Krystal.UI/Styles/Enums/Errata.hpp"
#include "Krystal.UI/Styles/Enums/ExperimentalFeature.hpp"

namespace Krys::UI
{

  struct Config
  {
  private:
    bool _useWebDefaults : 1 = false;
    ExperimentalFeature _experimentalFeatures
        : BitCount<ExperimentalFeature>() = ExperimentalFeature::None;
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
    bool IsExperimentalFeatureEnabled(ExperimentalFeature feature) const;
    ExperimentalFeature GetEnabledExperiments() const;

    void SetErrata(Errata errata);
    void AddErrata(Errata errata);
    void RemoveErrata(Errata errata);
    Errata GetErrata() const;
    bool HasErrata(Errata errata) const;

    void SetPointScaleFactor(float pointScaleFactor);
    float GetPointScaleFactor() const;

    void SetContext(void *context);
    void *GetContext() const;

    uint16 GetVersion() const noexcept;

    void SetCloneNodeCallback(CloneNodeFunc cloneNode);
    NodeRef CloneNode(NodeConstRef node, NodeConstRef owner, size_t childIndex) const;

    static const Config &Defaults();
  };

  bool ConfigUpdateInvalidatesLayout(const Config &oldConfig, const Config &newConfig);
}