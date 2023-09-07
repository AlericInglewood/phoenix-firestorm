#pragma once

#ifdef CWDEBUG
#define NAMESPACE_DEBUG debug
#define NAMESPACE_DEBUG_START namespace debug {
#define NAMESPACE_DEBUG_END }
#endif

#include "../cwds/debug.h"

#ifdef CWDEBUG
NAMESPACE_DEBUG_CHANNELS_START
extern channel_ct viewer;
NAMESPACE_DEBUG_CHANNELS_END
#endif
