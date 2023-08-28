#include "Mesh.h"

#include "Interaction.h"
#include "Camera.h"
#include "Texture.h"

#include <imgui.h>
#include <set>

#include <fstream>

namespace MyPBRT {

    float TriangleArea(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2)
    {
        //half of |AB x AC|
        float area = 0.5f * glm::length(glm::cross(p1 - p0, p2 - p0));
        return area;
    }

    std::shared_ptr<Mesh> Mesh::ParseMesh(const Json::Value& node)
    {
        std::shared_ptr<Mesh> ret(new Mesh({}, {}));
        ret->DeSerialize(node);
        return ret;
    }

    /*
    //bool Sphere::Intersect(const Ray& r, SurfaceInteraction* interaction, bool testAlphaTexture) const
    //{
    //    glm::vec3 origin = r.o - position;

    //    double a = glm::dot(r.d, r.d);
    //    double b = 2.0f * glm::dot(origin, r.d);
    //    double c = glm::dot(origin, origin) - radius * radius;

    //    double discriminant = b * b - 4.0 * a * c;
    //    if (discriminant < 0.0f)
    //        return false;

    //    double closestT = (-b - glm::sqrt(discriminant)) / (2.0 * a);
    //    if (closestT < 0.0001 || closestT > r.tMax) 
    //        return false;
    //    r.tMax = closestT;

    //    interaction->pos = origin + r.d * r.tMax;
    //    interaction->normal = glm::normalize(interaction->pos);
    //    float theta = acos(-interaction->normal.y);
    //    float phi = atan2(-interaction->normal.z, -interaction->normal.x) + PIf;
    //    float u = phi * .5f * INVPIf;
    //    float v = theta * INVPIf;
    //    interaction->uv = glm::vec2(u, v);

    //    if (radius < 0)
    //        interaction->normal = -interaction->normal;
    //    interaction->pos += position;
    //    interaction->front_face = glm::dot(r.d, interaction->normal) < 0.0f;

    //    return true;
    //}

    //bool Sphere::hasIntersections(const Ray& r, bool testAlphaTexture) const
    //{
    //    glm::vec3 origin = r.o - position;

    //    float a = glm::dot(r.d, r.d);
    //    float b = 2.0f * glm::dot(origin, r.d);
    //    float c = glm::dot(origin, origin) - radius * radius;

    //    float discriminant = b * b - 4.0f * a * c;
    //    if (discriminant < 0.0f)
    //        return false;

    //    float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a);
    //    if (closestT < 0 || closestT > r.tMax)
    //        return false;

    //    return true;
    //}

    //float Sphere::Area() const
    //{
    //    return 2 * PI * radius;
    //}

    //bool Sphere::CreateIMGUI(const std::vector<std::shared_ptr<Material>>& materials)
    //{
    //    bool changed = false;
    //    changed |= ImGui::DragFloat3("position", glm::value_ptr(position), .01, -10000, 10000);
    //    changed |= ImGui::DragFloat("radius", &radius, .01, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
    //    if (changed)
    //        bounds = Bounds(position - radius, position + radius);
    //    return changed;
    //}

    //void Sphere::DrawLines(const glm::vec2& resolution, const Camera& camera, const glm::vec3& color, IntegratorSetPixelFunctionPtr set_function) const
    //{
    //    glm::vec4 pos = camera.GetView() * glm::vec4(position,1);
    //    pos = camera.GetProjection() * pos;
    //    float w = pos.w;
    //    if (w == 0) return;
    //    pos /= w;
    //    pos += 1;
    //    pos *= 0.5;
    //    if (pos.z > 1) return;
    //    if (pos.x < 0 || pos.x > 1) return;
    //    if (pos.y < 0 || pos.y > 1) return;

    //    glm::ivec2 screenPosition = glm::ivec2(pos.x * resolution.x, pos.y * resolution.y);
    //    float screenRadius = (radius * resolution.y) / w;
    //    screenRadius *= 1.1;
    //    set_function(screenPosition.x, screenPosition.y, glm::vec4(color, 0.0f));

    //    float circumference = 2 * PIf * screenRadius;
    //    float ratio = (2*PIf) / circumference;

    //    for (int i = 0; i <= circumference; i++) {
    //        glm::ivec2 pos = screenPosition + 1;
    //        pos.x += cos(i * ratio) * screenRadius;
    //        pos.y += sin(i * ratio) * screenRadius;
    //        if (pos.x > 0 && pos.x < resolution.x && pos.y > 0 && pos.y < resolution.y)
    //            set_function(pos.x, pos.y, glm::vec4(color, 0.0f));
    //    }
    //}
    //void Sphere::Translate(const glm::vec3& vec)
    //{
    //    position += vec;
    //    bounds.Translate(vec);
    //}
    //void Sphere::RotateAround(const glm::quat& rotation, const glm::vec3& pivot)
    //{
    //    glm::vec3 prevpos = position;
    //    position = pivot + rotation * (position - pivot);
    //    bounds.Translate(position - prevpos);
    //}
    //glm::vec3 Sphere::Sample(const SurfaceInteraction& interaction) const
    //{
    //    glm::vec3 dir = position - interaction.pos;
    //    auto distance_squared = glm::length2(dir);

    //    glm::vec3 unit_w = glm::normalize(dir);
    //    glm::vec3 a = (fabs(unit_w.x) > 0.9) ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
    //    glm::vec3 v = glm::normalize(glm::cross(unit_w, a));
    //    glm::vec3 u = glm::cross(unit_w, v);

    //    glm::vec3 random = random_to_sphere(radius, distance_squared);

    //    return random.x * u + random.y * v + random.z * unit_w;
    //}
    //float Sphere::PDF_Value(const SurfaceInteraction& interacton, const glm::vec3& direction) const
    //{
    //    glm::vec3 dir = position - interacton.pos;
    //    auto cos_theta_max = sqrt(1.0f - radius * radius / glm::length2(dir));
    //    auto solid_angle = 2 * PIf * (1 - cos_theta_max);

    //    return  1 / solid_angle;
    //}
   */
    Mesh::Mesh(const std::vector<Vertex>& _vertices, const std::vector<uint32_t>& _indices)
        : vertices(_vertices), indices(_indices) {
        std::unordered_map<int, int> edgesMap;
        for (int i = 0; i < indices.size(); i += 3) {
            edgesMap[indices[i]] = indices[i + 1];
            edgesMap[indices[i+1]] = indices[i + 2];
            edgesMap[indices[i+2]] = indices[i];
        }
        for (auto& edge : edgesMap) {
            edges.push_back(std::make_pair(edge.first, edge.second));
        }
        ApplyTransformation();
    }
    void Mesh::Preprocess()
    {
    }
    bool Mesh::Intersect(const Ray& ray, SurfaceInteraction* interaction, bool testAlphaTexture) const
    {
        return accel.Intersect(0, ray, interaction, triangle_intersection_func);
    }
    bool Mesh::hasIntersections(const Ray& ray, bool testAlphaTexture) const
    {
        bool hit = false;

        for (int i = 0; i < indices.size(); i += 3) {
            const uint32_t i0 = indices[i], i1 = indices[i + 1], i2 = indices[i + 2];
            const Vertex* v0 = &transformed_vertices[i0], * v1 = &transformed_vertices[i1], * v2 = &transformed_vertices[i2];
            const glm::vec3 normal = v0->normal;

            // > because ray.d in incoming not outgoing
            if (glm::dot(v0->normal, ray.d) > 0) {
                continue;
            }

            //t - translated
            glm::vec3 p0t = v0->position - ray.o;
            glm::vec3 p1t = v1->position - ray.o;
            glm::vec3 p2t = v2->position - ray.o;

            int kz = MaxDimension(glm::abs(ray.d));
            int kx = kz + 1; if (kx == 3) kx = 0;
            int ky = kx + 1; if (ky == 3) ky = 0;

            //direction pointing towards z
            glm::vec3 d = glm::vec3(ray.d[kx], ray.d[ky], ray.d[kz]);
            p0t = glm::vec3(p0t[kx], p0t[ky], p0t[kz]);
            p1t = glm::vec3(p1t[kx], p1t[ky], p1t[kz]);
            p2t = glm::vec3(p2t[kx], p2t[ky], p2t[kz]);

            //first shear xy, z only if actual intersection
            float shearedZ = 1.0f / d.z;
            float shearedX = -d.x * shearedZ;
            float shearedY = -d.y * shearedZ;
            p0t.x += shearedX * p0t.z;
            p0t.y += shearedY * p0t.z;
            p1t.x += shearedX * p1t.z;
            p1t.y += shearedY * p1t.z;
            p2t.x += shearedX * p2t.z;
            p2t.y += shearedY * p2t.z;

            float e0 = p1t.x * p2t.y - p1t.y * p2t.x;
            float e1 = p2t.x * p0t.y - p2t.y * p0t.x;
            float e2 = p0t.x * p1t.y - p0t.y * p1t.x;

            //signs differ
            if ((e0 < 0 || e1 < 0 || e2 < 0) && (e0 > 0 || e1 > 0 || e2 > 0))
                continue;
            float det = e0 + e1 + e2;
            if (det == 0)
                continue;

            //shear z
            p0t.z *= shearedZ;
            p1t.z *= shearedZ;
            p2t.z *= shearedZ;

            //avoid floating point division
            float tScaled = e0 * p0t.z + e1 * p1t.z + e2 * p2t.z;
            if (det < 0 && (tScaled >= 0 || tScaled < ray.tMax * det))
                continue;
            else if (det > 0 && (tScaled <= 0 || tScaled > ray.tMax * det))
                continue;

            return true;
        }

        return false;
    }
    float Mesh::Area() const
    {
        return 0.0f;
    }
    bool Mesh::CreateIMGUI()
    {
        bool changed = false;

        changed |= ImGui::DragFloat3("position", glm::value_ptr(position), 0.01, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
        
        glm::vec3 rotationEuler = glm::degrees(glm::eulerAngles(rotation));
        changed |= ImGui::DragFloat3("rotation", glm::value_ptr(rotationEuler), 0.1, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
        rotation = glm::quat(glm::radians(rotationEuler));

        changed |= ImGui::DragFloat3("scale", glm::value_ptr(scale), 0.01, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());

        const std::vector<Texture::TextureType> normal_map_texture_types = { Texture::TextureType::Image };
        Texture::CreateTextureFromMenuFull(&selected_normal_map_texture, &normal_map, normal_map_texture_types);
        ImGui::DragFloat("normal map strength", &normal_map_strength, .01, 0, std::numeric_limits<float>::max());
        if (changed) {
            ApplyTransformation();
        }
        return changed;
    }
    void Mesh::DrawLines(const glm::vec2& resolution, const Camera& camera, const glm::vec3& color, IntegratorSetPixelFunctionPtr set_function) const
    {
        for (auto& edge : edges) {
            glm::vec4 pos1 = camera.GetView() * glm::vec4(transformed_vertices[edge.first].position, 1);
            pos1 = camera.GetProjection() * pos1;
            float inverseW1 = 1.0f / pos1.w;
            pos1 *= inverseW1;
            pos1 += 1;
            pos1 *= 0.5;

            glm::vec4 pos2 = camera.GetView() * glm::vec4(transformed_vertices[edge.second].position, 1);
            pos2 = camera.GetProjection() * pos2;
            float inverseW2 = 1.0f / pos2.w;
            pos2 *= inverseW2;
            pos2 += 1;
            pos2 *= 0.5;

            if (pos1.z > 1 && pos2.z > 1) continue;
            if ((pos1.x < 0 || pos1.x > 1) && (pos2.x < 0 || pos2.x > 1)) continue;
            if ((pos1.y < 0 || pos1.y > 1) && (pos2.y < 0 || pos2.y > 1)) continue;

            glm::vec2 screenPosition2 = glm::ivec2(pos2.x * resolution.x, pos2.y * resolution.y);
            glm::vec2 screenPosition1 = glm::ivec2(pos1.x * resolution.x, pos1.y * resolution.y);

            glm::vec2 dir = screenPosition2 - screenPosition1;

            float length = glm::length(dir);
            if (length == 0) continue;

            float inverseLength = 1.0f / length;
            dir *= inverseLength;

            for (float k = 0; k <= int(length); k++) {
                glm::ivec2 pos2 = screenPosition1 + dir * k;
                if (pos2.x <= 0.0f || pos2.y <= 0.0f || pos2.x >= resolution.x || pos2.y >= resolution.y) continue;
                set_function(pos2.x, pos2.y, glm::vec4(color, 0.0f));
            }
        }
    }
    void Mesh::Translate(const glm::vec3& vec)
    { 
        position += vec;
        ApplyTransformation();
    }
    void Mesh::RotateAround(const glm::quat& rotation_offset, const glm::vec3& pivot)
    {
        position = pivot + rotation_offset * (position - pivot);
        rotation = rotation_offset * rotation;
        ApplyTransformation();
    }
    bool Mesh::IntersectTriangle(const Ray& ray, SurfaceInteraction* interaction, int object) const
    {
        object *= 3;
        const uint32_t i0 = indices[object], i1 = indices[object + 1], i2 = indices[object + 2];
        const Vertex* v0 = &transformed_vertices[i0], * v1 = &transformed_vertices[i1], * v2 = &transformed_vertices[i2];

        //t - translated
        glm::vec3 p0t = v0->position - ray.o;
        glm::vec3 p1t = v1->position - ray.o;
        glm::vec3 p2t = v2->position - ray.o;

        int kz = MaxDimension(glm::abs(ray.d));
        int kx = kz + 1; if (kx == 3) kx = 0;
        int ky = kx + 1; if (ky == 3) ky = 0;

        //direction pointing towards z
        glm::vec3 d = glm::vec3(ray.d[kx], ray.d[ky], ray.d[kz]);
        p0t = glm::vec3(p0t[kx], p0t[ky], p0t[kz]);
        p1t = glm::vec3(p1t[kx], p1t[ky], p1t[kz]);
        p2t = glm::vec3(p2t[kx], p2t[ky], p2t[kz]);

        //first shear xy, z only if actual intersection
        float shearedZ = 1.0f / d.z;
        float shearedX = -d.x * shearedZ;
        float shearedY = -d.y * shearedZ;
        p0t.x += shearedX * p0t.z;
        p0t.y += shearedY * p0t.z;
        p1t.x += shearedX * p1t.z;
        p1t.y += shearedY * p1t.z;
        p2t.x += shearedX * p2t.z;
        p2t.y += shearedY * p2t.z;

        float e0 = p1t.x * p2t.y - p1t.y * p2t.x;
        float e1 = p2t.x * p0t.y - p2t.y * p0t.x;
        float e2 = p0t.x * p1t.y - p0t.y * p1t.x;

        //signs differ
        if ((e0 < 0 || e1 < 0 || e2 < 0) && (e0 > 0 || e1 > 0 || e2 > 0))
            return false;
        float det = e0 + e1 + e2;
        if (det == 0)
            return false;

        //shear z
        p0t.z *= shearedZ;
        p1t.z *= shearedZ;
        p2t.z *= shearedZ;

        float tScaled = e0 * p0t.z + e1 * p1t.z + e2 * p2t.z;
        if (det < 0 && (tScaled >= 0 || tScaled < ray.tMax * det))
            return false;
        else if (det > 0 && (tScaled <= 0.01 || tScaled > ray.tMax * det))
            return false;

        float invDet = 1.0f / det;
        float b0 = e0 * invDet;
        float b1 = e1 * invDet;
        float b2 = e2 * invDet;
        float t = tScaled * invDet;

        glm::vec3 hitPos = b0 * v0->position + b1 * v1->position + b2 * v2->position;
        interaction->normal = b0 * v0->normal + b1 * v1->normal + b2 * v2->normal;

        interaction->uv = b0 * v0->uv + b1 * v1->uv + b2 * v2->uv;
        interaction->pos = hitPos;
        interaction->front_face = glm::dot(ray.d, interaction->normal) < 0;
        ray.tMax = t;

        if (normal_map) {
            glm::vec4 normal = normal_map->Evaluate(*interaction);
            normal = glm::normalize(normal * 2.0f - 1.0f);
            interaction->normal += normal_map_strength * (normal.x * v0->tangent + normal.y * v0->bitangent);
            interaction->normal = glm::normalize(interaction->normal);
        }

        return true;
    }
    std::vector < std::vector<std::pair<Integrator::RasterPixel, Integrator::RasterPixel>>> Mesh::GetRasterizedEdges(const Camera& camera) const
    {
        std::vector < std::vector<std::pair<Integrator::RasterPixel, Integrator::RasterPixel>>> transformed_edges;

        for (int i = 0; i < indices.size(); i += 3) {
            const Vertex& v1 = transformed_vertices[indices[i]],
                &v2 = transformed_vertices[indices[i + 1]],
                &v3 = transformed_vertices[indices[i + 2]];

            Integrator::RasterPixel rp1, rp2, rp3;

            glm::vec4 pos1 = camera.GetProjection() * camera.GetView() * glm::vec4(v1.position, 1);
            pos1 = ((pos1 / pos1.w) + 1.0f) * .5f;
            rp1.normalized_position = pos1;
            rp1.uv = v1.uv;
            rp1.normal = v1.normal;
            rp1.depth = glm::distance2(camera.GetPosition(), v1.position);

            glm::vec4 pos2 = camera.GetProjection() * camera.GetView() * glm::vec4(v2.position, 1);
            pos2 = ((pos2 / pos2.w) + 1.0f) * .5f;
            rp2.normalized_position = pos2;
            rp2.uv = v2.uv;
            rp2.normal = v2.normal;
            rp2.depth = glm::distance2(camera.GetPosition(), v2.position);

            glm::vec4 pos3 = camera.GetProjection() * camera.GetView() * glm::vec4(v3.position, 1);
            pos3 = ((pos3 / pos3.w) + 1.0f) * .5f;
            rp3.normalized_position = pos3;
            rp3.uv = v3.uv;
            rp3.normal = v3.normal;
            rp3.depth = glm::distance2(camera.GetPosition(), v3.position);

            transformed_edges.push_back({ {rp1, rp2}, {rp1, rp3}, {rp2, rp3 } });
        }

        return transformed_edges;
    }

    void Mesh::DeSerialize(const Json::Value& node)
    {
        for (int i = 0; i < 3; i++)
            position[i] = node["position"][i].asFloat();
        for (int i = 0; i < 4; i++)
            rotation[i] = node["rotation"][i].asFloat();
        for (int i = 0; i < 3; i++)
            scale[i] = node["scale"][i].asFloat();
    }

    Json::Value Mesh::Serialize() const
    {
        Json::Value ret;
        ret["type"] = GetType();
        for(int i = 0; i < 3; i++)
            ret["position"].append(position[i]);
        for (int i = 0; i < 4; i++)
            ret["rotation"].append(rotation[i]);
        for (int i = 0; i < 3; i++)
            ret["scale"].append(scale[i]);
        if(normal_map)
            ret["normal map"] = normal_map->Serialize();
        ret["normal map strength"] = normal_map_strength;
        return ret;
    }

    void Mesh::ApplyTransformation()
    {
        triangle_areas.reserve(ceil(indices.size()/3));
        transformed_vertices.resize(vertices.size());
        glm::vec3 min(std::numeric_limits<float>::max()), max(std::numeric_limits<float>::lowest());
        
        //vertex transforms
        for (int i = 0; i < vertices.size(); i++) {
            transformed_vertices[i].position = position + rotation * (vertices[i].position * scale);
            transformed_vertices[i].normal = glm::normalize(rotation * vertices[i].normal);
            transformed_vertices[i].uv = vertices[i].uv;

            for(int j = 0; j < 3; j++){
                if (transformed_vertices[i].position[j] < min[j]) min[j] = transformed_vertices[i].position[j];
                else if (transformed_vertices[i].position[j] > max[j]) max[j] = transformed_vertices[i].position[j];
            }
        }
        
        //triangle transforms
        bounds = Bounds(min, max);
        std::vector<Bounds> all_bounds;
        for (int i = 0; i < indices.size(); i += 3) {
            glm::vec3 p0 = transformed_vertices[indices[i]].position,
                p1 = transformed_vertices[indices[i + 1]].position,
                p2 = transformed_vertices[indices[i + 2]].position;
            glm::vec3 min(std::min({ p0.x, p1.x, p2.x }), std::min({ p0.y, p1.y, p2.y}), std::min({ p0.z, p1.z, p2.z}));
            glm::vec3 max(std::max({p0.x, p1.x, p2.x}), std::max({ p0.y, p1.y, p2.y }), std::max({ p0.z, p1.z, p2.z }));
            all_bounds.push_back(Bounds(min, max));

            glm::vec2 uv0 = transformed_vertices[indices[i]].uv,
                uv1 = transformed_vertices[indices[i + 1]].uv,
                uv2 = transformed_vertices[indices[i + 2]].uv;
            glm::vec3 &t0 = transformed_vertices[indices[i]].tangent,
                &t1 = transformed_vertices[indices[i + 1]].tangent,
                &t2 = transformed_vertices[indices[i + 2]].tangent;
            glm::vec3& bt0 = transformed_vertices[indices[i]].bitangent,
                & bt1 = transformed_vertices[indices[i + 1]].bitangent,
                & bt2 = transformed_vertices[indices[i + 2]].bitangent;

            glm::vec3 e1 = p1 - p0, e2 = p2 - p0;
            glm::vec2 dUV1 = uv1 - uv0, dUV2 = uv2 - uv0;
            float invDeterminant = 1.0f / (dUV1.x * dUV2.y - dUV1.y * dUV2.x);
            t2.x = t1.x = t0.x = invDeterminant * (dUV2.y * e1.x - dUV1.y * e2.x);
            t2.y = t1.y = t0.y = invDeterminant * (dUV2.y * e1.y - dUV1.y * e2.y);
            t2.z = t1.z = t0.z = invDeterminant * (dUV2.y * e1.z - dUV1.y * e2.z);
            t0 = glm::normalize(t0);
            t1 = glm::normalize(t1);
            t2 = glm::normalize(t2);

            bt2.x = bt1.x = bt0.x = invDeterminant * (-dUV2.x * e1.x + dUV1.x * e2.x);
            bt2.y = bt1.y = bt0.y = invDeterminant * (-dUV2.x * e1.y + dUV1.x * e2.y);
            bt2.z = bt1.z = bt0.z = invDeterminant * (-dUV2.x * e1.z + dUV1.x * e2.z);
            bt0 = glm::normalize(bt0);
            bt1 = glm::normalize(bt1);
            bt2 = glm::normalize(bt2);

            float area = TriangleArea(p0, p1, p2);
            total_area += area;
            triangle_areas.push_back(area);
        }

        accel.Build(all_bounds);
    }
    void Mesh::Vertex::CreateIMGUI(const std::string& name)
    {
        ImGui::DragFloat3(name.c_str(), glm::value_ptr(position), 0.01, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
    }

    Json::Value Mesh::Vertex::Serialize() const
    {
        Json::Value ret;
        for(int i = 0; i < 3; i++)
            ret["position"].append(position[i]);
        for (int i = 0; i < 3; i++)
            ret["normal"].append(normal[i]);
        for (int i = 0; i < 2; i++)
            ret["uv"].append(uv[i]);
        return ret;
    }

    void Mesh::Vertex::DeSerialize(const Json::Value& node)
    {
        for (int i = 0; i < 3; i++)
            position[i] = node["position"][i].asFloat();
        for (int i = 0; i < 3; i++)
            normal[i] = node["normal"][i].asFloat();
        for (int i = 0; i < 2; i++)
            uv[i] = node["uv"][i].asFloat();
    }
}
