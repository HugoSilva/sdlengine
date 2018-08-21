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
    archive(basic.m_Name, basic.m_IsStatic, basic.m_Tag, basic.m_Layer);
}