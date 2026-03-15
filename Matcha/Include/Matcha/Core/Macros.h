#pragma once

/**
 * @file Macros.h
 * @brief Platform-aware shared library export/import macro.
 */

#if defined(MATCHA_LIB_EXPORT)
    #if defined(_MSC_VER)
        #define MATCHA_EXPORT __declspec(dllexport)
    #elif defined(__GNUC__) || defined(__clang__)
        #define MATCHA_EXPORT __attribute__((visibility("default")))
    #else
        #define MATCHA_EXPORT
    #endif
#else
    #if defined(_MSC_VER)
        #define MATCHA_EXPORT __declspec(dllimport)
    #else
        #define MATCHA_EXPORT
    #endif
#endif
