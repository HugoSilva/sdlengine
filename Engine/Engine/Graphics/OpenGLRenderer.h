#pragma once

#ifdef EMSCRIPTEN
#define GL_GLEXT_PROTOTYPES
#include <SDL_opengles2.h>

// OES_vertex_array_object
#define glGenVertexArrays glGenVertexArraysOES
#define glBindVertexArray glBindVertexArrayOES
#define glDeleteVertexArrays glDeleteVertexArraysOES

// OES_mapbuffer
#define glMapBuffer glMapBufferOES
#define glUnmapBuffer glUnmapBufferOES
#define GL_WRITE_ONLY GL_WRITE_ONLY_OES

#else
#include <GL/glew.h>
#endif // EMSCRIPTEN

#include <glm\glm.hpp>
#include <vector>
#include "Renderable2D.h"
#include "Renderer2D.h"
#include "IndexBuffer.h"
#include "../../Tests/TestClearColor.h"
#include "../Components/Shader.h"

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
		OpenGLRenderer(SDL_Window* win, Shader* shader);
		~OpenGLRenderer();

		void begin();
		void submit(TransformComponent& position, const Renderable2D* renderable);
		void end();
		void flush();

	private:
		unsigned int m_VAO, m_VBO;
		int m_IndexCount;
		VertexData* m_Buffer;
		IndexBuffer* m_IBO;
		SDL_Window* m_Window;
		std::vector<unsigned int> m_TextureSlots;
		Test::TestClearColor* clearColor;
		Shader* m_Shader;
#ifdef EMSCRIPTEN
		VertexData* m_BufferBase;
#endif // EMSCRIPTEN
	};
}
