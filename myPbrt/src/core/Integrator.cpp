#include "Integrator.h"

#include "Interaction.h"
#include "Scene.h"
#include "Material.h"
#include "Object.h"
#include "Mesh.h"
#include "Light.h"

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
		active_camera = &camera;
		active_scene = &scene;

		switch (rendering_type) {
		case RenderingType::PBR:
			RenderRayTraced();
			break;
		case RenderingType::Wireframe:
			RenderWireframe();
			break;
		case RenderingType::Rasterized:
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

	glm::vec3 Integrator::TraceRay(Ray* ray, int depth) const
	{
		SurfaceInteraction interaction;
		interaction.wo = glm::vec3(-1.0f);
		glm::vec3 color(0.0f);
		glm::vec3 contribution(1.0f);

		float prev_pdf = 1;

		while (depth < bounces) {
			depth++;

			if (!active_scene->IntersectAccel(*ray, &interaction)) {

				if (world_texture) {
					glm::vec3 spherePos = glm::normalize(ray->d);
					float theta = acos(-spherePos.y);
					float phi = atan2(-spherePos.z, spherePos.x) + PIf;
					interaction.uv = glm::vec2(phi / (2.0f * PIf), theta / PIf);
					glm::vec4 col = world_texture->Evaluate(interaction);
					color += glm::vec3(col.x, col.y, col.z);
				}
				else {
					float t = 0.5f * (ray->d.y + 1.0f);
					glm::vec3 skylight = glm::vec3(1.0f - t) * glm::vec3(1.0, 1.0, .8) + glm::vec3(t) * glm::vec3(0.5, 0.7, 1.0);
					color += skylight * 1.075f;
				}

				break;
			}

			if (interaction.front_face == false) {
				interaction.normal = -interaction.normal;
			}

			if (active_scene->materials.size() == 0) return glm::vec3(1, 0, 1);
			const std::shared_ptr<Material>& material = active_scene->materials[active_scene->objects[interaction.primitive].material];
			
			color += material->EvaluateLight(interaction);
			glm::vec3 materialColor = material->Evaluate(&interaction);

			bool has_pdf = false;
			if (!material->ScatterRay(interaction, ray->d, has_pdf)) {
				break;
			}

			if (has_pdf) {
				glm::vec3 d = ray->d;
				if (active_scene->lights.size() > 0)
				{
					int index = random_int(0, active_scene->lights.size() - 1);
					const std::shared_ptr<Light>& light = active_scene->lights[index];
					glm::vec3 point_on_light = light->Sample(interaction);
					ray->d = point_on_light - interaction.pos;

					if (glm::dot(interaction.normal, ray->d) < 0) goto material;
					ray->tMax = glm::distance(point_on_light, interaction.pos);
					if (active_scene->hasIntersectionsAccel(*ray)) goto material;

					float light_pdf = light->PDF_Value(interaction, ray->d);
					color += light->Color() / light_pdf;
				}
			material:
				ray->d = d;
				contribution *= materialColor / prev_pdf;
				prev_pdf = material->Pdf_Value(ray->d, interaction.normal);
			}
			else {
				contribution *= materialColor / prev_pdf;
				prev_pdf = 1;
			}

			ray->o = interaction.pos + ray->d * 0.0001f;
			ray->tMax = std::numeric_limits<float>::max();
		}

		return color * glm::clamp(contribution, 0.0f, 1.0f);
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

		//every primitive gives us back a vector of shapes (=a vector of edges) which are rasterized first with bersenhem 
		//and then the scanlines are filled, normals, uv and depth are linearly interpolated
		for (int obj = 0; obj < active_scene->objects.size(); obj++) {
			std::vector<std::vector<std::pair<RasterPixel, RasterPixel>>> shapes = active_scene->ObjectToMesh(obj).GetRasterizedEdges(*active_camera);
			
			//cull shapes that are outside the view 
			for (int i = 0; i < shapes.size(); i++) {
				bool inside = false;
				for (auto& edge : shapes[i]) {
					edge.first.screen_positon = glm::ivec2(edge.first.normalized_position.x * render_resolution.x, edge.first.normalized_position.y * render_resolution.y);
					inside |= edge.first.screen_positon.x > 0 && edge.first.screen_positon.x < render_resolution.x;
					inside |= edge.first.screen_positon.y > 0 && edge.first.screen_positon.y < render_resolution.y;
				
					edge.second.screen_positon = glm::ivec2(edge.second.normalized_position.x * render_resolution.x, edge.second.normalized_position.y * render_resolution.y);
					inside |= edge.second.screen_positon.x > 0 && edge.second.screen_positon.x < render_resolution.x;
					inside |= edge.second.screen_positon.y > 0 && edge.second.screen_positon.y < render_resolution.y;
				}
				if (!inside) {
					shapes.erase(shapes.begin() + i);
					i--;
				}
			}

			//get all edges and then fill in scanlines
			for (auto& shape : shapes) {
				std::vector<RasterPixel> points;

				//get and interpolate all the points on every edge
				for (auto& edge : shape) {
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

				//get the left and rightmost points on every scanline

				std::unordered_map<int, std::pair<RasterPixel, RasterPixel>> points_per_scanline;

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

				//fill in the space between the left and rightmost points, 
				//lineraly interpolate the data then use gooch shading and set the pixel color

				std::for_each(std::execution::par, points_per_scanline.begin(), points_per_scanline.end(), [this](std::unordered_map<int, std::pair<RasterPixel, RasterPixel>>::value_type& pair) {
					int y = pair.first;
					RasterPixel* p1 = &pair.second.first, * p2 = &pair.second.second;
					if (p1->screen_positon.x > p2->screen_positon.x) {
						std::swap<RasterPixel*>(p1, p2);
					}
					int dist = p2->screen_positon.x - p1->screen_positon.x;
					float inverse_dist = 1.0f / (float)dist;

					//cull if both xs are out of range
					if (p2->screen_positon.x < 0 || p1->screen_positon.x > render_resolution.x) return;

					for (int i = 0; i < dist; i++) {
						int x = p1->screen_positon.x + i;
						if (x < 0) {
							i += -x;
							continue;
						}
						if (x >= render_resolution.x) break;

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

	uint32_t* Integrator::GetImage(bool overlays)
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
		if (overlays)
			DrawOverlays();
		return output_image;
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
				active_scene->ObjectToMesh(obj).DrawLines(render_resolution, *active_camera, overlay_color, set_pixel_uint32);
			}
			break;
		}
	}

	void Integrator::CreateIMGUI()
	{
		auto prevType = rendering_type;

		ImGui::Combo("Engine ?", (int*)&rendering_type, rendering_options, IM_ARRAYSIZE(rendering_options));
		if (prevType != rendering_type) OnResize(image_resolution);
		
		switch (rendering_type) {
		case MyPBRT::Integrator::RenderingType::PBR:
			ImGui::DragInt("bounces", &bounces, 1, 0, std::numeric_limits<int>::max());
			Texture::CreateTextureFromMenuFull(&selected_world_texture, &world_texture, world_texture_types);
			break;
		case MyPBRT::Integrator::RenderingType::Rasterized:
			ImGui::ColorEdit3("Cool", glm::value_ptr(gooch_cool));
			ImGui::ColorEdit3("Warm", glm::value_ptr(gooch_warm));
			break;
		case MyPBRT::Integrator::RenderingType::Wireframe:
			ImGui::ColorEdit3("Color", glm::value_ptr(wireframe_color));
			break;
		}

		ImGui::Text((std::to_string(frame) + " samples").c_str());

		if (ImGui::DragFloat2("scale", glm::value_ptr(image_scale), .01, 0.01, 2)) {
			OnResize(image_resolution);
		}

		ImGui::Combo("Overlay ?", (int*)&overlay_type, overlay_options, IM_ARRAYSIZE(overlay_options));
		if (ImGui::IsItemHovered()) {
			ImGui::SetTooltip("selection outlines");
		}
		if (overlay_type == OverlayType::Selection || overlay_type == OverlayType::All) {
			ImGui::ColorEdit3("Outline Color", glm::value_ptr(overlay_color));
		}
	}

}