#include "Sound.h"

namespace audio {

	Sound::Sound(const std::string& name, const std::string& filename)
		: m_Name(name), m_Filename(filename)
	{
		m_Sound = Mix_LoadWAV(filename.c_str());
		if (m_Sound == nullptr)
		{
			Logger::error("SDL_mixer could not load filename " + m_Filename);
		}
	}

	Sound::~Sound()
	{
		Mix_FreeChunk(m_Sound);
	}

	void Sound::play()
	{
		m_Channel = Mix_PlayChannel(-1, m_Sound, 0);
		if (m_Channel == -1)
		{
			Logger::error("SDL_mixer could not play filename " + m_Filename);
		}
	}

	void Sound::loop()
	{
		m_Channel = Mix_PlayChannel(-1, m_Sound, -1);
		if (m_Channel == -1)
		{
			Logger::error("SDL_mixer could not loop filename " + m_Filename);
		}
	}

	void Sound::resume()
	{
		Mix_Resume(m_Channel);
	}

	void Sound::pause()
	{
		Mix_Pause(m_Channel);
	}

	void Sound::stop()
	{
		Mix_HaltChannel(m_Channel);
	}

	void Sound::setVolume(int volume)
	{
		m_Volume = volume;
		Mix_VolumeChunk(m_Sound, m_Volume);
	}
}