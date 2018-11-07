#include "Music.hpp"

#include "../Utils/Logger.hpp"

namespace audio
{
	Music::Music(const std::string& name, const std::string& filename)
		: m_Name(name), m_Filename(filename)
	{
		m_Music = Mix_LoadMUS(filename.c_str());
		if (m_Music == nullptr)
		{
			Logger::error("SDL_mixer could not load filename " + m_Filename);
		}
	}

	Music::~Music()
	{
		Mix_FreeMusic(m_Music);
	}

	void Music::play()
	{
		if (Mix_PlayMusic(m_Music, 0) == -1)
		{
			Logger::error("SDL_mixer could not play filename " + m_Filename);
		}
	}

	void Music::loop()
	{
		if (Mix_PlayMusic(m_Music, -1) == -1)
		{
			Logger::error("SDL_mixer could not loop filename " + m_Filename);
		}
	}

	void Music::resume()
	{
		Mix_ResumeMusic();
	}

	void Music::pause()
	{
		Mix_PauseMusic();
	}

	void Music::stop()
	{
		Mix_HaltMusic();
	}

	void Music::setVolume(int volume)
	{
		m_Volume = volume;
		Mix_VolumeMusic(m_Volume);
	}
}