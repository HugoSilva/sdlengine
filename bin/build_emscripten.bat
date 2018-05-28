emcc -std=c++11 -I"../Engine/Engine/" -I"../Thirdparty/Box2D/include/" -I"../Thirdparty/Glew/include/" -I"../Thirdparty/glm/include/" -I"../Thirdparty/imgui/include/" -I"../Thirdparty/SDL2_mixer/include/" -I"../Thirdparty/SDL2_ttf/include/" "../Engine/Engine/Audio/Music.cpp" "../Engine/Engine/Audio/Sound.cpp" "../Engine/Engine/Audio/SoundManager.cpp" "../Engine/Engine/Components/Label.cpp" "../Engine/Engine/Components/Shader.cpp" "../Engine/Engine/Components/Sprite.cpp" "../Engine/Engine/Components/Texture.cpp" "../Engine/Engine/Components/TextureManager.cpp" "../Engine/Engine/Graphics/Font.cpp" "../Engine/Engine/Graphics/FontManager.cpp" "../Engine/Engine/Graphics/Indexbuffer.cpp" "../Engine/Engine/Graphics/Layer.cpp" "../Engine/Engine/Graphics/OpenGLRenderer.cpp" "../Engine/Engine/Graphics/Window.cpp" "../Engine/Engine/IO/InputManager.cpp" "../Engine/Engine/IO/Keyboard.cpp" "../Engine/Engine/IO/Mouse.cpp" "../Engine/Engine/Physics/PhysicsManager.cpp" "../Engine/Engine/Physics/Rigidbody.cpp" "../Engine/Engine/Utils/imguigl2.cpp" "../Engine/Engine/Utils/Logger.cpp" "../Engine/Engine/Camera.cpp" "../Engine/Engine/Core.cpp" "../Game/Source/Main.cpp" "../Game/Source/Scene01.cpp" imgui.bc box2d.bc -s USE_SDL=2 -s FULL_ES3=1 -s LEGACY_GL_EMULATION=1 -s USE_SDL_IMAGE=2 -s USE_OGG=1 -s USE_VORBIS=1 -s WARN_ON_UNDEFINED_SYMBOLS=1 -DEMSCRIPTEN=1 --memory-init-file 0 --embed-file Resources -O3 -o Emscripten/index.html