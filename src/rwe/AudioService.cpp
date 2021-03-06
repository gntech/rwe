#include "AudioService.h"

namespace rwe
{
    AudioService::AudioService(
        SdlContext* sdlContext,
        SdlMixerContext* sdlMixerContext,
        AbstractVirtualFileSystem* fileSystem)
        : sdlContext(sdlContext),
          sdlMixerContext(sdlMixerContext),
          fileSystem(fileSystem)
    {
    }

    AudioService::LoopToken AudioService::loopSound(const SoundHandle& sound)
    {
        auto channel = sdlMixerContext->playChannel(-1, sound.get(), -1);
        return LoopToken(this, channel, sound);
    }

    void AudioService::playSound(const SoundHandle& sound)
    {
        sdlMixerContext->playChannel(-1, sound.get(), 0);
    }

    boost::optional<AudioService::SoundHandle> AudioService::loadSound(const std::string& soundName)
    {

        auto soundIter = soundBank.find(soundName);
        if (soundIter != soundBank.end())
        {
            return soundIter->second;
        }


        auto bytes = fileSystem->readFile("sounds/" + soundName + ".WAV");
        if (!bytes)
        {
            return boost::none;
        }

        auto rwOps = sdlContext->rwFromConstMem(bytes->data(), bytes->size());
        std::shared_ptr sound(sdlMixerContext->loadWavRw(rwOps.get()));
        sdlMixerContext->volumeChunk(sound.get(), MIX_MAX_VOLUME / 4);
        soundBank[soundName] = sound;

        return sound;
    }

    void AudioService::reserveChannels(unsigned int count)
    {
        auto num = sdlMixerContext->reserveChannels(count);
        if (num < count)
        {
            throw std::runtime_error("Failed to reserve audio channels");
        }
    }

    void AudioService::haltChannel(int channel)
    {
        sdlMixerContext->haltChannel(channel);
    }

    void AudioService::playSoundIfFree(const AudioService::SoundHandle& sound, unsigned int channel)
    {
        if (sdlMixerContext->playing(channel))
        {
            return;
        }

        sdlMixerContext->playChannel(channel, sound.get(), 0);
    }

    AudioService::LoopToken::LoopToken(AudioService* audioService, int channel, const AudioService::SoundHandle& sound)
        : audioService(audioService), channel(channel), sound(sound)
    {
    }

    AudioService::LoopToken::~LoopToken()
    {
        if (channel != -1)
        {
            audioService->haltChannel(channel);
        }
    }

    AudioService::LoopToken& AudioService::LoopToken::operator=(AudioService::LoopToken&& other) noexcept
    {
        audioService = other.audioService;
        channel = other.channel;
        sound = std::move(other.sound);

        other.channel = -1;

        return *this;
    }

    AudioService::LoopToken::LoopToken(AudioService::LoopToken&& other) noexcept
        : audioService(other.audioService), channel(other.channel), sound(std::move(other.sound))
    {
        other.channel = -1;
    }

    AudioService::LoopToken::LoopToken() : audioService(nullptr), channel(-1), sound(nullptr) {}

    boost::optional<const AudioService::SoundHandle&> AudioService::LoopToken::getSound()
    {
        if (channel == -1)
        {
            return boost::none;
        }

        return sound;
    }
}
