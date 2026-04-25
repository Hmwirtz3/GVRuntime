#include "Modules/StaticMesh/StaticMesh.h"
#include "Framework/Chunk/ChunkTypes.h"
#include "Framework/Utils/BinaryReader.h"
#include "Framework/MessageHandler/MessageHandler.h"

#include <cstdlib>
#include <cstring>
#include <algorithm> // <-- added

namespace GV
{
    static std::vector<StaticMeshInstance> g_meshes;
    static bool g_sorted = false;

    static void Align16(const uint8_t*& ptr)
    {
        ptr = (const uint8_t*)(((uintptr_t)ptr + 15) & ~15);
    }

    // --------------------------------------------------
    // NEW: Sort meshes largest → smallest (by vertex count)
    // --------------------------------------------------
    static void SortMeshesBySize()
    {
        std::sort(g_meshes.begin(), g_meshes.end(),
            [](const StaticMeshInstance& a, const StaticMeshInstance& b)
            {
                uint32_t sizeA = 0;
                uint32_t sizeB = 0;

                for (const auto& sm : a.submeshes)
                    sizeA += sm.vertexCount;

                for (const auto& sm : b.submeshes)
                    sizeB += sm.vertexCount;

                return sizeA > sizeB;
            });
    }

    void StaticMesh::Load(
        const std::vector<uint8_t>& bytes,
        uint32_t start,
        uint32_t end)
    {
        const uint8_t* base   = bytes.data();
        const uint8_t* ptr    = base + start;
        const uint8_t* endPtr = base + end;

        ptr += sizeof(GV_ChunkHeader) + 4;

        const GV_ChunkHeader* outerHeader = (const GV_ChunkHeader*)ptr;
        ptr += sizeof(GV_ChunkHeader) + 4;

        uint32_t paramCount = ReadUInt32(ptr);

        StaticMeshInstance mesh{};

        if (paramCount >= 9)
        {
            mesh.posX = ReadFloat(ptr);
            mesh.posY = ReadFloat(ptr);
            mesh.posZ = ReadFloat(ptr);

            mesh.rotX = ReadFloat(ptr);
            mesh.rotY = ReadFloat(ptr);
            mesh.rotZ = ReadFloat(ptr);

            mesh.scaleX = ReadFloat(ptr);
            mesh.scaleY = ReadFloat(ptr);
            mesh.scaleZ = ReadFloat(ptr);
        }
        else
        {
            return;
        }

        Align16(ptr);

        if (ptr + sizeof(GV_ChunkHeader) > endPtr)
        {
            return;
        }

        const GV_ChunkHeader* innerWrapper = (const GV_ChunkHeader*)ptr;

        if (innerWrapper->type != GV_CHUNK_STATIC_MESH)
        {
            return;
        }

        const uint8_t* innerStart = ptr;
        ptr += sizeof(GV_ChunkHeader) + 4;
        const uint8_t* innerEnd = innerStart + innerWrapper->size;

        uint32_t currentVertexCount = 0;
        uint32_t currentTextureID = 0;

        PSPVertex* pendingVerts = nullptr;
        uint32_t pendingCount = 0;

        while (ptr < innerEnd && ptr < endPtr)
        {
            if (ptr + sizeof(GV_ChunkHeader) > innerEnd)
            {
                break;
            }

            const GV_ChunkHeader* h = (const GV_ChunkHeader*)ptr;
            ptr += sizeof(GV_ChunkHeader) + 4;

            uint32_t payloadSize = h->size - sizeof(GV_ChunkHeader);
            const uint8_t* payloadStart = ptr;
            const uint8_t* nextChunk = payloadStart + payloadSize;

            switch (h->type)
            {
                case GV_CHUNK_STRUCT:
                {
                    ptr += 4;
                    currentVertexCount = *(const uint32_t*)ptr; ptr += 4;
                    ptr += 8;
                    break;
                }

                case GV_CHUNK_GEOMETRY:
                {
                    size_t size = currentVertexCount * sizeof(PSPVertex);

                    PSPVertex* dst = (PSPVertex*)malloc(size);
                    if (!dst)
                    {
                        return;
                    }

                    memcpy(dst, ptr, size);

                    pendingVerts = dst;
                    pendingCount = currentVertexCount;

                    ptr += size;
                    break;
                }

                case GV_CHUNK_MATERIAL:
                {
                    currentTextureID = *(const uint32_t*)ptr;
                    ptr += 4;

                    if (pendingVerts)
                    {
                        StaticSubmesh sm{};
                        sm.vertices = pendingVerts;
                        sm.vertexCount = pendingCount;
                        sm.textureID = currentTextureID;

                        mesh.submeshes.push_back(sm);

                        pendingVerts = nullptr;
                        pendingCount = 0;
                    }
                    break;
                }

                default:
                {
                    break;
                }
            }

            ptr = nextChunk;
            Align16(ptr);
        }

        g_meshes.push_back(mesh);
    }

    // --------------------------------------------------
    // NEW: Sort happens ONCE after all meshes are loaded
    // --------------------------------------------------
    void StaticMesh::Finalize()
    {
        SortMeshesBySize();
    }

    void StaticMesh::HandleMessage(uint32_t index, const std::string& msg)
    {
        if (index >= g_meshes.size())
            return;

        StaticMeshInstance& mesh = g_meshes[index];
        mesh.visible = true;
    }

    void StaticMesh::BuildRenderData(uint32_t meshIndex, uint32_t submeshIndex, void* dst)
    {
        const StaticMeshInstance& mesh = g_meshes[meshIndex];
        const StaticSubmesh& sm = mesh.submeshes[submeshIndex];

        MeshGpuData* out = (MeshGpuData*)dst;

        PSPVertex* verts = (PSPVertex*)(out + 1);
        size_t size = sm.vertexCount * sizeof(PSPVertex);

        memcpy(verts, sm.vertices, size);

        out->textureID = sm.textureID;
        out->vertexCount = sm.vertexCount;
        out->verts = verts;
    }

   uint32_t StaticMesh::GetCount()
{
    if (!g_sorted)
    {
        SortMeshesBySize();
        g_sorted = true;
    }

    return (uint32_t)g_meshes.size();
}

    const StaticMeshInstance& StaticMesh::Get(uint32_t index)
    {
        return g_meshes[index];
    }
}