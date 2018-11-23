#pragma once

#include "../../Physics/Rigidbody.h"

struct RigidBodyComponent
{
	RigidBodyComponent() { };

	RigidBodyComponent(Rigidbody* rigidbody) : m_Rigidbody(rigidbody)
	{
	};

	std::shared_ptr<Rigidbody> m_Rigidbody;

	template<typename Archive>
	void serialize(Archive &archive)
	{
		archive(cereal::make_nvp("Rigidbody", m_Rigidbody));
	}
};