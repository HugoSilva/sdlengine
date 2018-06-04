#include "TestClearColor.h"

#include <imgui.h>
#include <GL/glew.h>

namespace Test
{
	TestClearColor::TestClearColor() : m_clearColor { 1.0f, 0.0f, 1.0f, 1.0f }
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
		glClearColor(m_clearColor[0], m_clearColor[1], m_clearColor[2], m_clearColor[3]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void TestClearColor::OnImguiRender()
	{
		ImGui::ColorEdit4("Clear Color", m_clearColor);
	}
}
