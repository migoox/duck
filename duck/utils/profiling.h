#pragma once

#ifdef ENABLE_PROFILING
#include "tracy/Tracy.hpp"
#define PROFILE_ZONE(name) ZoneScopedN(name)
#else
#define PROFILE_ZONE(name)
#endif
