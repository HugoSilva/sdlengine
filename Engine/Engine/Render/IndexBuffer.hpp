#pragma once

namespace graphics
{
	class IndexBuffer
	{
	public:
		IndexBuffer(unsigned short* data, int count);
		IndexBuffer(unsigned int* data, int count);
		~IndexBuffer();
		void bind() const;
		void unbind() const;

		inline unsigned int getCount() const { return m_Count; }

	private:
		unsigned int m_BufferID;
		unsigned int m_Count;
	};
}