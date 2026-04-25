#include "Modules/StaticMesh/StaticMesh.h"
#include "Framework/Chunk/ChunkTypes.h"
#include "Framework/Utils/BinaryReader.h"
#include "Framework/MessageHandler/MessageHandler.h"

#include <cstring>

namespace GV
{
    static std::vector<StaticMeshInstance> g_meshes;

    static void Align16(const uint8_t*& ptr, const uint8_t* endPtr)
    {
        uintptr_t p = (uintptr_t)ptr;
        p = (p + 15) & ~15;
        const uint8_t* aligned = (const uint8_t*)p;
        if (aligned <= endPtr)
            ptr = aligned;
    }

    static bool ReadChunkHeaderSafe(const uint8_t*& ptr, const uint8_t* endPtr, GV_ChunkHeader& outHeader)
    {
        if (ptr + sizeof(GV_ChunkHeader) > endPtr)
            return false;

        std::memcpy(&outHeader, ptr, sizeof(GV_ChunkHeader));
        return true;
    }

    static bool ReadUInt32Safe(const uint8_t*& ptr, const uint8_t* endPtr, uint32_t& outValue)
    {
        if (ptr + sizeof(uint32_t) > endPtr)
            return false;

        std::memcpy(&outValue, ptr, sizeof(uint32_t));
        ptr += sizeof(uint32_t);
        return true;
    }

    static bool ReadBytesSafe(const uint8_t*& ptr, const uint8_t* endPtr, void* dst, uint32_t size)
    {
        if (ptr + size > endPtr)
            return false;

        std::memcpy(dst, ptr, size);
        ptr += size;
        return true;
    }

    void StaticMesh::Load(
        const std::vector<uint8_t>& bytes,
        uint32_t start,
        uint32_t end)
    {
        const uint8_t* base   = bytes.data();
        const uint8_t* ptr    = base + start;
        const uint8_t* endPtr = base + end;

        if (start >= bytes.size() || end > bytes.size() || ptr >= endPtr)
            return;

        if (ptr + sizeof(GV_ChunkHeader) + 4 > endPtr)
            return;

        ptr += sizeof(GV_ChunkHeader) + 4;

        GV_ChunkHeader outerHeader{};
        if (!ReadChunkHeaderSafe(ptr, endPtr, outerHeader))
            return;

        if (ptr + sizeof(GV_ChunkHeader) + 4 > endPtr)
            return;

        ptr += sizeof(GV_ChunkHeader) + 4;

        uint32_t paramCount = 0;
        if (!ReadUInt32Safe(ptr, endPtr, paramCount))
            return;

        StaticMeshInstance mesh{};

        if (paramCount >= 9)
        {
            if (ptr + (9 * sizeof(float)) > endPtr)
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
        }
        else
        {
            return;
        }

        Align16(ptr, endPtr);

        if (ptr + sizeof(GV_ChunkHeader) > endPtr)
            return;

        GV_ChunkHeader innerWrapper{};
        if (!ReadChunkHeaderSafe(ptr, endPtr, innerWrapper))
            return;

        if (innerWrapper.type != GV_CHUNK_STATIC_MESH)
            return;

        const uint8_t* innerStart = ptr;
        if (ptr + sizeof(GV_ChunkHeader) + 4 > endPtr)
            return;

        ptr += sizeof(GV_ChunkHeader) + 4;

        const uint8_t* innerEnd = innerStart + innerWrapper.size;
        if (innerEnd > endPtr)
            return;

        uint32_t currentVertexCount = 0;
        uint32_t currentTextureID = 0;

        std::vector<PSPVertex> pendingVerts;

        while (ptr < innerEnd && ptr < endPtr)
        {
            if (ptr + sizeof(GV_ChunkHeader) > innerEnd)
                break;

            GV_ChunkHeader h{};
            if (!ReadChunkHeaderSafe(ptr, innerEnd, h))
                break;

            if (ptr + sizeof(GV_ChunkHeader) + 4 > innerEnd)
                break;

            ptr += sizeof(GV_ChunkHeader) + 4;

            if (h.size < sizeof(GV_ChunkHeader))
                break;

            uint32_t payloadSize = h.size - sizeof(GV_ChunkHeader);
            const uint8_t* payloadStart = ptr;
            const uint8_t* nextChunk = payloadStart + payloadSize;

            if (nextChunk > innerEnd)
                break;

            switch (h.type)
            {
                case GV_CHUNK_STRUCT:
                {
                    if (ptr + 16 > nextChunk)
                        return;

                    ptr += 4;

                    if (!ReadUInt32Safe(ptr, nextChunk, currentVertexCount))
                        return;

                    ptr += 8;
                    break;
                }

                case GV_CHUNK_GEOMETRY:
                {
                    uint32_t bytesNeeded = currentVertexCount * sizeof(PSPVertex);

                    if (ptr + bytesNeeded > nextChunk)
                        return;

                    pendingVerts.resize(currentVertexCount);
                    if (!pendingVerts.empty())
                    {
                        std::memcpy(
                            pendingVerts.data(),
                            ptr,
                            bytesNeeded
                        );
                    }

                    ptr += bytesNeeded;
                    break;
                }

                case GV_CHUNK_MATERIAL:
                {
                    if (!ReadUInt32Safe(ptr, nextChunk, currentTextureID))
                        return;

                    if (!pendingVerts.empty())
                    {
                        StaticSubmesh sm{};
                        sm.vertexCount = (uint32_t)pendingVerts.size();
                        sm.textureID = currentTextureID;

                        PSPVertex* ownedVerts = new PSPVertex[sm.vertexCount];
                        std::memcpy(
                            ownedVerts,
                            pendingVerts.data(),
                            sm.vertexCount * sizeof(PSPVertex)
                        );

                        sm.vertices = ownedVerts;
                        mesh.submeshes.push_back(sm);

                        pendingVerts.clear();
                    }
                    break;
                }

                default:
                    break;
            }

            ptr = nextChunk;
            Align16(ptr, innerEnd);
        }

        g_meshes.push_back(mesh);
    }

    void StaticMesh::HandleMessage(uint32_t index, const std::string& msg)
    {
        if (index >= g_meshes.size())
            return;

        g_meshes[index].visible = true;
    }

    void StaticMesh::BuildRenderData(uint32_t meshIndex, uint32_t submeshIndex, void* dst)
    {
        const StaticMeshInstance& mesh = g_meshes[meshIndex];
        const StaticSubmesh& sm = mesh.submeshes[submeshIndex];

        MeshGpuData* out = (MeshGpuData*)dst;

        PSPVertex* verts = (PSPVertex*)(out + 1);
        uint32_t size = sm.vertexCount * sizeof(PSPVertex);

        std::memcpy(verts, sm.vertices, size);

        out->textureID = sm.textureID;
        out->vertexCount = sm.vertexCount;
        out->verts = verts;
    }

    uint32_t StaticMesh::GetCount()
    {
        return (uint32_t)g_meshes.size();
    }

    const StaticMeshInstance& StaticMesh::Get(uint32_t index)
    {
        return g_meshes[index];
    }
}