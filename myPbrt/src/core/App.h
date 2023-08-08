#pragma once

#include "core.h"
#include "Camera.h"
#include "Integrator.h"
#include "Scene.h"

namespace MyPBRT {

#ifdef _DEBUG
#define SCREENSCALE glm::vec2(.2f)
#else
#define SCREENSCALE glm::vec2(.5f)
#endif    

	class App
	{
	public:
		//overwritten every frame
		glm::ivec2 scaled_resolution = glm::vec3(0);
		glm::vec2 normalized_mouse_position = glm::vec2(0);

	public:
		App();
		~App();
		void Update(double dt, glm::ivec2 resolution);
		uint32_t* GetImage();
		
		void MousePressed(int button);
		void MouseReleased(int button);
		void ButtonPressed(int key);
		void ButtonReleased(int key);
		void MouseMoved(double xpos, double ypos);
		void ScrollMoved(double offset);
		
		void CreateIMGUI();

	private:
		Camera camera;
		Integrator integrator;
		Scene scene;

		glm::vec3 next_sphere_pos = glm::vec3(0);
		float next_sphere_radius = 1;

		const char* material_options[4] = { "Diffuse", "Emmisive", "Metallic", "Glass" };
		const std::string str_mat_opt[4] = { "Diffuse", "Emissive", "Metallic", "Glass" };
		int current_selected_material_option = 0;

		std::string obj_file_to_load = "";

		bool holding_shift = false;
		bool holding_lmouse = false;

		float afk_timer = 0;

	private:
		void IMGUISettings();
		void IMGUIScene();
		void IMGUIMaterials();
		void IMGUIRendering();
		void IMGUISelection();
	};

}

