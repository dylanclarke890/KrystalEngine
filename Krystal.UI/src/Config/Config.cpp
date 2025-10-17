#include "Krystal.UI/Config/Config.hpp"
#include "Krystal.UI/Api/Node.hpp"
#include "Krystal.UI/Node/Node.hpp"

namespace Krys::UI
{
  bool ConfigUpdateInvalidatesLayout(const Config &oldConfig, const Config &newConfig)
  {
    return oldConfig.GetErrata() != newConfig.GetErrata()
           || oldConfig.GetEnabledExperiments() != newConfig.GetEnabledExperiments()
           || oldConfig.GetPointScaleFactor() != newConfig.GetPointScaleFactor()
           || oldConfig.UseWebDefaults() != newConfig.UseWebDefaults();
  }

  void Config::SetUseWebDefaults(bool useWebDefaults)
  {
    _useWebDefaults = useWebDefaults;
  }

  bool Config::UseWebDefaults() const
  {
    return _useWebDefaults;
  }

  void Config::SetExperimentalFeatureEnabled(ExperimentalFeature feature, bool enabled)
  {
    if (IsExperimentalFeatureEnabled(feature) != enabled)
    {
      if (enabled)
      {
        _experimentalFeatures = _experimentalFeatures | feature;
      }
      else
      {
        _experimentalFeatures = _experimentalFeatures & (~feature);
      }
      _version++;
    }
  }

  bool Config::IsExperimentalFeatureEnabled(ExperimentalFeature feature) const
  {
    return (_experimentalFeatures & feature) != ExperimentalFeature::None;
  }

  ExperimentalFeature Config::GetEnabledExperiments() const
  {
    return _experimentalFeatures;
  }

  void Config::SetErrata(Errata errata)
  {
    if (_errata != errata)
    {
      _errata = errata;
      _version++;
    }
  }

  void Config::AddErrata(Errata errata)
  {
    if (!HasErrata(errata))
    {
      _errata |= errata;
      _version++;
    }
  }

  void Config::RemoveErrata(Errata errata)
  {
    if (HasErrata(errata))
    {
      _errata &= (~errata);
      _version++;
    }
  }

  Errata Config::GetErrata() const
  {
    return _errata;
  }

  bool Config::HasErrata(Errata errata) const
  {
    return (_errata & errata) != Errata::None;
  }

  void Config::SetPointScaleFactor(float pointScaleFactor)
  {
    if (_pointScaleFactor != pointScaleFactor)
    {
      _pointScaleFactor = pointScaleFactor;
      _version++;
    }
  }

  float Config::GetPointScaleFactor() const
  {
    return _pointScaleFactor;
  }

  void Config::SetContext(void *context)
  {
    _context = context;
  }

  void *Config::GetContext() const
  {
    return _context;
  }

  uint16 Config::GetVersion() const noexcept
  {
    return _version;
  }

  void Config::SetCloneNodeCallback(CloneNodeFunc cloneNode)
  {
    _cloneNodeCallback = cloneNode;
  }

  NodeRef Config::CloneNode(NodeConstRef node, NodeConstRef owner, size_t childIndex) const
  {
    NodeRef clone = nullptr;
    if (_cloneNodeCallback != nullptr)
    {
      clone = _cloneNodeCallback(node, owner, childIndex);
    }
    if (clone == nullptr)
    {
      clone = NodeClone(node);
    }
    return clone;
  }

  const Config &Config::Defaults()
  {
    static Config config {};
    return config;
  }
}