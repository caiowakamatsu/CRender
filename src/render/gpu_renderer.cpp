#include "gpu_renderer.h"

cr::gpu_renderer::gpu_renderer(cr::scene *scene, const glm::ivec2 &resolution)
    : _scene(scene), _resolution(resolution)
{
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
    glBufferData(
      GL_SHADER_STORAGE_BUFFER,
      4,
      &zero,
      GL_DYNAMIC_COPY);

    glGenBuffers(1, &_buffers.scene);

//    glGenTextures(1, &_opengl_handles.accumulation);
//    glBindTexture(GL_TEXTURE_2D, _opengl_handles.accumulation);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexImage2D(
//      GL_TEXTURE_2D,
//      0,
//      GL_RGBA8,
//      static_cast<int>(_resolution.x),
//      static_cast<int>(_resolution.y),
//      0,
//      GL_RGBA,
//      GL_UNSIGNED_BYTE,
//      nullptr);
//    glClearTexImage(_opengl_handles.target, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glGenTextures(1, &_opengl_handles.accumulation);
    glBindTexture(GL_TEXTURE_2D, _opengl_handles.accumulation);
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
    glClearTexImage(_opengl_handles.accumulation, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
}

void cr::gpu_renderer::build()
{
    //    _build_bvh();
    //    _build_triangle_buffer();
    //    _build_material_buffer();
}

void cr::gpu_renderer::render(const glm::ivec2 &resolution)
{
    // Todo: Update resolution and reload resolution dependent resources
    if (resolution != _resolution) _resolution = resolution;

    kernel_generate();

    auto fired = std::uint32_t(_resolution.x * _resolution.y);
    for (auto i = 0; i < 2; i++)
    {
        kernel_extend(fired);

        kernel_shade(fired);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffers.ray_count);
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 4, &fired);
    }

    _current_frame++;
}

GLuint cr::gpu_renderer::texture() const
{
    return _opengl_handles.accumulation;
}

void cr::gpu_renderer::_update_resolution()
{
    glBindTexture(GL_TEXTURE_2D, _opengl_handles.target);
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
    glClearTexImage(_opengl_handles.target, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, _opengl_handles.accumulation);
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
    glClearTexImage(_opengl_handles.accumulation, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
}

void cr::gpu_renderer::_build_bvh()
{
    auto primitives = std::vector<RTCBuildPrimitive>();

    const auto &geometries = _scene->registry()->entities.view<cr::entity::geometry>();

    auto triangles = std::vector<glm::vec3>();
    for (auto i = 0; i < geometries.size(); i++)
    {
        const auto &geometry =
          _scene->registry()->entities.get<cr::entity::geometry>(geometries[i]);

        primitives.reserve(primitives.size() + geometry.vert_coords->size() / 3);
        for (auto j = 0; j < geometry.vert_coords->size() / 3; j++)
        {
            const auto vertices = std::array<glm::vec3, 3>({
              (*geometry.vert_coords)[j * 3 + 0],
              (*geometry.vert_coords)[j * 3 + 1],
              (*geometry.vert_coords)[j * 3 + 2],
            });

            const auto max = glm::max(vertices[0], glm::max(vertices[1], vertices[2]));
            const auto min = glm::min(vertices[0], glm::min(vertices[1], vertices[2]));

            auto primitive    = RTCBuildPrimitive();
            primitive.lower_x = min.x;
            primitive.lower_y = min.y;
            primitive.lower_z = min.z;
            primitive.upper_x = max.x;
            primitive.upper_y = max.y;
            primitive.upper_z = max.z;

            primitive.geomID = i;
            primitive.primID = j;

            primitives.push_back(primitive);
        }
    }

    primitives.reserve(primitives.size() + 30000000);

    auto bvh = rtcNewBVH(_embree_ctx.device);

    auto node_count = size_t(0);

    auto arguments                   = rtcDefaultBuildArguments();
    arguments.byteSize               = sizeof(arguments);
    arguments.buildFlags             = RTC_BUILD_FLAG_NONE;
    arguments.buildQuality           = RTC_BUILD_QUALITY_MEDIUM;
    arguments.maxBranchingFactor     = 2;
    arguments.maxDepth               = 1024;
    arguments.sahBlockSize           = 1;
    arguments.minLeafSize            = 1;
    arguments.maxLeafSize            = 1;
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
    node.set_leaf(false);
    flat_nodes.emplace_back(node);
    bvh_node::flatten_nodes(flat_nodes, 0, root);

    if (auto buffer = _opengl_handles.bvh_data_buffer; buffer != ~0) glDeleteBuffers(1, &buffer);
    glGenBuffers(1, &_opengl_handles.bvh_data_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _opengl_handles.bvh_data_buffer);
    glBufferData(
      GL_SHADER_STORAGE_BUFFER,
      flat_nodes.size() * 32,
      flat_nodes.data(),
      GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void cr::gpu_renderer::_build_triangle_buffer()
{
    auto triangles = std::vector<gpu_triangle>();

    const auto &entities =
      _scene->registry()->entities.view<cr::entity::geometry, cr::entity::model_materials>();

    for (auto entity : entities)
    {
        const auto &geometry = _scene->registry()->entities.get<cr::entity::geometry>(entity);
        const auto &material =
          _scene->registry()->entities.get<cr::entity::model_materials>(entity);

        triangles.reserve(triangles.size() + geometry.vert_coords->size() / 3);
        for (auto j = 0; j < geometry.vert_coords->size() / 3; j++)
        {
            const auto vertices = std::array<glm::vec3, 3>({
              (*geometry.vert_coords)[j * 3 + 0],
              (*geometry.vert_coords)[j * 3 + 1],
              (*geometry.vert_coords)[j * 3 + 2],
            });

            auto triangle = gpu_triangle();
            triangle.v0   = glm::vec4(vertices[0], 0.0f);
            triangle.v1   = glm::vec4(vertices[1], 0.0f);
            triangle.v2   = glm::vec4(vertices[2], 0.0f);
            triangles.push_back(triangle);
        }
    }

    if (auto buffer = _opengl_handles.triangle_data_buffer; buffer != ~0)
        glDeleteBuffers(1, &buffer);
    glGenBuffers(1, &_opengl_handles.triangle_data_buffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _opengl_handles.triangle_data_buffer);
    glBufferData(
      GL_SHADER_STORAGE_BUFFER,
      triangles.size() * sizeof(float) * 12,
      triangles.data(),
      GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
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

    glDispatchCompute(_resolution.x / 8, _resolution.y / 8, 1);
    //    glUniform1ui(glGetUniformLocation(_opengl_handles.compute, "current_frame"),
    //    _current_frame);
    // Bind path tracing compute shader
}

void cr::gpu_renderer::kernel_extend(std::uint32_t fired_rays)
{
    glUseProgram(_kernels.extend);

    // Update the amount of rays here
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, _buffers.ray_count);
    glBufferData(
      GL_SHADER_STORAGE_BUFFER,
      4,
      &fired_rays,
      GL_DYNAMIC_COPY);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffers.ray);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _buffers.intersections);
    glUniform1ui(0, fired_rays);

    glDispatchCompute((_resolution.x * _resolution.y) / 8, 1, 1);
}

void cr::gpu_renderer::kernel_shade(std::uint32_t intersections)
{
    glUseProgram(_kernels.shade);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _buffers.ray);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _buffers.intersections);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _buffers.ray_count);

    glBindImageTexture(0, _opengl_handles.accumulation, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    glUniform2i(0, _resolution.x, _resolution.y);
    glUniform1ui(1, intersections);

    glDispatchCompute((_resolution.x * _resolution.y) / 8, 1, 1);
}
