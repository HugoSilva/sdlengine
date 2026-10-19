#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

class Image {
 public:
  // Fills `mode` with the matching GL_* upload format for `surface`.
  static void checkImageMode(int* mode, SDL_Surface* surface);

  // Renders `text` into a new surface. The caller owns it (SDL_DestroySurface).
  static SDL_Surface* loadFont(TTF_Font* font, const char* text, SDL_Color textColor);

  // Loads an image file into a new surface. The caller owns it (SDL_DestroySurface).
  static SDL_Surface* loadImage(const char* filename);
};
