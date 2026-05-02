#include "Modules/StaticMesh/StaticMesh.h"
#include "Framework/Chunk/ChunkTypes.h"
#include "Framework/Utils/BinaryReader.h"
#include "Framework/MessageHandler/MessageHandler.h"
#include "Font/FontRenderer.h"

#include <algorithm>
#include <cstring>

static uint32_t g_meshCount = 0;

namespace GV
{
    static std::vector<StaticMeshInstance> g_meshes;
    static bool g_batchesBuilt = false;

    static void Align16(const uint8_t*& ptr, const uint8_t* endPtr)
    {
        uintptr_t p = (uintptr_t)ptr;
        p = (p + 15) & ~15;

        const uint8_t* aligned = (const uint8_t*)p;

        if (aligned <= endPtr)
            ptr = aligned;
    }

    static bool ReadUInt32Safe(const uint8_t*& ptr, const uint8_t* endPtr, uint32_t& out)
    {
        if (ptr + sizeof(uint32_t) > endPtr)
            return false;

        std::memcpy(&out, ptr, sizeof(uint32_t));
        ptr += sizeof(uint32_t);
        return true;
    }

    static bool ReadStringSafe(const uint8_t*& ptr, const uint8_t* endPtr, std::string& out)
    {
        uint32_t len = 0;

        if (!ReadUInt32Safe(ptr, endPtr, len))
            return false;

        if (ptr + len > endPtr)
            return false;

        out.assign((const char*)ptr, len);
        ptr += len;

        return true;
    }

    void StaticMesh::BuildBatches()
    {
        if (g_batchesBuilt)
            return;

        for (auto& mesh : g_meshes)
        {
            mesh.batches.clear();

            if (mesh.submeshes.empty())
                continue;

            std::sort(mesh.submeshes.begin(), mesh.submeshes.end(),
                [](const StaticSubmesh& a, const StaticSubmesh& b)
                {
                    return a.textureID < b.textureID;
                });

            uint32_t start = 0;
            uint32_t currentTex = mesh.submeshes[0].textureID;

            for (uint32_t i = 0; i < mesh.submeshes.size(); i++)
            {
                if (mesh.submeshes[i].textureID != currentTex)
                {
                    StaticBatch batch{};
                    batch.startIndex = start;
                    batch.count = i - start;
                    batch.textureID = currentTex;

                    mesh.batches.push_back(batch);

                    start = i;
                    currentTex = mesh.submeshes[i].textureID;
                }
            }

            StaticBatch batch{};
            batch.startIndex = start;
            batch.count = (uint32_t)mesh.submeshes.size() - start;
            batch.textureID = currentTex;

            mesh.batches.push_back(batch);
        }

        g_batchesBuilt = true;
    }

    void StaticMesh::Finalize()
    {
        g_batchesBuilt = false;
        BuildBatches();
    }

    void StaticMesh::Load(
        const std::vector<uint8_t>& bytes,
        uint32_t start,
        uint32_t end)
    {
        g_batchesBuilt = false;

        const uint8_t* base   = bytes.data();
        const uint8_t* ptr    = base + start;
        const uint8_t* endPtr = base + end;

        if (ptr >= endPtr)
            return;

        ptr += sizeof(GV_ChunkHeader) + 4;
        ptr += sizeof(GV_ChunkHeader) + 4;

        uint32_t paramCount = 0;
        if (!ReadUInt32Safe(ptr, endPtr, paramCount))
            return;

        StaticMeshInstance mesh{};
        mesh.visible = true;

        if (paramCount < 9)
            return;

        mesh.posX = ReadFloat(ptr);
        mesh.posY = ReadFloat(ptr);
        mesh.posZ = ReadFloat(ptr);

        mesh.rotX = ReadFloat(ptr);
        mesh.rotY = ReadFloat(ptr);
        mesh.rotZ = ReadFloat(ptr);

        mesh.scaleX = ReadFloat(ptr);
        mesh.scaleY = ReadFloat(ptr);
        mesh.scaleZ = ReadFloat(ptr);

        if (paramCount > 9)
        {
            if (!ReadStringSafe(ptr, endPtr, mesh.visibilityMessage))
                return;
        }

        Align16(ptr, endPtr);

        const GV_ChunkHeader* innerWrapper = (const GV_ChunkHeader*)ptr;
        if (innerWrapper->type != GV_CHUNK_STATIC_MESH)
            return;

        const uint8_t* innerStart = ptr;
        ptr += sizeof(GV_ChunkHeader) + 4;
        const uint8_t* innerEnd = innerStart + innerWrapper->size;

        uint32_t currentVertexCount = 0;
        uint32_t currentTextureID = 0;

        PSPVertex* pendingVerts = nullptr;
        uint32_t pendingCount = 0;

        while (ptr < innerEnd)
        {
            const GV_ChunkHeader* h = (const GV_ChunkHeader*)ptr;
            ptr += sizeof(GV_ChunkHeader) + 4;

            uint32_t payloadSize = h->size - sizeof(GV_ChunkHeader);
            const uint8_t* nextChunk = ptr + payloadSize;

            switch (h->type)
            {
                case GV_CHUNK_STRUCT:
                    ptr += 4;
                    currentVertexCount = *(const uint32_t*)ptr; ptr += 4;
                    ptr += 8;
                    break;

                case GV_CHUNK_GEOMETRY:
                    pendingVerts = (PSPVertex*)ptr;
                    pendingCount = currentVertexCount;
                    ptr += currentVertexCount * sizeof(PSPVertex);
                    break;

                case GV_CHUNK_MATERIAL:
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

            ptr = nextChunk;
            Align16(ptr, innerEnd);
        }

        uint32_t index = g_meshCount++;

        if (g_meshes.size() <= index)
            g_meshes.resize(index + 1);

        g_meshes[index] = mesh;

        if (!g_meshes[index].visibilityMessage.empty())
        {
            MessageHandler::Register(
                g_meshes[index].visibilityMessage,
                GV_CHUNK_STATIC_MESH,
                index
            );
        }
    }

    void StaticMesh::HandleMessage(uint32_t index, const std::string& msg)
    {
        if (index >= g_meshes.size())
            return;

        StaticMeshInstance& mesh = g_meshes[index];
        mesh.visible = !mesh.visible;
    }

    void StaticMesh::BuildRenderData(
        uint32_t meshIndex,
        uint32_t batchIndex,
        void* dst)
    {
        const StaticMeshInstance& mesh = g_meshes[meshIndex];
        const StaticBatch& batch = mesh.batches[batchIndex];

        MeshGpuData* out = (MeshGpuData*)dst;

        const StaticSubmesh& sm = mesh.submeshes[batch.startIndex];

        out->textureID = batch.textureID;
        out->vertexCount = sm.vertexCount;
        out->verts = sm.vertices;
    }

    uint32_t StaticMesh::GetCount()
    {
        BuildBatches();
        return (uint32_t)g_meshes.size();
    }

    const StaticMeshInstance& StaticMesh::Get(uint32_t index)
    {
        return g_meshes[index];
    }
}