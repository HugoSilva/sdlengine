#include "Sound.h"

namespace audio {

	Sound::Sound(const std::string& name, const std::string& filename)
		: m_Name(name), m_Filename(filename), m_Playing(false)
	{
		//m_Wave = Mix_LoadWAV(filename.c_str());
		//if (m_Wave == nullptr)
		//{
		//	//log error
		//	return;
		//}

		m_Music = Mix_LoadMUS(filename.c_str());
		if (m_Music == nullptr)
		{
			//log error
			return;
		}
	}

	Sound::~Sound()
	{
		//Mix_FreeChunk(m_Wave);
		Mix_FreeMusic(m_Music);
	}

	void Sound::play()
	{
		//if (Mix_PlayChannel(-1, m_Wave, 0) == -1)
		if (Mix_PlayMusic(m_Music, 0) == -1)
		{
			//log error
		}
		m_Playing = true;
	}

	void Sound::loop()
	{
		if (Mix_PlayMusic(m_Music, -1) == -1)
		{
			//log error
		}
		m_Playing = true;
	}

	void Sound::resume()
	{
		if (m_Playing)
			return;

		m_Playing = true;

		//Mix_Resume
		Mix_ResumeMusic();
	}

	void Sound::pause()
	{
		if (!m_Playing)
			return;

		m_Playing = false;
		Mix_PauseMusic();
	}

	void Sound::stop()
	{
		if (!m_Playing)
			return;

		m_Playing = false;
		Mix_HaltChannel(-1);
	}

	void Sound::setGain(float gain)
	{
		if (!m_Playing)
		{
			//log Cannot set gain! Sound is not currently playing
			return;
		}
		m_Gain = gain;
		Mix_VolumeChunk(m_Wave, m_Gain);
	}
}