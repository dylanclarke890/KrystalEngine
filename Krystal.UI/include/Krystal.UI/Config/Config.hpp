#pragma once

#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.Log/ILogger.hpp"
#include <bitset>

#include "Krystal.UI/Enums/Errata.hpp"
#include "Krystal.UI/Enums/ExperimentalFeature.hpp"

namespace Krys::UI
{
  using ExperimentalFeatureSet = std::bitset<OrdinalCount<ExperimentalFeature>()>;

  typedef NodeRef (*CloneNodeFunc)(NodeConstRef oldNode, NodeConstRef owner, size_t childIndex);

  class Config
  {
  private:
    CloneNodeFunc _cloneNodeCallback {nullptr};
    Log::ILogger *_logger {};

    bool _useWebDefaults : 1 = false;

    uint32 _version = 0;
    ExperimentalFeatureSet _experimentalFeatures {};
    Errata _errata = Errata::None;
    float _pointScaleFactor = 1.0f;
    void *_context = nullptr;

  public:
    explicit Config(Log::ILogger *logger) noexcept : _logger(logger)
    {
    }

    void SetUseWebDefaults(bool useWebDefaults) noexcept;
    bool UseWebDefaults() const noexcept;

    void SetExperimentalFeatureEnabled(ExperimentalFeature feature, bool enabled);
    bool IsExperimentalFeatureEnabled(ExperimentalFeature feature) const;
    ExperimentalFeatureSet GetEnabledExperiments() const;

    void SetErrata(Errata errata);
    void AddErrata(Errata errata);
    void RemoveErrata(Errata errata);
    Errata GetErrata() const;
    bool HasErrata(Errata errata) const;

    void SetPointScaleFactor(float pointScaleFactor);
    float GetPointScaleFactor() const;

    void SetContext(void *context);
    void *GetContext() const;

    uint32_t GetVersion() const noexcept;

    void SetCloneNodeCallback(CloneNodeFunc cloneNode);
    NodeRef CloneNode(NodeConstRef node, NodeConstRef owner, size_t childIndex) const;

    static const Config &Defaults();
  }