#pragma once

struct AudioComponent
{
	AudioComponent() { };

	AudioComponent(const char* name, bool isStatic, int tag, int layer)
		: m_Name(name), m_IsStatic(isStatic), m_Tag(tag), m_Layer(layer)
	{
	};

	const char* m_Name{ "New Entity" };
	bool m_IsStatic{ false };
	int m_Tag{ 0 };
	int m_Layer{ 0 };

	template<typename Archive>
	void serialize(Archive &archive) {
		archive(cereal::make_nvp("Name", std::string(m_Name)), cereal::make_nvp("Static", m_IsStatic),
			cereal::make_nvp("Tag", m_Tag), cereal::make_nvp("Layer", m_Layer));
	}
};
