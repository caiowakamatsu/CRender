#pragma once

#include <memory>

#include <render/bvh_node.h>

namespace cr
{
    class renderer
    {
    public:
        renderer() = default;

    private:
        std::unique_ptr<cr::bvh_node> parent_node;
    };
}