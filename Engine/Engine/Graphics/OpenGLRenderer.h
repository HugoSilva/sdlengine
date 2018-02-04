#pragma once

#include <SDL.h>
#include <glm\glm.hpp>
#include <vector>
#include "Renderable2d.h"
#include "Renderer2D.h"
#include "IndexBuffer.h"

namespace graphics
{
	#define RENDERER_MAX_SPRITES	60000
	#define RENDERER_VERTEX_SIZE	sizeof(VertexData)
	#define RENDERER_SPRITE_SIZE	RENDERER_VERTEX_SIZE * 4
	#define RENDERER_BUFFER_SIZE	RENDERER_SPRITE_SIZE * RENDERER_MAX_SPRITES
	#define RENDERER_INDICES_SIZE	RENDERER_MAX_SPRITES * 6
	#define RENDERER_MAX_TEXTURES	32

	#define SHADER_VERTEX_INDEX 0
	#define SHADER_UV_INDEX		1
	#define SHADER_TID_INDEX	2
	#define SHADER_COLOR_INDEX	3


	class OpenGLRenderer : public Renderer2D
	{
	public:
		OpenGLRenderer(SDL_Window* win);
		~OpenGLRenderer();

		void begin();
		void submit(const Renderable2D* renderable);
		void end();
		void flush();

	private:
		GLuint m_VAO, m_VBO;
		GLsizei m_IndexCount;
		VertexData* m_Buffer;
		IndexBuffer* m_IBO;
		SDL_Window* m_Window;
		std::vector<unsigned int> m_TextureSlots;
	};
}
