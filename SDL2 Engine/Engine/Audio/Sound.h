#pragma once

#include <iostream>
#include <string>

#include <SDL2_mixer\SDL_mixer.h>

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

		void setGain(float gain);

		inline const bool isPlaying() const { return m_Playing; }
		inline const float getGain() const { return m_Gain; }
		inline const std::string& getName() const { return m_Name; }
		inline const std::string& getFileName() const { return m_Filename; }

	private:
		std::string m_Name;
		std::string m_Filename;

		Mix_Chunk* m_Wave = NULL;
		Mix_Music* m_Music = NULL;

		bool m_Playing;
		float m_Gain;
	};

}