#include "Texture.h"

int NearestPowerOf2(int n)
{
	if (!n) return n;
	int x = 1;
	while (x < n)
	{
		x <<= 1;
	}
	return x;
}

void* LoadImage(const char* filename, int* width, int* height, int* mode)
{
	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags)) {
		printf("SDL_image could not initialize. SDL_image Error: %s\n", IMG_GetError());
		return false;
	}

	SDL_Surface* surface = IMG_Load(filename);

	SDL_Surface* resizedSurface = NULL;

	int newWidth = NearestPowerOf2(surface->w);
	int newHeight = NearestPowerOf2(surface->h);

	int bpp;
	Uint32 Rmask, Gmask, Bmask, Amask;

	SDL_PixelFormatEnumToMasks(
		SDL_PIXELFORMAT_ABGR8888, &bpp,
		&Rmask, &Gmask, &Bmask, &Amask
	);

	resizedSurface = SDL_CreateRGBSurface(0, newWidth, newHeight, bpp,
		Rmask, Gmask, Bmask, Amask
	);

	SDL_Rect area;

	area.x = 0;
	area.y = 0;
	area.w = surface->w;
	area.h = surface->h;

	SDL_SetSurfaceAlphaMod(surface, 0xFF);
	SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);
	SDL_BlitSurface(surface, &area, resizedSurface, &area);

	bool lock = SDL_MUSTLOCK(resizedSurface);
	if (lock)
		SDL_LockSurface(resizedSurface);  // should check that return value == 0
								   // access pixel data, e.g. call glTexImage2D
	if (lock)
		SDL_UnlockSurface(resizedSurface);

	//glTexImage2D(GL_TEXTURE_2D, 0, Mode, resizedSurface->w, resizedSurface->h, 0, Mode, GL_UNSIGNED_BYTE, resizedSurface->pixels);

	if (resizedSurface->format->BytesPerPixel == 4) {
		*mode = GL_RGBA;
	}

	*width = resizedSurface->w;
	*height = resizedSurface->h;

	return resizedSurface->pixels;

	SDL_FreeSurface(resizedSurface);
	SDL_FreeSurface(surface);

	if (surface == NULL)
	{
		//throw Exception("ResourceManager::texture(name) - cannot load texture from file " + filename + ": " + IMG_GetError());
	}
}

Texture::Texture(std::string pathname)
	:m_Filename(pathname)
{
	int Mode = GL_RGB;
	void* pixels = LoadImage(m_Filename.c_str(), &m_Width, &m_Height, &Mode);

	unsigned int result;
	glGenTextures(1, &result);
	glBindTexture(GL_TEXTURE_2D, result);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, Mode, m_Width, m_Height, 0, Mode, GL_UNSIGNED_BYTE, pixels);

	glBindTexture(GL_TEXTURE_2D, 0);

	m_Id = result;
}

Texture::~Texture()
{

}

void Texture::bind() const
{
	glBindTexture(GL_TEXTURE_2D, m_Id);
}

void Texture::unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}
