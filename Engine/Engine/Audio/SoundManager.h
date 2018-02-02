#pragma once

#include <vector>
#include <SDL_mixer.h>

#include "Sound.h"
#include "Music.h"
#include "../Utils/Logger.h"

namespace audio
{
	class SoundManager
	{
	public:
		static void init();
		static void add(Sound* sound);
		static void add(Music* music);
		static Sound* getSound(const std::string& name);
		static Music* getMusic(const std::string& name);
		static void clean();

	private:
		SoundManager() { }
		static std::vector<Sound*> m_Sounds;
		static std::vector<Music*> m_Music;
	};
}