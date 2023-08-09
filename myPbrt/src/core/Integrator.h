#pragma once

#include "core.h"
#include "BaseTypes.h"
#include "Camera.h"
#include "Sampler.h"
#include "Texture.h"

#include <thread>
#include <set>

namespace MyPBRT {

	class Integrator
	{
	public:
		enum class RenderingType {
			PBR = 0,
			Wireframe = 1,
			Rasterized = 2
		};

		enum class OverlayType {
			None = 0,
			Selection = 1,
			All = 2
		};

		struct RasterPixel {
			glm::vec2 uv;
			glm::vec3 normal;
			union {
				glm::vec2 normalized_position;
				glm::ivec2 screen_positon;
			};
			float depth;
			//in case of overlap
			float order;
		};

		int bounces;
		glm::vec2 image_scale = glm::vec2(1.0f);
		
		bool depth_only = false;

		const char* rendering_options[3] = { "PBR", "Wireframe", "Rasterized" };
		RenderingType rendering_type = RenderingType::PBR;
		const char* overlay_options[3] = { "None", "Selection", "All" };
		OverlayType overlay_type = OverlayType::Selection;

		glm::vec3 gooch_warm = glm::vec3(0.8, 0.6, 0.6), gooch_cool = glm::vec3(0.1, 0.1, 0.3);
		glm::vec3 wireframe_color = glm::vec3(0.0f);
		glm::vec3 overlay_color = glm::vec3(.9,1,.4);

		std::set<int> selected_objects;

	public:
		Integrator(uint32_t _bounces, const glm::ivec2& _resolution, const glm::vec2& scale);
		~Integrator() {}
		virtual void Predprocess(const Scene& scene, Sampler& sampler) {}
		void Render(const Scene& scene, const Camera& camera);
		glm::vec3 TraceRay(Ray* ray, int depth = 0) const;
		void ResetFrameIndex() { frame = 0; }
		void OnResize(const glm::ivec2& size);
		uint32_t* GetImage(bool overlays = true);
		void Clear();

		void RenderRayTraced();
		void RenderWireframe();
		void RenderRasterized();

		void CreateIMGUI();

		glm::ivec2 ScaledResolution() { return render_resolution; }
		const glm::ivec2& ScaledResolution() const { return render_resolution; }
		glm::ivec2 Resolution() { return image_resolution; }
		const glm::ivec2& Resolution() const { return image_resolution; }

		std::weak_ptr<Texture> GetWorldTexture() { return world_texture; }

	private:
		glm::ivec2 render_resolution;
		glm::ivec2 image_resolution;
		uint32_t frame = 0;

		bool draw_overlays = true;

		//w channel for depth
		glm::vec4* image;
		uint32_t* output_image;

		const Camera* active_camera;
		const Scene* active_scene;
	
		std::vector<uint32_t> height_iterator, width_iterator;

		std::shared_ptr<Texture> world_texture;
		std::vector<Texture::TextureType> world_texture_types = { Texture::TextureType::ConstantColor, Texture::TextureType::Image };
		int selected_world_texture = 0;

	private:
		void DrawOverlays();

		IntegratorSetPixelFunctionPtr set_pixel_uint32 = [this](uint32_t x, uint32_t y, glm::vec4 c) {	output_image[x + y * render_resolution.x] = ToUint(c); };
		IntegratorSetPixelFunctionPtr set_pixel_vec4 = [this](uint32_t x, uint32_t y, glm::vec4 c) {image[x + y * render_resolution.x] = c; };

	};

}
