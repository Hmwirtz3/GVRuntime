#pragma once

#include <cstdint>
#include <vector>
#include <string>

#include <psptypes.h>
#include <pspkernel.h>
#include <pspmp3.h>

#ifdef GV_EDITOR

BEGIN_LOGIC_UNIT(AudioSource, GV_CHUNK_AUDIO_SOURCE)

UI_PARAM_ASSET(audioFile, "", "Path or name of audio file")
UI_PARAM_FLOAT(volume, 1.0f, "Volume 0-1")
UI_PARAM_BOOL(loop, false, "Loop playback")

UI_PARAM_BOOL(playOnStart, false, "Auto play on scene load")

UI_PARAM_FLOAT(posX, 0.0f, "World position X")
UI_PARAM_FLOAT(posY, 0.0f, "World position Y")
UI_PARAM_FLOAT(posZ, 0.0f, "World position Z")

UI_PARAM_FLOAT(minDistance, 1.0f, "Full volume distance")
UI_PARAM_FLOAT(maxDistance, 20.0f, "Max hearing distance")

UI_MESSAGE(play, "","Play audio")
UI_MESSAGE(stop, "","Stop audio")
UI_MESSAGE(pause, "","Pause audio")

END_LOGIC_UNIT

#endif

namespace GV
{
    struct AudioSourceInstance
    {
        float volume;
        bool loop;
        bool playOnStart;

        float posX;
        float posY;
        float posZ;

        float minDistance;
        float maxDistance;

        std::string playMsg;
        std::string stopMsg;
        std::string pauseMsg;

        uint32_t sampleRate;
        uint32_t channels;

        std::vector<uint8_t> pcm;

        bool playing;
        uint32_t cursor;
        int channel;

        float smoothedVolume;

        bool isMP3;
        SceUID mp3Handle;
        uint8_t* mp3Buffer;
        uint32_t mp3Size;
        int16_t* mp3PCM;
    };

    class AudioSource
    {
    public:
        static void Load(const std::vector<uint8_t>& bytes, uint32_t start, uint32_t end);
        static void Update();
        static void HandleMessage(uint32_t index,
                          const std::string& msg,
                          uint32_t senderType,
                          uint32_t senderIndex,
                          const void* payload,
                          uint32_t payloadSize);
                          
        static int FindInstanceIndex(AudioSourceInstance& inst);

    private:
        static AudioSourceInstance s_instances[32];
        static int s_count;

        static void ReleaseMixChannelIfIdle();
        
    };
}