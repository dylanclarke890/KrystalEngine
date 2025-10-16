#include "Krystal.UI/Api/Config.hpp"
#include "Krystal.UI/Config/Config.hpp"
#include <cassert>

namespace Krys::UI
{
  ConfigRef ConfigCreate()
  {
    return new Config();
  }

  void ConfigDestroy(ConfigRef config)
  {
    delete config;
  }

  ConfigConstRef ConfigGetDefault()
  {
    return &Config::Defaults();
  }

  void ConfigSetUseWebDefaults(ConfigRef config, bool enabled)
  {
    config->SetUseWebDefaults(enabled);
  }

  bool ConfigGetUseWebDefaults(ConfigConstRef config)
  {
    return config->UseWebDefaults();
  }

  void ConfigSetPointScaleFactor(ConfigRef config, float pixelsInPoint)
  {
    assert(pixelsInPoint >= 0.0f && "Scale factor should not be less than zero");
    config->SetPointScaleFactor(pixelsInPoint);
  }

  float ConfigGetPointScaleFactor(ConfigConstRef config)
  {
    return config->GetPointScaleFactor();
  }

  void ConfigSetErrata(ConfigRef config, Errata errata)
  {
    config->SetErrata(errata);
  }

  Errata ConfigGetErrata(ConfigConstRef config)
  {
    return config->GetErrata();
  }

  void ConfigSetContext(ConfigRef config, void *context)
  {
    config->SetContext(context);
  }

  void *ConfigGetContext(ConfigConstRef config)
  {
    return config->GetContext();
  }

  void ConfigSetExperimentalFeatureEnabled(ConfigRef config, ExperimentalFeature feature, bool enabled)
  {
    config->SetExperimentalFeatureEnabled(feature, enabled);
  }

  bool ConfigIsExperimentalFeatureEnabled(ConfigConstRef config, ExperimentalFeature feature)
  {
    return config->IsExperimentalFeatureEnabled(feature);
  }

  void ConfigSetCloneNodeFunc(ConfigRef config, CloneNodeFunc callback)
  {
    config->SetCloneNodeCallback(callback);
  }
}