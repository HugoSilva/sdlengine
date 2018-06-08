#pragma once

#include "Test.h"

namespace Test
{
	class TestClearColor : public Test
	{
	public:
		TestClearColor();
		~TestClearColor();

		void OnUpdate(float deltaTime) override;
		void OnRender() override;
		void OnImguiRender() override;

	private:
		float m_clearColor[4];
	};
}

