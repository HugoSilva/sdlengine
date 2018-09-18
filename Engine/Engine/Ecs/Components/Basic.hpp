#pragma once

struct BasicComponent
{
	BasicComponent() { };

	BasicComponent(const char* name, bool isStatic, int tag, int layer) 
		: m_Name(name), m_IsStatic(isStatic), m_Tag(tag), m_Layer(layer)
	{
	};

    const char* m_Name{ "New Entity" };
	bool m_IsStatic{ false };
	int m_Tag{ 0 };
	int m_Layer{ 0 };
};

template<typename Archive>
void serialize(Archive &archive, BasicComponent &basic) {
    archive(cereal::make_nvp("Name", std::string(basic.m_Name)), cereal::make_nvp("Static", basic.m_IsStatic), 
        cereal::make_nvp("Tag", basic.m_Tag), cereal::make_nvp("Layer", basic.m_Layer));
}