#include "SoundManager.hpp"

#include <SDL_mixer.h>

#include "Sound.hpp"
#include "Music.hpp"
#include "../Utils/Logger.hpp"

namespace rse
{
	std::vector<Sound*> SoundManager::s_Sounds;
	std::vector<Music*> SoundManager::s_Music;

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
		s_Sounds.push_back(sound);
	}

	void SoundManager::add(Music* music)
	{
		s_Music.push_back(music);
	}

	Sound* SoundManager::getSound(const std::string& name)
	{
		for (Sound* sound : s_Sounds)
		{
			if (sound->getName() == name)
				return sound;
		}
		return nullptr;
	}

	Music* SoundManager::getMusic(const std::string& name)
	{
		for (Music* music : s_Music)
		{
			if (music->getName() == name)
				return music;
		}
		return nullptr;
	}

	void SoundManager::clean()
	{
		for (unsigned int i = 0; i < s_Sounds.size(); i++)
			delete s_Sounds[i];

		for (unsigned int i = 0; i < s_Music.size(); i++)
			delete s_Music[i];

		Mix_CloseAudio();
	}
}