#pragma once

#include <string>
#include <SDL_mixer.h>

namespace rse
{
	class Music
	{
	public:
		Music(const std::string& name, const std::string& filename);
		~Music();

		void play();
		void loop();
		void pause();
		void resume();
		void stop();

		void setVolume(int volume);

		inline const bool isPlaying() const { return Mix_PlayingMusic() != 0; }
		inline const int getVolume() const { return m_Volume; }
		inline const std::string& getName() const { return m_Name; }
		inline const std::string& getFileName() const { return m_Filename; }

	private:
		std::string m_Name;
		std::string m_Filename;
		Mix_Music* m_Music;
		int m_Volume;
	};
}