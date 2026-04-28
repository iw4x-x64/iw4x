#pragma once

// Must be included first on MinGW
//
#include <winsock2.h>
#include <ws2tcpip.h>

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#  ifndef NOMINMAX
#    define NOMINMAX
#    include <windows.h>
#    include <psapi.h>
#    undef NOMINMAX
#  else
#    include <windows.h>
#    include <psapi.h>
#  endif
#  undef WIN32_LEAN_AND_MEAN
#else
#  ifndef NOMINMAX
#    define NOMINMAX
#    include <windows.h>
#    include <psapi.h>
#    undef NOMINMAX
#  else
#    include <windows.h>
#    include <psapi.h>
#  endif
#endif

// Linker pseudo-variable.
//
extern "C" IMAGE_DOS_HEADER __ImageBase;
