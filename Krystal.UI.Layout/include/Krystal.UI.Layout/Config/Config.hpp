#pragma once

#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"
#include <bitset>

#include "Krystal.UI.Layout/Api/Forward.hpp"
#include "Krystal.UI.Styles/Enums/Errata.hpp"
#include "Krystal.UI.Styles/Enums/ExperimentalFeature.hpp"

namespace Krys::UI::Layout
{

  struct Config
  {
  private:
    bool _useWebDefaults : 1 = false;
    Styles::ExperimentalFeature _experimentalFeatures
        : BitCount<Styles::ExperimentalFeature>() = Styles::ExperimentalFeature::None;
    Styles::Errata _errata = Styles::Errata::None;
    uint16 _version = 0;
    float _pointScaleFactor = 1.0f;
    CloneNodeFunc _cloneNodeCallback {nullptr};
    void *_context = nullptr;

  public:
    Config() = default;

    void SetUseWebDefaults(bool useWebDefaults);
    bool UseWebDefaults() const;

    void SetExperimentalFeatureEnabled(Styles::ExperimentalFeature feature, bool enabled);
    bool IsExperimentalFeatureEnabled(Styles::ExperimentalFeature feature) const;
    Styles::ExperimentalFeature GetEnabledExperiments() const;

    void SetErrata(Styles::Errata errata);
    void AddErrata(Styles::Errata errata);
    void RemoveErrata(Styles::Errata errata);
    Styles::Errata GetErrata() const;
    bool HasErrata(Styles::Errata errata) const;

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