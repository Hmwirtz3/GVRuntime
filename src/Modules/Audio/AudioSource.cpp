#include "Modules/Audio/AudioSource.h"
#include "Font/FontRenderer.h"
#include "Framework/Utils/BinaryReader.h"
#include "Framework/Chunk/ChunkTypes.h"
#include "Modules/Camera/CameraSystem.h"

#include <pspaudio.h>
#include <pspmp3.h>
#include <psputility_avmodules.h>

#include <cmath>
#include <cstring>
#include <malloc.h>

namespace GV
{
    AudioSourceInstance AudioSource::s_instances[32];
    int AudioSource::s_count = 0;

    static const uint32_t AUDIO_SAMPLES = 1024;
    static const uint32_t MP3_PCM_SAMPLES = 1152 * 2;
    static const uint32_t MP3_STREAM_BUFFER_SIZE = 32 * 1024;
    static const uint32_t MP3_PCM_BUFFER_SIZE = 1152 * 2 * 2;

    static int s_mixChannel = -1;
    static bool s_mp3ModulesLoaded = false;

    static int32_t s_mixAccum[AUDIO_SAMPLES * 2] __attribute__((aligned(64)));
    static int16_t s_buffer[AUDIO_SAMPLES * 2] __attribute__((aligned(64)));

    static float hpInL[32];
    static float hpInR[32];
    static float hpOutL[32];
    static float hpOutR[32];

    static int16_t s_mp3Pending[32][MP3_PCM_SAMPLES] __attribute__((aligned(64)));
    static uint32_t s_mp3PendingSamples[32];
    static uint32_t s_mp3PendingCursor[32];

    static uint8_t* s_mp3StreamBuffer[32];
    static uint8_t* s_mp3PCMBuffer[32];
    static uint32_t s_mp3ReadPos[32];
    static uint32_t s_mp3StreamStart[32];

    static void Align16(const uint8_t*& ptr)
    {
        ptr = (const uint8_t*)(((uintptr_t)ptr + 15) & ~15);
    }

    static float Clamp01(float v)
    {
        if (v < 0.0f)
            return 0.0f;

        if (v > 1.0f)
            return 1.0f;

        return v;
    }

    static bool IsMP3Data(const uint8_t* data, uint32_t size)
    {
        if (!data || size < 3)
            return false;

        if (data[0] == 'I' && data[1] == 'D' && data[2] == '3')
            return true;

        if (size >= 2 && data[0] == 0xFF && ((data[1] & 0xE0) == 0xE0))
            return true;

        return false;
    }

    static uint32_t GetMP3StreamStart(const uint8_t* data, uint32_t size)
    {
        if (!data || size < 10)
            return 0;

        if (data[0] != 'I' || data[1] != 'D' || data[2] != '3')
            return 0;

        uint32_t tagSize =
            ((uint32_t)(data[6] & 0x7F) << 21) |
            ((uint32_t)(data[7] & 0x7F) << 14) |
            ((uint32_t)(data[8] & 0x7F) << 7) |
            ((uint32_t)(data[9] & 0x7F));

        uint32_t start = 10 + tagSize;

        if (start >= size)
            return 0;

        return start;
    }

    static void EnsureMP3Modules()
    {
        if (s_mp3ModulesLoaded)
            return;

        sceUtilityLoadAvModule(PSP_AV_MODULE_AVCODEC);
        sceUtilityLoadAvModule(PSP_AV_MODULE_MP3);

        s_mp3ModulesLoaded = true;
    }

    static void EnsureMixChannel()
    {
        if (s_mixChannel >= 0)
            return;

        s_mixChannel = sceAudioChReserve(
            PSP_AUDIO_NEXT_CHANNEL,
            AUDIO_SAMPLES,
            PSP_AUDIO_FORMAT_STEREO
        );
    }

    static float ComputeDistanceVolume(const AudioSourceInstance& inst)
    {
        const Camera& cam = CameraSystem::GetActiveCameraConst();

        float dx = cam.posX - inst.posX;
        float dy = cam.posY - inst.posY;
        float dz = cam.posZ - inst.posZ;

        float dist = sqrtf(dx * dx + dy * dy + dz * dz);

        float baseVolume = Clamp01(inst.volume);

        if (inst.maxDistance <= inst.minDistance)
            return baseVolume;

        if (dist <= inst.minDistance)
            return baseVolume;

        if (dist >= inst.maxDistance)
            return 0.0f;

        float t = (dist - inst.minDistance) / (inst.maxDistance - inst.minDistance);

        float falloff = 1.0f - t;
        falloff = falloff * falloff;

        return baseVolume * falloff;
    }

    static void ResetFilterState(int index)
    {
        if (index < 0 || index >= 32)
            return;

        hpInL[index] = 0.0f;
        hpInR[index] = 0.0f;
        hpOutL[index] = 0.0f;
        hpOutR[index] = 0.0f;

        s_mp3PendingSamples[index] = 0;
        s_mp3PendingCursor[index] = 0;
        s_mp3ReadPos[index] = s_mp3StreamStart[index];
    }

    static bool FeedMP3Data(AudioSourceInstance& inst, int index)
    {
        if (!inst.isMP3 || inst.mp3Handle < 0 || index < 0 || index >= 32)
            return false;

        if (s_mp3ReadPos[index] >= inst.mp3Size)
            return false;

        unsigned char* dst = nullptr;
        SceInt32 writable = 0;
        SceInt32 srcpos = 0;

        int infoResult = sceMp3GetInfoToAddStreamData(
            inst.mp3Handle,
            &dst,
            &writable,
            &srcpos
        );

        if (infoResult < 0 || !dst || writable <= 0)
            return false;

        uint32_t remaining = inst.mp3Size - s_mp3ReadPos[index];
        uint32_t toCopy = remaining;

        if (toCopy > (uint32_t)writable)
            toCopy = (uint32_t)writable;

        if (toCopy == 0)
            return false;

        std::memcpy(dst, inst.mp3Buffer + s_mp3ReadPos[index], toCopy);
        s_mp3ReadPos[index] += toCopy;

        int notifyResult = sceMp3NotifyAddStreamData(inst.mp3Handle, toCopy);

        if (notifyResult < 0)
            return false;

        return true;
    }

    void AudioSource::ReleaseMixChannelIfIdle()
    {
        if (s_mixChannel < 0)
            return;

        for (int i = 0; i < AudioSource::s_count; i++)
        {
            if (AudioSource::s_instances[i].playing)
                return;
        }

        sceAudioChRelease(s_mixChannel);
        s_mixChannel = -1;
    }

    static void Start(AudioSourceInstance& inst, int index)
    {
        if (!inst.isMP3 && inst.pcm.empty())
            return;

        inst.cursor = 0;
        inst.playing = true;
        inst.channel = -1;
        inst.smoothedVolume = 0.0f;

        ResetFilterState(index);

        if (inst.isMP3 && inst.mp3Handle >= 0)
        {
            sceMp3ResetPlayPosition(inst.mp3Handle);
            s_mp3ReadPos[index] = s_mp3StreamStart[index];
            FeedMP3Data(inst, index);
        }

        EnsureMixChannel();
    }

    static void Stop(AudioSourceInstance& inst, int index)
    {
        inst.playing = false;
        inst.cursor = 0;
        inst.channel = -1;
        inst.smoothedVolume = 0.0f;

        ResetFilterState(index);

        if (inst.isMP3 && inst.mp3Handle >= 0)
            sceMp3ResetPlayPosition(inst.mp3Handle);
    }

    static void Pause(AudioSourceInstance& inst)
    {
        inst.playing = false;
    }

    void AudioSource::Load(
        const std::vector<uint8_t>& bytes,
        uint32_t start,
        uint32_t end)
    {
        if (s_count >= 32)
            return;

        if (start >= bytes.size() || end > bytes.size() || start >= end)
            return;

        int index = s_count;

        const uint8_t* ptr = bytes.data() + start;
        const uint8_t* fileEnd = bytes.data() + bytes.size();

        ptr += sizeof(GV_ChunkHeader) + 4;
        ptr += sizeof(GV_ChunkHeader) + 4;

        if (ptr >= fileEnd)
            return;

        uint32_t paramCount = ReadUInt32(ptr);

        if (paramCount < 11)
            return;

        AudioSourceInstance inst{};
        inst.playing = false;
        inst.cursor = 0;
        inst.channel = -1;
        inst.smoothedVolume = 0.0f;
        inst.isMP3 = false;
        inst.mp3Handle = -1;
        inst.mp3Buffer = nullptr;
        inst.mp3Size = 0;
        inst.mp3PCM = nullptr;

        inst.volume = ReadFloat(ptr);
        inst.loop = ReadBool(ptr);
        inst.playOnStart = ReadBool(ptr);

        inst.posX = ReadFloat(ptr);
        inst.posY = ReadFloat(ptr);
        inst.posZ = ReadFloat(ptr);

        inst.minDistance = ReadFloat(ptr);
        inst.maxDistance = ReadFloat(ptr);

        uint32_t len = 0;

        len = ReadUInt32(ptr);
        if (ptr + len > fileEnd)
            return;

        inst.playMsg.assign((const char*)ptr, len);
        ptr += len;

        len = ReadUInt32(ptr);
        if (ptr + len > fileEnd)
            return;

        inst.stopMsg.assign((const char*)ptr, len);
        ptr += len;

        len = ReadUInt32(ptr);
        if (ptr + len > fileEnd)
            return;

        inst.pauseMsg.assign((const char*)ptr, len);
        ptr += len;

        Align16(ptr);

        if (ptr + 12 > fileEnd)
            return;

        inst.sampleRate = ReadUInt32(ptr);
        inst.channels = ReadUInt32(ptr);
        uint32_t size = ReadUInt32(ptr);

        if (ptr + size > fileEnd)
            return;

        inst.isMP3 = IsMP3Data(ptr, size);

        if (inst.isMP3)
        {
            EnsureMP3Modules();

            inst.mp3Buffer = (uint8_t*)memalign(64, size);
            if (!inst.mp3Buffer)
                return;

            std::memcpy(inst.mp3Buffer, ptr, size);
            inst.mp3Size = size;

            s_mp3StreamStart[index] = GetMP3StreamStart(inst.mp3Buffer, inst.mp3Size);
            s_mp3ReadPos[index] = s_mp3StreamStart[index];

            s_mp3StreamBuffer[index] = (uint8_t*)memalign(64, MP3_STREAM_BUFFER_SIZE);
            if (!s_mp3StreamBuffer[index])
                return;

            s_mp3PCMBuffer[index] = (uint8_t*)memalign(64, MP3_PCM_BUFFER_SIZE);
            if (!s_mp3PCMBuffer[index])
                return;

            SceMp3InitArg arg;
            std::memset(&arg, 0, sizeof(arg));

            arg.mp3StreamStart = s_mp3StreamStart[index];
            arg.mp3StreamEnd = inst.mp3Size - 1;
            arg.mp3Buf = s_mp3StreamBuffer[index];
            arg.mp3BufSize = MP3_STREAM_BUFFER_SIZE;
            arg.pcmBuf = s_mp3PCMBuffer[index];
            arg.pcmBufSize = MP3_PCM_BUFFER_SIZE;

            inst.mp3Handle = sceMp3ReserveMp3Handle(&arg);

            if (inst.mp3Handle < 0)
                return;

            int initResult = sceMp3Init(inst.mp3Handle);
            if (initResult < 0)
                return;

            FeedMP3Data(inst, index);

            inst.mp3PCM = (int16_t*)s_mp3PCMBuffer[index];
            inst.channels = sceMp3GetMp3ChannelNum(inst.mp3Handle);
            inst.sampleRate = sceMp3GetSamplingRate(inst.mp3Handle);
        }
        else
        {
            if (inst.channels != 1 && inst.channels != 2)
                return;

            inst.pcm.resize(size);
            std::memcpy(inst.pcm.data(), ptr, size);
        }

        ptr += size;

        Align16(ptr);

        s_instances[s_count] = inst;
        s_count++;

        ResetFilterState(index);

        if (s_instances[index].playOnStart)
            Start(s_instances[index], index);
    }

    void AudioSource::Update()
    {
        bool anyPlaying = false;

        std::memset(s_mixAccum, 0, sizeof(s_mixAccum));
        std::memset(s_buffer, 0, sizeof(s_buffer));

        for (int i = 0; i < s_count; i++)
        {
            AudioSourceInstance& inst = s_instances[i];

            if (!inst.playing)
                continue;

            float targetVolume = ComputeDistanceVolume(inst);
            inst.smoothedVolume += (targetVolume - inst.smoothedVolume) * 0.02f;

            float sourceGain = Clamp01(inst.smoothedVolume);

            if (sourceGain <= 0.0001f)
            {
                anyPlaying = true;
                continue;
            }

            uint32_t frames = 0;

            while (frames < AUDIO_SAMPLES)
            {
                if (inst.isMP3)
                {
                    if (inst.mp3Handle < 0 || inst.channels == 0)
                    {
                        inst.playing = false;
                        break;
                    }

                    if (s_mp3PendingCursor[i] >= s_mp3PendingSamples[i])
                    {
                        s_mp3PendingCursor[i] = 0;
                        s_mp3PendingSamples[i] = 0;

                        if (sceMp3CheckStreamDataNeeded(inst.mp3Handle) > 0)
                        {
                            if (!FeedMP3Data(inst, i))
                            {
                                if (!inst.loop && s_mp3ReadPos[i] >= inst.mp3Size)
                                {
                                    inst.playing = false;
                                    break;
                                }
                            }
                        }

                        int16_t* decoded = inst.mp3PCM;
                        int bytesDecoded = sceMp3Decode(inst.mp3Handle, &decoded);
                        inst.mp3PCM = decoded;

                        if (bytesDecoded <= 0)
                        {
                            if (inst.loop)
                            {
                                sceMp3ResetPlayPosition(inst.mp3Handle);
                                ResetFilterState(i);
                                s_mp3ReadPos[i] = s_mp3StreamStart[i];
                                FeedMP3Data(inst, i);
                                continue;
                            }

                            inst.playing = false;
                            break;
                        }

                        uint32_t decodedSamples = (uint32_t)(bytesDecoded / 2);
                        if (decodedSamples > MP3_PCM_SAMPLES)
                            decodedSamples = MP3_PCM_SAMPLES;

                        std::memcpy(s_mp3Pending[i], inst.mp3PCM, decodedSamples * sizeof(int16_t));

                        s_mp3PendingSamples[i] = decodedSamples;
                        s_mp3PendingCursor[i] = 0;
                    }

                    if (s_mp3PendingCursor[i] >= s_mp3PendingSamples[i])
                        break;

                    float xL = 0.0f;
                    float xR = 0.0f;

                    if (inst.channels == 1)
                    {
                        int16_t s = s_mp3Pending[i][s_mp3PendingCursor[i]++];
                        xL = (float)s * sourceGain;
                        xR = xL;
                    }
                    else
                    {
                        if (s_mp3PendingCursor[i] + 1 >= s_mp3PendingSamples[i])
                            break;

                        xL = (float)s_mp3Pending[i][s_mp3PendingCursor[i] + 0] * sourceGain;
                        xR = (float)s_mp3Pending[i][s_mp3PendingCursor[i] + 1] * sourceGain;

                        s_mp3PendingCursor[i] += 2;
                    }

                    float yL = xL - hpInL[i] + 0.995f * hpOutL[i];
                    float yR = xR - hpInR[i] + 0.995f * hpOutR[i];

                    hpInL[i] = xL;
                    hpInR[i] = xR;
                    hpOutL[i] = yL;
                    hpOutR[i] = yR;

                    uint32_t outIndex = frames * 2;

                    s_mixAccum[outIndex + 0] += (int32_t)yL;
                    s_mixAccum[outIndex + 1] += (int32_t)yR;

                    frames++;
                    anyPlaying = true;
                    continue;
                }

                uint32_t bytesPerFrame = inst.channels * 2;
                uint32_t totalBytes = (uint32_t)inst.pcm.size();

                if (bytesPerFrame == 0 || totalBytes == 0)
                    break;

                if (inst.cursor >= totalBytes)
                {
                    if (inst.loop)
                    {
                        inst.cursor = 0;
                    }
                    else
                    {
                        inst.playing = false;
                        inst.cursor = 0;
                        break;
                    }
                }

                const int16_t* src = (const int16_t*)(inst.pcm.data() + inst.cursor);

                float xL = 0.0f;
                float xR = 0.0f;

                if (inst.channels == 1)
                {
                    xL = (float)src[0] * sourceGain;
                    xR = xL;
                }
                else
                {
                    xL = (float)src[0] * sourceGain;
                    xR = (float)src[1] * sourceGain;
                }

                float yL = xL - hpInL[i] + 0.995f * hpOutL[i];
                float yR = xR - hpInR[i] + 0.995f * hpOutR[i];

                hpInL[i] = xL;
                hpInR[i] = xR;
                hpOutL[i] = yL;
                hpOutR[i] = yR;

                uint32_t outIndex = frames * 2;

                s_mixAccum[outIndex + 0] += (int32_t)yL;
                s_mixAccum[outIndex + 1] += (int32_t)yR;

                inst.cursor += bytesPerFrame;
                frames++;
                anyPlaying = true;
            }
        }

        if (!anyPlaying)
        {
            ReleaseMixChannelIfIdle();
            return;
        }

        EnsureMixChannel();

        if (s_mixChannel < 0)
            return;

        for (uint32_t i = 0; i < AUDIO_SAMPLES * 2; i++)
        {
            int32_t v = s_mixAccum[i];

            if (v > 32767)
                v = 32767;

            if (v < -32768)
                v = -32768;

            s_buffer[i] = (int16_t)v;
        }

        sceAudioOutputBlocking(
            s_mixChannel,
            PSP_AUDIO_VOLUME_MAX,
            s_buffer
        );

        ReleaseMixChannelIfIdle();
    }

    void AudioSource::HandleMessage(int index, const std::string& msg)
    {
        if (index < 0 || index >= s_count)
            return;

        AudioSourceInstance& inst = s_instances[index];

        if (!inst.playMsg.empty() && msg == inst.playMsg)
        {
            Start(inst, index);
            return;
        }

        if (!inst.stopMsg.empty() && msg == inst.stopMsg)
        {
            Stop(inst, index);
            return;
        }

        if (!inst.pauseMsg.empty() && msg == inst.pauseMsg)
        {
            Pause(inst);
            return;
        }
    }
}