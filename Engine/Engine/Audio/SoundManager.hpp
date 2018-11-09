#pragma once

#include <vector>

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

	private:
		SoundManager() { }
		static std::vector<Sound*> s_Sounds;
		static std::vector<Music*> s_Music;
	};
}