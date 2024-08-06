#ifndef __EV_EXPOSE_H__
#define __EV_EXPOSE_H__

#if defined(_WIN32) || defined(__CYGWIN__)
#   if defined(EV_USE_DLL)
#       if defined(EV_BUILDING_DLL)
#           if defined(__GNUC__) || defined(__clang__)
#               define EV_API   __attribute__ ((dllexport))
#           else
#               define EV_API   __declspec(dllexport)
#           endif
#       else
#           if defined(__GNUC__) || defined(__clang__)
#               define EV_API   __attribute__ ((dllimport))
#           else
#               define EV_API   __declspec(dllimport)
#           endif
#       endif
#   else
#       define EV_API
#   endif
#elif (defined(__GNUC__) && __GNUC__ >= 4) || defined(__clang__)
#   define EV_API __attribute__((visibility ("default")))
#else
#   define EV_API
#endif

#if defined(EV_AMALGAMATE_BUILD)
#   if defined(__GNUC__) || defined(__clang__)
#       define EV_LOCAL static __attribute__((unused))
#   else
#       define EV_LOCAL static
#   endif
#elif (defined(__GNUC__) || defined(__clang__)) && !defined(_WIN32)
#   define EV_LOCAL __attribute__((visibility ("hidden")))
#else
#   define EV_LOCAL
#endif

#endif
