#pragma once

/// @brief Check if a particular environment is being used (DEV|STAGE|PROD).
#define KRYS_ENV(ENV) (defined KRYS_ENV_##ENV && KRYS_ENV_##ENV)

// Default to development environment if none is specified.
#if !KRYS_ENV(DEV) && !KRYS_ENV(STAGE) && !KRYS_ENV(PROD)
  #define KRYS_ENV_DEV 1
#endif
