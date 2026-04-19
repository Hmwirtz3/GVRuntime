#include "Modules/StaticMesh/StaticMesh.h"
#include <cstdio>
#include <pspiofilemgr.h>
#include <stdarg.h>

namespace GV
{
    static void WriteLog(const char* fmt, ...)
    {
        SceUID fd = sceIoOpen("STMSHlog.txt", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);
        if (fd < 0) return;

        char buf[256];
        buf[0] = '\0';

        va_list args;
        va_start(args, fmt);
        int len = vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);

        if (len > 0 && len < (int)sizeof(buf))
            sceIoWrite(fd, buf, len);

        sceIoClose(fd);
    }

    void StaticMesh::Load(
        const std::vector<uint8_t>& bytes,
        uint32_t start,
        uint32_t end)
    {
        printf("StaticMesh::Load [%u - %u]\n", start, end);
        WriteLog("[DISPATCH] StaticMesh [%u - %u]\n", start, end);
    }
}