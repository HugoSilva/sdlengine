#include "TestClearColor.h"

#include <imgui.h>

namespace Test
{
	TestClearColor::TestClearColor()
	{
	}


	TestClearColor::~TestClearColor()
	{
	}

	void TestClearColor::OnUpdate(float deltaTime)
	{

	}

	void TestClearColor::OnRender()
	{

	}

	void TestClearColor::OnImguiRender()
	{
		ImGui::ColorEdit4("Clear Color", m_clearColor);
	}
}
