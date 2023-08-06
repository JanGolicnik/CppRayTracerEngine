# C++ Software Raytracer

This is a C++ software raytracer that provides an interactive experience for creating and rendering scenes
## Features

	Lambertian, metallic, glass, emmisive materials all with varius textures and normal maps to enhance the visuals
	Support for triangle meshes and spheres
	Interactive user interface using ImGui for scene setup and control.
	Supports loading 3D models in OBJ format using the obj_loader library.

## Prerequisites

Before you can build and run the raytracer, ensure you have the following dependencies installed:

	GLAD: OpenGL loader-generator, OpenGL 3.3 and extensions loading library.
	GLFW: Multi-platform library for OpenGL, OpenGL ES, and Vulkan development.
	GLM: Mathematics library for graphics software.
	OBJ Loader: Library for loading OBJ 3D models.
	stb_image: Image loading and saving library.

## User Interface

Upon running the raytracer, a window will open, showing the interactive user interface powered by ImGui. The interface allows you to:

	Load 3D models in OBJ format to be included in the raytraced scene.
	Manipulate the camera position and orientation.
	Adjust the material properties of objects, such as color, reflection, and transparency.
	Add custom image textures and normal maps.
	Control the lighting in the scene.

## Acknowledgments

This raytracer was built with the help of the following fantastic libraries:

[ImGui](https://github.com/ocornut/imgui) for the interactive user interface.
[GLAD](https://glad.dav1d.de/) for OpenGL loading.
[GLFW](https://www.glfw.org/) for window management and input handling.
[GLM](https://github.com/g-truc/glm) for mathematics operations.
[OBJ Loader](https://github.com/Bly7/OBJ-Loader) for loading 3D models in OBJ format.
[stb_image](https://github.com/nothings/stb) for image loading and saving.
