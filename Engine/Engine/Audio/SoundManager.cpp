#include "SoundManager.h"

namespace audio
{
	std::vector<Sound*> SoundManager::m_Sounds;
	std::vector<Music*> SoundManager::m_Music;

	void SoundManager::init()
	{
		if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
		{
			std::string mixerError("error: %s\n", Mix_GetError());
			Logger::error("SDL_mixer could not initialize, " + mixerError);
		}
	}

	void SoundManager::add(Sound* sound)
	{
		m_Sounds.push_back(sound);
	}

	void SoundManager::add(Music* music)
	{
		m_Music.push_back(music);
	}

	Sound* SoundManager::getSound(const std::string& name)
	{
		for (Sound* sound : m_Sounds)
		{
			if (sound->getName() == name)
				return sound;
		}
		return nullptr;
	}

	Music* SoundManager::getMusic(const std::string& name)
	{
		for (Music* music : m_Music)
		{
			if (music->getName() == name)
				return music;
		}
		return nullptr;
	}

	void SoundManager::clean()
	{
		for (unsigned int i = 0; i < m_Sounds.size(); i++)
			delete m_Sounds[i];

		for (unsigned int i = 0; i < m_Music.size(); i++)
			delete m_Music[i];

		Mix_CloseAudio();
	}
}