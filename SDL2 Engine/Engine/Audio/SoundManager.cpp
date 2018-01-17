#include "SoundManager.h"

namespace audio {

	std::vector<Sound*> SoundManager::m_Sounds;

	void SoundManager::init()
	{
		if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
			return;
	}

	void SoundManager::add(Sound* sound)
	{
		m_Sounds.push_back(sound);
	}

	Sound* SoundManager::get(const std::string& name)
	{
		for (Sound* sound : m_Sounds)
		{
			if (sound->getName() == name)
				return sound;
		}
		return nullptr;
	}

	void SoundManager::clean()
	{
		for (int i = 0; i < m_Sounds.size(); i++)
			delete m_Sounds[i];

		// quit SDL_mixer
		Mix_CloseAudio();
	}

	void SoundManager::update()
	{
	}

}