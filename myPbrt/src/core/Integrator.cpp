#include "Integrator.h"

#include "Interaction.h"
#include "Scene.h"
#include "Material.h"
#include "Object.h"
#include "Shape.h"

#include <glm/geometric.hpp>

#include <algorithm>
#include <execution>

namespace MyPBRT {

	Integrator::Integrator(uint32_t _bounces, const glm::ivec2& _resolution, const glm::vec2& scale)
		: image_resolution(_resolution), bounces(_bounces), image_scale(scale)
	{
		OnResize(image_resolution);
	}
	void Integrator::Render(const Scene& scene, const Camera& camera)
	{
		scene.Preprocess();

		active_camera = &camera;
		active_scene = &scene;

		switch (rendering_type) {
		case RenderingType::PBR:
			RenderRayTraced();
			break;
		case RenderingType::Wireframe:
			RenderWireframe();
			break;
		case RenderingType::Shaded:
			RenderRasterized();
			break;
		default:
			return;
		}
	}

	void Integrator::Clear() {
		for (int i = 0; i < render_resolution.x * render_resolution.y; i++) {
			image[i] = glm::vec4(0, 0, 0, std::numeric_limits<float>::max());
		}
	}

	void Integrator::RenderRayTraced()
	{
		frame++;

		if (frame == 1) {
			Clear();
		}

		//multithreaded
#if 1
		std::for_each(std::execution::par, height_iterator.begin(), height_iterator.end(), [this](uint32_t y) {
			std::for_each(std::execution::par, width_iterator.begin(), width_iterator.end(), [this, y](uint32_t x) {

				Ray ray = active_camera->GetRay(glm::ivec2(x, y));
				glm::vec3 light = TraceRay(&ray);
				image[x + y * render_resolution.x] += glm::vec4(light, 0);

				});
	});

#else
		std::for_each(height_iterator.begin(), height_iterator.end(), [this](uint32_t y) {
			std::for_each(width_iterator.begin(), width_iterator.end(), [this, y](uint32_t x) {

				Ray ray = active_camera->GetRay(glm::ivec2(x, y));
				glm::vec3 light = TraceRay(&ray);
				image[x + y * render_resolution.x] += glm::vec4(light, 0);

				});
			});

#endif
	}

	void Integrator::RenderWireframe()
	{
		std::for_each(std::execution::par, height_iterator.begin(), height_iterator.end(), [this](uint32_t y) {
			std::for_each(std::execution::par, width_iterator.begin(), width_iterator.end(), [this, y](uint32_t x) {
				Ray ray = active_camera->GetRay(glm::ivec2(x, y));
				float t = 0.5f * (ray.d.y + 1.0f);
				glm::vec3 skylight = glm::vec3(1.0f - t) * glm::vec3(1.0, 1.0, .8) + glm::vec3(t) * glm::vec3(0.5, 0.7, 1.0);
				image[x + y * render_resolution.x] = glm::vec4(skylight * 1.075f, 0);
				});
			}
		);

		active_scene->DrawLines(render_resolution, *active_camera, wireframe_color, set_pixel_vec4);

		frame = 1;
	}

	void Integrator::RenderRasterized()
	{
		frame = 1;
		Clear();

		for (auto& prim : active_scene->primitives) {
			std::vector<std::vector<std::pair<RasterPixel, RasterPixel>>> shapes = prim.shape->GetRasterizedEdges(*active_camera);

			for (auto& shape : shapes) {
				std::unordered_map<int, std::pair<RasterPixel, RasterPixel>> points_per_scanline;
				std::vector<RasterPixel> points;

				for (auto& edge : shape) {
					edge.first.screen_positon = glm::ivec2(edge.first.normalized_position.x * render_resolution.x, edge.first.normalized_position.y * render_resolution.y);
					edge.second.screen_positon = glm::ivec2(edge.second.normalized_position.x * render_resolution.x, edge.second.normalized_position.y * render_resolution.y);

					glm::vec2 dir = edge.second.screen_positon - edge.first.screen_positon;

					float length = glm::length(dir);
					if (length == 0) continue;

					float inverseLength = 1.0f / length;
					dir *= inverseLength;

					for (float k = 0; k <= int(length+1); k++) {
						RasterPixel p = edge.first;
						p.screen_positon += glm::ivec2(dir * k);

						if (p.screen_positon.y >= render_resolution.y || p.screen_positon.y < 0) continue;

						float ratio = k * inverseLength;
						float one_minus_ratio = 1.0f - ratio;

						p.normal = edge.first.normal * one_minus_ratio + edge.second.normal * ratio;
						p.uv = edge.first.uv * one_minus_ratio + edge.second.uv * ratio;
						p.depth = edge.first.depth * one_minus_ratio + edge.second.depth * ratio;
						points.push_back(p);
					}
				}

				for (auto& point : points) {
					if (points_per_scanline.count(point.screen_positon.y) <= 0)
						points_per_scanline[point.screen_positon.y] = { point, point };
					else {
						std::pair<RasterPixel, RasterPixel>& pair = points_per_scanline[point.screen_positon.y];
						if (point.screen_positon.x < pair.first.screen_positon.x)
							pair.first = point;
						else if (point.screen_positon.x > pair.second.screen_positon.x)
							pair.second = point;
					}
				}


				std::for_each(points_per_scanline.begin(), points_per_scanline.end(), [this](std::unordered_map<int, std::pair<RasterPixel, RasterPixel>>::value_type& pair) {
					int y = pair.first;
					RasterPixel* p1 = &pair.second.first, * p2 = &pair.second.second;
					if (p1->screen_positon.x > p2->screen_positon.x) {
						std::swap<RasterPixel*>(p1, p2);
					}
					int dist = p2->screen_positon.x - p1->screen_positon.x;
					float inverse_dist = 1.0f / (float)dist;

					for (int i = 0; i < dist; i++) {
						int x = p1->screen_positon.x + i;
						if (x >= render_resolution.x || x < 0) continue;

						float ratio = i * inverse_dist;
						float one_minus_ratio = 1.0f - ratio;
						float depth = p1->depth * one_minus_ratio + p2->depth * ratio;

						int pixel = x + y * render_resolution.x;
						if (image[pixel].w <= depth) continue;

						glm::vec3 normal = p1->normal * one_minus_ratio + p2->normal * ratio;
						glm::vec2 uv = p1->uv * one_minus_ratio + p2->uv * ratio;

						const glm::vec3 lightDir(1, 1, -1);
						float gooch = (1.0f + glm::dot(lightDir, normal)) / 2.0f;
						gooch = gooch * 0.6 + 0.2;

						glm::vec3 goochDiffuse = gooch * gooch_warm + (1.0f - gooch) * gooch_cool;

						glm::vec3 reflectionDir = glm::reflect(lightDir, normal);
						float specular = glm::max(glm::dot(active_camera->GetDirection(), glm::normalize(reflectionDir)), 0.0f);
						specular = glm::pow(specular, 20);

						image[pixel] = glm::vec4(goochDiffuse + specular, depth);
					}

				});
			}


			
		}

	}


	void Integrator::OnResize(const glm::ivec2& size)
	{
		image_resolution = size;

		glm::ivec2 prev_res = render_resolution;
		render_resolution = glm::ivec2(glm::vec2(image_resolution) * image_scale);

		if (prev_res == render_resolution) return;

		delete[] image;
		image = new glm::vec4[render_resolution.x * render_resolution.y];
		delete[] output_image;
		output_image = new uint32_t[render_resolution.x * render_resolution.y];

		width_iterator.resize(render_resolution.x);
		height_iterator.resize(render_resolution.y);
		for (uint32_t i = 0; i < render_resolution.x; i++) {
			width_iterator[i] = i;
		}
		for (uint32_t i = 0; i < render_resolution.y; i++) {
			height_iterator[i] = i;
		}

		ResetFrameIndex();
	}

	uint32_t* Integrator::GetImage()
	{
		float inverse_frame = 1.0f / (float)frame;
		for (int x = 0; x < render_resolution.x; x++) {
			for (int y = 0; y < render_resolution.y; y++) {
				glm::vec4 pixel = image[x + y * render_resolution.x];
				//color = glm::sqrt(color);
				if (depth_only) {
					output_image[x + y * render_resolution.x] = ToUint(glm::vec4(pixel.w, pixel.w, pixel.w, 1.0f));
					continue;
				}
				
				output_image[x + y * render_resolution.x] = ToUint(glm::vec4(glm::vec3(pixel.r, pixel.g, pixel.b) * inverse_frame, 1.0f));
			}
		}
		DrawOverlays();
		return output_image;
	}

	glm::vec3 Integrator::TraceRay(Ray* ray, int depth) const
	{
		SurfaceInteraction interaction;

		glm::vec3 light(0.0f);
		glm::vec3 contribution(1.0f);

		while (depth < bounces) {
			depth++;

			if (!active_scene->IntersectAccel(*ray, &interaction)) {

				if (world_texture) {
					glm::vec3 spherePos = glm::normalize(ray->d);
					float theta = acos(-spherePos.y);
					float phi = atan2(-spherePos.z, spherePos.x) + PIf;
					interaction.uv = glm::vec2(phi / (2.0f * PIf), theta / PIf);
					glm::vec4 col = world_texture->Evaluate(interaction);
					light += glm::vec3(col.x, col.y, col.z);
				}
				else {
					float t = 0.5f * (ray->d.y + 1.0f);
					glm::vec3 skylight = glm::vec3(1.0f - t) * glm::vec3(1.0, 1.0, .8) + glm::vec3(t) * glm::vec3(0.5, 0.7, 1.0);
					light += skylight * 1.075f;
				}

				break;
			}

			glm::vec3 wo = interaction.wo;
			std::shared_ptr<Material> material = active_scene->materials[active_scene->primitives[interaction.primitive].material];
			glm::vec3 materialColor = material->Evaluate(&interaction, wo, ray->d, &light);

			contribution *= materialColor;

			ray->o = interaction.pos;
			if (!material->ScatterRay(interaction, ray)) {
				break;
			}
			ray->tMax = std::numeric_limits<float>::max();
		}
		
		return light * glm::clamp(contribution, 0.0f, 1.0f);
	}

	void Integrator::DrawOverlays()
	{
		switch (overlay_type) {
		case OverlayType::None:		
			return;
		case OverlayType::All:
			active_scene->DrawLines(render_resolution, *active_camera, overlay_color, set_pixel_uint32);
			break;
		case OverlayType::Selection:
			for (auto& obj : selected_objects) {
				active_scene->primitives[obj].DrawLines(render_resolution, *active_camera, overlay_color, set_pixel_uint32);
			}
			break;
		}
	}

	void Integrator::CreateIMGUI()
	{
		auto prevType = rendering_type;
		
		if (ImGui::DragFloat2("scale", glm::value_ptr(image_scale), .01, 0, 2)) {
			OnResize(image_resolution);
		}

		ImGui::Combo("Engine ?", (int*)&rendering_type, rendering_options, IM_ARRAYSIZE(rendering_options));
		
		if (prevType != rendering_type) OnResize(image_resolution);

		switch (rendering_type) {
		case MyPBRT::Integrator::RenderingType::PBR:
			ImGui::DragInt("bounces", &bounces, 1, 0, std::numeric_limits<int>::max());
			Texture::CreateTextureFromMenuFull(&selected_world_texture, &world_texture, world_texture_types);
			break;
		case MyPBRT::Integrator::RenderingType::Shaded:
			ImGui::ColorPicker3("Cool", glm::value_ptr(gooch_cool));
			ImGui::ColorPicker3("Warm", glm::value_ptr(gooch_warm));
			break;
		case MyPBRT::Integrator::RenderingType::Wireframe:
			ImGui::ColorPicker3("Color", glm::value_ptr(wireframe_color));
			break;
		}

		ImGui::Combo("Overlay ?", (int*)&overlay_type, overlay_options, IM_ARRAYSIZE(overlay_options));
		if (overlay_type == OverlayType::Selection || overlay_type == OverlayType::All) {
			ImGui::ColorPicker3("Outline Color", glm::value_ptr(overlay_color));
		}
	}

}