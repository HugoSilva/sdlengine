#pragma once

#include <string>
#include <vector>

#include <SDL3_mixer/SDL_mixer.h>

namespace rse
{
	class Sound; //forward declaration for Sound
	class Music; //forward declaration for Music

	class SoundManager
	{
	public:
		static void init();
		static void add(Sound* sound);
		static void add(Music* music);
		static Sound* getSound(const std::string& name);
		static Music* getMusic(const std::string& name);
		static void clean();

		// SDL3_mixer has no global mixer: the engine owns one and shares it with
		// every Sound/Music instance.
		static MIX_Mixer* getMixer() { return s_Mixer; }

	private:
		SoundManager() { }
		static std::vector<Sound*> s_Sounds;
		static std::vector<Music*> s_Music;
		static MIX_Mixer* s_Mixer;
	};
}
