#pragma once

#include "../../Audio/Sound.hpp"

struct AudioComponent
{
	AudioComponent() { };

	AudioComponent(rse::Sound* sound) : m_Sound(sound)
	{
	};

	std::shared_ptr<rse::Sound> m_Sound;

	template<typename Archive>
	void serialize(Archive &archive)
	{
		archive(cereal::make_nvp("Sound", m_Sound));
	}
};
