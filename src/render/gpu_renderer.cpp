#include "gpu_renderer.h"

#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

#define WAIT_FENCE
#ifdef WAIT_FENCE
#define WAIT_ON
#endif

cr::gpu_renderer::gpu_renderer(cr::scene *scene, const glm::ivec2 &resolution)
    : _scene(scene), _resolution(resolution)
{
    ZoneScopedN("gpu_renderer::ctor");
    // Create the shaders
    _kernels.shaders.generate = cr::opengl::create_shader(
      std::string(CRENDER_ASSET_PATH) + "shaders/pathtrace/kernel/generate.comp",
      GL_COMPUTE_SHADER);
    _kernels.generate = cr::opengl::create_program(_kernels.shaders.generate);

    _kernels.shaders.extend = cr::opengl::create_shader(
      std::string(CRENDER_ASSET_PATH) + "shaders/pathtrace/kernel/extend.comp",
      GL_COMPUTE_SHADER);
    _kernels.extend = cr::opengl::create_program(_kernels.shaders.extend);

    _kernels.shaders.shade = cr::opengl::create_shader(
      std::string(CRENDER_ASSET_PATH) + "shaders/pathtrace/kernel/shade.comp",
      GL_COMPUTE_SHADER);
    _kernels.shade = cr::opengl::create_program(_kernels.shaders.shade);

    // Create the buffers
    glGenBuffers(1, &_buffers.ray);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffers.ray);
    // A GPU ray is 16 bytes, we need 1 per pixel
    glBufferData(
      GL_SHADER_STORAGE_BUFFER,
      8 * sizeof(float) * resolution.x * resolution.y,
      nullptr,
      GL_DYNAMIC_COPY);

    glGenBuffers(1, &_buffers.intersections);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffers.intersections);
    // The size of a single intersection is 16 bytes, once again. 1 per pixel
    glBufferData(
      GL_SHADER_STORAGE_BUFFER,
      8 * sizeof(float) * resolution.x * resolution.y,
      nullptr,
      GL_DYNAMIC_COPY);

    glGenBuffers(1, &_buffers.ray_count);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffers.ray_count);
    auto zero = std::uint32_t(0);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 4, &zero, GL_DYNAMIC_COPY);

    glGenBuffers(1, &_buffers.scene);

    glGenTextures(1, &_opengl_handles.throughput);
    glBindTexture(GL_TEXTURE_2D, _opengl_handles.throughput);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGBA32F,
      static_cast<int>(_resolution.x),
      static_cast<int>(_resolution.y),
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      nullptr);
    glClearTexImage(_opengl_handles.throughput, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glGenTextures(1, &_opengl_handles.final_image);
    glBindTexture(GL_TEXTURE_2D, _opengl_handles.final_image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGBA32F,
      static_cast<int>(_resolution.x),
      static_cast<int>(_resolution.y),
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      nullptr);
    glClearTexImage(_opengl_handles.final_image, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glGenTextures(1, &_opengl_handles.colour);
    glBindTexture(GL_TEXTURE_2D, _opengl_handles.colour);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGBA32F,
      static_cast<int>(_resolution.x),
      static_cast<int>(_resolution.y),
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      nullptr);
    glClearTexImage(_opengl_handles.colour, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    _embree_ctx.device = rtcNewDevice(nullptr);
}

void cr::gpu_renderer::build()
{
    ZoneScopedN("gpu_renderer::build");
    _build_bvh();
    _current_frame = 0;
    //    _build_material_buffer();
}

void cr::gpu_renderer::render(const glm::ivec2 &resolution)
{
    // Todo: Update resolution and reload resolution dependent resources
    //    if (resolution != _resolution) _resolution = resolution;

    ZoneScopedN("gpu_renderer::render");
    kernel_generate();

    auto fired = std::uint32_t(_resolution.x * _resolution.y);
    for (auto i = 0; i < 2 && fired > 0; i++)
    {
        kernel_extend(fired);

        kernel_shade(fired, _current_frame, i);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffers.ray_count);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 4, &fired);
    }

    _current_frame++;
}

GLuint cr::gpu_renderer::texture() const
{
    return _opengl_handles.final_image;
}

void cr::gpu_renderer::_update_resolution()
{
    glBindTexture(GL_TEXTURE_2D, _opengl_handles.final_image);
    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGBA8,
      static_cast<int>(_resolution.x),
      static_cast<int>(_resolution.y),
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      nullptr);
    glClearTexImage(_opengl_handles.final_image, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, _opengl_handles.colour);
    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGBA8,
      static_cast<int>(_resolution.x),
      static_cast<int>(_resolution.y),
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      nullptr);
    glClearTexImage(_opengl_handles.colour, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, _opengl_handles.throughput);
    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGBA32F,
      static_cast<int>(_resolution.x),
      static_cast<int>(_resolution.y),
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      nullptr);
    glClearTexImage(_opengl_handles.throughput, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
}

void cr::gpu_renderer::_build_bvh()
{
    ZoneScopedN("gpu_renderer::build_bvh");

    auto primitives = std::vector<RTCBuildPrimitive>();
    auto sorted_gpu_prims = std::vector<gpu_triangle>();
    auto gpu_prims = std::vector<gpu_triangle>();

    const auto &geometries = _scene->registry()->entities.view<cr::entity::geometry>();

    for (auto i = 0; i < geometries.size(); i++)
    {
        const auto &geometry =
          _scene->registry()->entities.get<cr::entity::geometry>(geometries[i]);

        primitives.reserve(primitives.size() + geometry.vert_coords->size() / 3);
        sorted_gpu_prims.reserve(primitives.size() + geometry.vert_coords->size() / 3);
        gpu_prims.reserve(primitives.size() + geometry.vert_coords->size() / 3);
        for (auto j = 0; j < geometry.vert_coords->size() / 3; j++)
        {
            const auto vertices = std::array<glm::vec3, 3>({
                                                             (*geometry.vert_coords)[j * 3 + 0],
                                                             (*geometry.vert_coords)[j * 3 + 1],
                                                             (*geometry.vert_coords)[j * 3 + 2],
                                                           });

            const auto max = glm::max(vertices[0], glm::max(vertices[1], vertices[2]));
            const auto min = glm::min(vertices[0], glm::min(vertices[1], vertices[2]));

            auto gpu_prim = gpu_triangle();
            gpu_prim.v0 = glm::vec4(vertices[0], 0.0);
            gpu_prim.v1 = glm::vec4(vertices[1], 0.0);
            gpu_prim.v2 = glm::vec4(vertices[2], 0.0);

            auto primitive    = RTCBuildPrimitive();
            primitive.lower_x = min.x;
            primitive.lower_y = min.y;
            primitive.lower_z = min.z;
            primitive.upper_x = max.x;
            primitive.upper_y = max.y;
            primitive.upper_z = max.z;

            primitive.geomID = i;
            primitive.primID = j;

            gpu_prims.push_back(gpu_prim);
            primitives.push_back(primitive);
        }
    }

    // 30MB should be plenty enough
    primitives.reserve(primitives.size() + 30000000);

    auto bvh = rtcNewBVH(_embree_ctx.device);

    auto node_count = size_t(0);

    auto arguments                   = rtcDefaultBuildArguments();
    arguments.byteSize               = sizeof(arguments);
    arguments.buildFlags             = RTC_BUILD_FLAG_NONE;
    arguments.buildQuality           = RTC_BUILD_QUALITY_HIGH;
    arguments.maxBranchingFactor     = 2;
    arguments.maxDepth               = 1024;
    arguments.sahBlockSize           = 1;
    arguments.minLeafSize            = 1;
    arguments.maxLeafSize            = 16;
    arguments.traversalCost          = 1.0f;
    arguments.intersectionCost       = 1.0f;
    arguments.bvh                    = bvh;
    arguments.primitives             = primitives.data();
    arguments.primitiveCount         = primitives.size();
    arguments.primitiveArrayCapacity = primitives.capacity();
    arguments.createNode             = cr::embree_node::create_node;
    arguments.setNodeChildren        = cr::embree_node::set_children;
    arguments.setNodeBounds          = cr::embree_node::set_bounds;
    arguments.createLeaf             = cr::embree_node::create_leaf;
    arguments.splitPrimitive         = cr::split_primitive;
    arguments.buildProgress          = nullptr;
    arguments.userPtr                = &node_count;

    const auto root = static_cast<cr::embree_node *>(rtcBuildBVH(&arguments));

    // Place the BVHs into a single array
    auto flat_nodes = std::vector<bvh_node>();
    flat_nodes.reserve(node_count);

    auto node = cr::bvh_node(root->bounds[0].merge(root->bounds[1]));
    flat_nodes.emplace_back(node);
    flatten_nodes(flat_nodes, 0, root, sorted_gpu_prims, gpu_prims);

    if (auto buffer = _opengl_handles.bvh_data_buffer; buffer != ~0) glDeleteBuffers(1, &buffer);
    glGenBuffers(1, &_opengl_handles.bvh_data_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _opengl_handles.bvh_data_buffer);
    glBufferData(
      GL_SHADER_STORAGE_BUFFER,
      flat_nodes.size() * sizeof(float) * 8,
      flat_nodes.data(),
      GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    if (auto buffer = _opengl_handles.triangle_data_buffer; buffer != ~0) glDeleteBuffers(1, &buffer);
    glGenBuffers(1, &_opengl_handles.triangle_data_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _opengl_handles.triangle_data_buffer);
    glBufferData(
      GL_SHADER_STORAGE_BUFFER,
      sorted_gpu_prims.size() * sizeof(float) * 12,
      sorted_gpu_prims.data(),
      GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    /*
     *     bvh::SweepSahBuilder<bvh::Bvh<float>> builder(bvh);
    auto [bboxes, centers] = bvh::compute_bounding_boxes_and_centers(bvh_triangles.data(),
    bvh_triangles.size()); auto global_bbox = bvh::compute_bounding_boxes_union(bboxes.get(),
    bvh_triangles.size()); builder.build(global_bbox, bboxes.get(), centers.get(),
    bvh_triangles.size());
    auto bvh = bvh::Bvh<float>();

    auto        triangles  = std::vector<bvh::Triangle<float>>();
    auto gpu_triangles = std::vector<gpu_triangle>();
    const auto &geometries = _scene->registry()->entities.view<cr::entity::geometry>();

    for (auto i = 0; i < geometries.size(); i++)
    {
        const auto &geometry =
          _scene->registry()->entities.get<cr::entity::geometry>(geometries[i]);

        triangles.reserve(triangles.size() + geometry.vert_coords->size() / 3);
        gpu_triangles.reserve(gpu_triangles.size() + geometry.vert_coords->size() / 3);
        for (auto j = 0; j < geometry.vert_coords->size() / 3; j++)
        {
            auto verts = std::array<bvh::Vector3<float>, 3>();

            auto gpu_tri = gpu_triangle();
            gpu_tri.v0 = glm::vec4((*geometry.vert_coords)[j * 3 + 0], 0.0);
            gpu_tri.v1 = glm::vec4((*geometry.vert_coords)[j * 3 + 1], 0.0);
            gpu_tri.v2 = glm::vec4((*geometry.vert_coords)[j * 3 + 2], 0.0); // Shrug, no material ID yet

            gpu_triangles.push_back(gpu_tri);

            for (auto k = 0; k < 3; k++)
            {
                const auto coords = (*geometry.vert_coords)[j * 3 + k];

//                fmt::print("Tri Vertex X: {}, Y: {}, Z: {}\n", coords.x, coords.y, coords.z);

                for (auto l = 0; l < 3; l++)
                    verts[k].values[l] = coords[l];
            }

            triangles.emplace_back(verts[0], verts[1], verts[2]);
        }
    }

    auto builder          = bvh::SweepSahBuilder<bvh::Bvh<float>>(bvh);
    builder.max_leaf_size = 1;
    auto [boxes, centers] = bvh::compute_bounding_boxes_and_centers(triangles.data(), triangles.size());
    auto global_bbx       = bvh::compute_bounding_boxes_union(boxes.get(), triangles.size());
    builder.build(global_bbx, boxes.get(), centers.get(), triangles.size());

    // Flatten the nodes... but they're pretty good already so almost there.
    auto flat_nodes = std::vector<bvh_node>();
    flat_nodes.reserve(bvh.node_count);

    for (auto i = 0; i < bvh.node_count; i++)
    {
        const auto cpu_node = bvh.nodes[i];
        const auto node_bounds = cpu_node.bounds;

        const auto bounds = cr::bbox(
          glm::vec3(node_bounds[0], node_bounds[2], node_bounds[4]),
          glm::vec3(node_bounds[1], node_bounds[3], node_bounds[5]));

        auto gpu_node = bvh_node(bounds);
        gpu_node.set_primitive_count(cpu_node.primitive_count);
        gpu_node.set_primitive_first_id(cpu_node.first_child_or_primitive);
        flat_nodes.push_back(gpu_node);
    }

      if (auto buffer = _opengl_handles.bvh_data_buffer; buffer != ~0) glDeleteBuffers(1, &buffer);
    glGenBuffers(1, &_opengl_handles.bvh_data_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _opengl_handles.bvh_data_buffer);
    glBufferData(
      GL_SHADER_STORAGE_BUFFER,
      flat_nodes.size() * 32,
      flat_nodes.data(),
      GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    if (auto buffer = _opengl_handles.triangle_data_buffer; buffer != ~0)
        glDeleteBuffers(1, &buffer);
    glGenBuffers(1, &_opengl_handles.triangle_data_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _opengl_handles.triangle_data_buffer);
    glBufferData(
      GL_SHADER_STORAGE_BUFFER,
      gpu_triangles.size() * sizeof(float) * 12,
      gpu_triangles.data(),
      GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    */
}

void cr::gpu_renderer::_build_material_buffer()
{
    //    auto material_buffer =

    if (auto buffer = _opengl_handles.material_data_buffer; buffer != ~0)
        glDeleteBuffers(1, &buffer);
    glGenBuffers(1, &_opengl_handles.material_data_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _opengl_handles.material_data_buffer);
    //    glBufferData(
    //      GL_SHADER_STORAGE_BUFFER,
    //      triangles.size() * sizeof(float) * 12,
    //      triangles.data(),
    //      GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void cr::gpu_renderer::kernel_generate()
{
    ZoneScoped;
    TracyGpuZone("Generate");
    glUseProgram(_kernels.generate);

    // Update the camera data
    auto rendering_resources = cr::gpu_renderer::rendering_resources();
    rendering_resources.mvp  = _scene->registry()->camera()->mat4();
    std::memcpy(
      glm::value_ptr(rendering_resources.camera_data),
      glm::value_ptr(_resolution),
      sizeof(_resolution));
    rendering_resources.camera_data.z = glm::radians(_scene->registry()->camera()->fov);

    if (rendering_resources != _previous_resources) _current_frame = 0;
    _previous_resources = rendering_resources;

    glBindBuffer(GL_UNIFORM_BUFFER, _buffers.scene);
    glBufferData(GL_UNIFORM_BUFFER, 20 * sizeof(float), &rendering_resources, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, _buffers.scene);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffers.ray);

    // We will use the generate shader to reset the throughput and colour accumulation buffer.
    // as we do a pass over all of them, it'll make it faster (i think)

    glBindImageTexture(0, _opengl_handles.throughput, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(1, _opengl_handles.colour, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(2, _opengl_handles.final_image, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    glDispatchCompute(_resolution.x / 8, _resolution.y / 8, 1);
    WAIT_ON;
}

void cr::gpu_renderer::kernel_extend(std::uint32_t fired_rays)
{
    ZoneScopedN("gpu_renderer::extend");
    TracyGpuZone("Extend");
    glUseProgram(_kernels.extend);

    // Update the amount of rays here
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffers.ray_count);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 4, &fired_rays, GL_DYNAMIC_COPY);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffers.ray);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _buffers.intersections);

    if (auto buf = _opengl_handles.bvh_data_buffer; buf != ~0)
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, buf);

    if (auto buf = _opengl_handles.triangle_data_buffer; buf != ~0)
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, buf);

    glUniform1ui(0, fired_rays);

    glDispatchCompute((_resolution.x * _resolution.y) / 64, 1, 1);
    WAIT_ON;
}

void cr::gpu_renderer::kernel_shade(
  std::uint32_t intersections,
  std::uint32_t current_frame,
  std::uint32_t current_bounce)
{
    ZoneScopedN("gpu_renderer::shade");
    TracyGpuZone("Shade");
    glUseProgram(_kernels.shade);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffers.ray);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _buffers.intersections);

    // Reset the amount of rays
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffers.ray_count);
    const auto zero = std::uint32_t(0);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 4, &zero, GL_DYNAMIC_COPY);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _buffers.ray_count);

    glBindImageTexture(0, _opengl_handles.throughput, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(1, _opengl_handles.colour, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(2, _opengl_handles.final_image, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    glUniform2i(0, _resolution.x, _resolution.y);
    glUniform1ui(1, intersections);
    glUniform3i(2, current_frame, current_bounce, 2);

    glDispatchCompute((_resolution.x * _resolution.y) / 64, 1, 1);
    WAIT_ON;
}

void cr::gpu_renderer::flatten_nodes(
  std::vector<bvh_node> &          nodes,
  size_t                           flat_parent_idx,
  cr::embree_node *                parent,
  std::vector<gpu_triangle> &      new_prims,
  const std::vector<gpu_triangle> &prims)
{
    const auto child_index = nodes.size();
    nodes[flat_parent_idx].set_child_node_start(child_index);

    for (auto i = 0; i < 2; i++)
    {
        auto node = bvh_node(parent->children[i]->bounds[0].merge(parent->children[i]->bounds[1]));
        if (parent->children[i]->is_leaf)
        {
            for (auto &id : parent->children[i]->ids)
            {
                const auto triangle = prims[id];
                new_prims.push_back(triangle);
                id = new_prims.size() - 1;
            }
            node.set_primitive_count(parent->children[i]->ids.size());
            node.set_primitive_first_id(parent->children[i]->ids[0]);
        }
        nodes.emplace_back(node);
    }

    for (auto i = 0; i < 2; i++)
        if (!parent->children[i]->is_leaf)
            flatten_nodes(nodes, child_index + i, parent->children[i], new_prims, prims);
}
