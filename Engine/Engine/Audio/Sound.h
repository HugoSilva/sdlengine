#pragma once

#include <iostream>
#include <string>
#include <SDL_mixer.h>

#include "../Utils/Logger.h"

namespace audio {

	class Sound
	{
	public:
		Sound(const std::string& name, const std::string& filename);
		~Sound();

		void play();
		void loop();
		void pause();
		void resume();
		void stop();

		void setVolume(int volume);

		inline const bool isPlaying() const { return Mix_Playing(m_Channel) != 0; }
		inline const int getVolume() const { return m_Volume; }
		inline const std::string& getName() const { return m_Name; }
		inline const std::string& getFileName() const { return m_Filename; }

	private:
		std::string m_Name;
		std::string m_Filename;
		Mix_Chunk* m_Sound;
		int m_Volume;
		int m_Channel;
	};
}