#pragma once

#ifdef PROFILER_USE_STUB

struct ProfileScope
{
    inline ProfileScope(const char*) {}
    inline ~ProfileScope() {}
};

inline void Profiler_Reset() {}
inline void Profiler_Draw(float, float) {}

#define PROFILE_SCOPE(name) ProfileScope __scope__(name)

#else

#include <pspkernel.h>
#include <cstdio>
#include "Font/FontRenderer.h"

#define MAX_PROFILE_ENTRIES 64

#define PROFILE_FUNCTION() ProfileScope __func_scope__(__FUNCTION__)

struct ProfileEntry
{
    const char* name;
    uint64_t totalTime;
    int calls;
};

inline ProfileEntry g_profiles[MAX_PROFILE_ENTRIES];
inline int g_profileCount = 0;

inline ProfileEntry* Profiler_Get(const char* name)
{
    for (int i = 0; i < g_profileCount; i++)
    {
        if (g_profiles[i].name == name)
            return &g_profiles[i];
    }

    if (g_profileCount < MAX_PROFILE_ENTRIES)
    {
        ProfileEntry& e = g_profiles[g_profileCount++];
        e.name = name;
        e.totalTime = 0;
        e.calls = 0;
        return &e;
    }

    return nullptr;
}

inline void Profiler_Reset()
{
    for (int i = 0; i < g_profileCount; i++)
    {
        g_profiles[i].totalTime = 0;
        g_profiles[i].calls = 0;
    }
}

struct ProfileScope
{
    ProfileEntry* entry;
    uint64_t start;

    inline ProfileScope(const char* name)
    {
        entry = Profiler_Get(name);
        start = sceKernelGetSystemTimeWide();
    }

    inline ~ProfileScope()
    {
        uint64_t end = sceKernelGetSystemTimeWide();

        if (entry)
        {
            entry->totalTime += (end - start);
            entry->calls++;
        }
    }
};

#define PROFILE_SCOPE(name) ProfileScope __scope__(name)

inline void Profiler_Draw(float x, float y)
{
    char buf[128];

    for (int i = 0; i < g_profileCount; i++)
    {
        const ProfileEntry& e = g_profiles[i];

        float ms = (float)e.totalTime / 1000.0f;

        snprintf(buf, sizeof(buf),
            "%s: %.2f ms (%d)",
            e.name,
            ms,
            e.calls);

        DrawText(buf, x, y, 0xFFFFFF00);
        y += 17.0f;
    }
}

#endif