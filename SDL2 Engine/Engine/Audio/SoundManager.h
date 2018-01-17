#pragma once

#include <vector>
#include <SDL2_mixer\SDL_mixer.h>

#include "sound.h"

namespace audio {

	class SoundManager
	{
	public:
		static void init();
		static void add(Sound* sound);
		static Sound* get(const std::string& name);
		static void update();
		static void clean();
	private:
		SoundManager() { }

		static std::vector<Sound*> m_Sounds;
	};

}