#include "OpenGLRenderer.h"
#include <imgui.h>
#include "../Utils/imgui_impl_sdl_gl3.h"

namespace graphics
{
	OpenGLRenderer::OpenGLRenderer(SDL_Window* win)
		: m_Window(win)
	{
		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);

		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, RENDERER_BUFFER_SIZE, NULL, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(SHADER_VERTEX_INDEX);
		glEnableVertexAttribArray(SHADER_UV_INDEX);
		glEnableVertexAttribArray(SHADER_TID_INDEX);
		glEnableVertexAttribArray(SHADER_COLOR_INDEX);

		glVertexAttribPointer(SHADER_VERTEX_INDEX, 3, GL_FLOAT, GL_FALSE, RENDERER_VERTEX_SIZE, (const GLvoid*)0);
		glVertexAttribPointer(SHADER_UV_INDEX, 2, GL_FLOAT, GL_FALSE, RENDERER_VERTEX_SIZE, (const GLvoid*)(offsetof(VertexData, uv)));
		glVertexAttribPointer(SHADER_TID_INDEX, 1, GL_FLOAT, GL_FALSE, RENDERER_VERTEX_SIZE, (const GLvoid*)(offsetof(VertexData, tid)));
		glVertexAttribPointer(SHADER_COLOR_INDEX, 4, GL_UNSIGNED_BYTE, GL_TRUE, RENDERER_VERTEX_SIZE, (const GLvoid*)(offsetof(VertexData, color)));

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		unsigned int* indices = new unsigned int[RENDERER_INDICES_SIZE];

		int offset = 0;
		for (int i = 0; i < RENDERER_INDICES_SIZE; i += 6)
		{
			indices[i] = offset + 0;
			indices[i + 1] = offset + 1;
			indices[i + 2] = offset + 2;

			indices[i + 3] = offset + 2;
			indices[i + 4] = offset + 3;
			indices[i + 5] = offset + 0;

			offset += 4;
		}

		m_IBO = new IndexBuffer(indices, RENDERER_INDICES_SIZE);

		glBindVertexArray(0);

		#ifdef EMSCRIPTEN
			m_BufferBase = new VertexData[RENDERER_MAX_SPRITES * 4];
		#endif // EMSCRIPTEN

	}

	OpenGLRenderer::~OpenGLRenderer()
	{
		delete m_IBO;
		glDeleteBuffers(1, &m_VBO);
		glDeleteVertexArrays(1, &m_VAO);
	}

	void OpenGLRenderer::begin()
	{
		ImGui_ImplSdlGL3_NewFrame(m_Window);

		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		#ifdef EMSCRIPTEN
			m_Buffer = m_BufferBase;
		#else
			m_Buffer = (VertexData*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
		#endif // EMSCRIPTEN

		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void OpenGLRenderer::submit(const Renderable2D* renderable)
	{
		const glm::vec3& position = renderable->getPosition();
		const glm::vec2& size = renderable->getSize();
		const unsigned int& color = renderable->getColor();
		const std::vector<glm::vec2>& uv = renderable->getUV();
		const unsigned int tid = renderable->getTId();

		float ts = 0.0f;
		if (tid > 0)
		{
			bool found = false;
			for (unsigned int i = 0; i < m_TextureSlots.size(); i++)
			{
				if (m_TextureSlots[i] == tid)
				{
					ts = (float)(i + 1);
					found = true;
					break;
				}
			}

			if (!found)
			{
				if (m_TextureSlots.size() >= RENDERER_MAX_TEXTURES)
				{
					end();
					flush();
					begin();
				}
				m_TextureSlots.push_back(tid);
				ts = (float)(m_TextureSlots.size());
			}
		}

		m_Buffer->vertex = position;
		m_Buffer->uv = uv[0];
		m_Buffer->tid = ts;
		m_Buffer->color = color;
		m_Buffer++;

		m_Buffer->vertex = glm::vec3(position.x, position.y + size.y, position.z);
		m_Buffer->uv = uv[1];
		m_Buffer->tid = ts;
		m_Buffer->color = color;
		m_Buffer++;

		m_Buffer->vertex = glm::vec3(position.x + size.x, position.y + size.y, position.z);
		m_Buffer->uv = uv[2];
		m_Buffer->tid = ts;
		m_Buffer->color = color;
		m_Buffer++;

		m_Buffer->vertex = glm::vec3(position.x + size.x, position.y, position.z);
		m_Buffer->uv = uv[3];
		m_Buffer->tid = ts;
		m_Buffer->color = color;
		m_Buffer++;

		m_IndexCount += 6;
	}

	void OpenGLRenderer::end()
	{
		#ifdef EMSCRIPTEN
			glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, (m_Buffer - m_BufferBase) * RENDERER_VERTEX_SIZE, m_BufferBase);
			m_Buffer = m_BufferBase;
		#else
			glUnmapBuffer(GL_ARRAY_BUFFER);
		#endif // EMSCRIPTEN

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void OpenGLRenderer::flush()
	{
		for (unsigned int i = 0; i < m_TextureSlots.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, m_TextureSlots[i]);
		}

		glBindVertexArray(m_VAO);
		m_IBO->bind();

		glDrawElements(GL_TRIANGLES, m_IndexCount, GL_UNSIGNED_INT, NULL);

		m_IBO->unbind();
		glBindVertexArray(0);

		m_IndexCount = 0;

		ImGui::Render();
		ImGui_ImplSdlGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(m_Window);
	}
}
