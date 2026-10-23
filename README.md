## A little 3D engine I have been working on. Made public to showcase to others.


Uses DirectX11 to render, and SDL3 for windowing, input etc.

This is a personal repo, and as such, this repo does not really use a .gitignore to ignore most unnecessary files.
It may not work straight away without fixing filepaths!

### Features
- Full shaded 3d rendering with multiple lights and light colours, textures and normal maps.
- Custom mesh format (.nge) and converter to format
- Support for skybox/skydomes
- 2D text rendering support

### Interesting files:
Myopic.cpp - contains the game loop
Graphics.cpp/Graphics.h - contains the bulk of the abstraction layer for rendering
Assets.cpp/Assets.h - contains asset loading/management functions

MeshCompiler/MeshCompiler.cpp - contains code that uses Assimp to convert files into custom mesh format.

