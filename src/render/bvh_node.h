#pragma once

#include <memory>

#include <math/bbox.h>

namespace cr
{
    void inline assert_bounds(const RTCBounds &bounds)
    {
        assert(
          (bounds.lower_x == bounds.upper_x && bounds.lower_x == 0) &&
          (bounds.lower_y == bounds.upper_y && bounds.lower_y == 0) &&
          (bounds.lower_z == bounds.upper_z && bounds.lower_z == 0));
    }

    [[nodiscard]] inline RTCBounds bounds_from_prim(const RTCBuildPrimitive &build_primitive)
    {
        auto bounds    = RTCBounds();
        bounds.lower_x = build_primitive.lower_x;
        bounds.lower_y = build_primitive.lower_y;
        bounds.lower_z = build_primitive.lower_z;
        bounds.upper_x = build_primitive.upper_x;
        bounds.upper_y = build_primitive.upper_y;
        bounds.upper_z = build_primitive.upper_z;
//        assert_bounds(bounds);
        return bounds;
    }

    void inline split_primitive(
      const RTCBuildPrimitive *prim,
      unsigned int             dim,
      float                    pos,
      RTCBounds *              lprim,
      RTCBounds *              rprim,
      void *                   userPtr)
    {
        assert(dim < 3);
        *lprim                 = bounds_from_prim(*prim);
        *rprim                 = bounds_from_prim(*prim);

        switch (dim)
        {
        case 0:
            lprim->upper_x = rprim->lower_x = pos;
            break;
        case 1:
            lprim->upper_y = rprim->lower_y = pos;
            break;
        case 2:
            lprim->upper_z = rprim->lower_z = pos;
            break;
        }

//        assert_bounds(*lprim);
//        assert_bounds(*rprim);
    }

    namespace itl
    {
        struct Node
        {
            virtual float sah() = 0;
        };

        struct InnerNode : public Node
        {
            cr::bbox bounds[2];
            Node* children[2];

            InnerNode() {
                bounds[0] = bounds[1] = cr::bbox();
                children[0] = children[1] = nullptr;
            }

            float sah() {
                return 1.0f + (bounds[0].area()*children[0]->sah() + bounds[1].area()*children[1]->sah())/bounds[0].merge(bounds[1]).area();
            }

            static void* create (RTCThreadLocalAllocator alloc, unsigned int numChildren, void* userPtr)
            {
                assert(numChildren == 2);
                void* ptr = rtcThreadLocalAlloc(alloc,sizeof(InnerNode),16);
                return (void*) new (ptr) InnerNode;
            }

            static void  setChildren (void* nodePtr, void** childPtr, unsigned int numChildren, void* userPtr)
            {
                assert(numChildren == 2);
                for (size_t i=0; i<2; i++)
                    ((InnerNode*)nodePtr)->children[i] = (Node*) childPtr[i];
            }

            static void  setBounds (void* nodePtr, const RTCBounds** bounds, unsigned int numChildren, void* userPtr)
            {
                assert(numChildren == 2);
                for (size_t i=0; i<2; i++)
                    ((InnerNode*)nodePtr)->bounds[i] = cr::bbox(*bounds[i]);
            }
        };

        struct LeafNode : public Node
        {
            unsigned id;
            cr::bbox bounds;

            LeafNode (unsigned id, const cr::bbox& bounds)
              : id(id), bounds(bounds) {}

            float sah() {
                return 1.0f;
            }

            static void* create (RTCThreadLocalAllocator alloc, const RTCBuildPrimitive* prims, size_t numPrims, void* userPtr)
            {
                assert(numPrims == 1);
                void* ptr = rtcThreadLocalAlloc(alloc,sizeof(LeafNode),16);
                return (void*) new (ptr) LeafNode(prims->primID,cr::bbox(*prims));
            }
        };
    }

    struct embree_node
    {
        bool         is_leaf = false;
        std::vector<std::uint32_t> ids;
        embree_node *children[2];
        cr::bbox     bounds[2];

        embree_node() = default;

        embree_node(const RTCBuildPrimitive *prims, size_t count, const cr::bbox &bounds) : is_leaf(true), bounds()
        {
            ids.resize(count);
            for (auto i = 0; i < count; i++)
                ids[i] = prims[i].primID;

            this->bounds[0] = bounds;
            this->bounds[1] = cr::bbox();
        }

        [[nodiscard]] float sah()
        {
            if (is_leaf) return 1.0f;

            const auto left  = bounds[0];
            const auto right = bounds[1];

            return 1.0f +
              (left.area() * children[0]->sah() + right.area() * children[1]->sah()) /
              left.merge(right).area();
        }

        static void *
          create_node(RTCThreadLocalAllocator allocator, unsigned int num_children, void *user_ptr)
        {
            assert(num_children == 2);
            *(size_t *) user_ptr += 2;
            auto ptr = rtcThreadLocalAlloc(allocator, sizeof(embree_node), 16);
            return (void *) new (ptr) embree_node;
        }

        static void set_bounds(
          void *            node_ptr,
          const RTCBounds **bounds,
          unsigned int      num_children,
          void *            user_ptr)
        {
            assert(num_children == 2);
            const auto node = reinterpret_cast<embree_node*>(node_ptr);
            node->bounds[0] = cr::bbox(*bounds[0]);
            node->bounds[1] = cr::bbox(*bounds[1]);
        }

        static void
          set_children(void *node_ptr, void **child_ptr, unsigned int num_children, void *user_ptr)
        {
            assert(num_children == 2);
            for (size_t i = 0; i < 2; i++)
                reinterpret_cast<embree_node *>(node_ptr)->children[i] =
                  reinterpret_cast<embree_node *>(child_ptr[i]);
        }

        static void *create_leaf(
          RTCThreadLocalAllocator  allocator,
          const RTCBuildPrimitive *primitives,
          size_t                   num_prims,
          void *                   user_ptr)
        {
            (*(size_t *) user_ptr)++;
            auto ptr = rtcThreadLocalAlloc(allocator, sizeof(embree_node), 16);
            return (void *) new (ptr) embree_node(primitives, num_prims, cr::bbox(*primitives));
        }
    };

    class bvh_node
    {
    public:
        /*
         * Data is represented as so
         * [0..2] - Minimum XYZ
         * [3] - Primitive count - 0 If not leaf
         * [4..6] - Maximum XYZ
         * [7] - Next node (or primitive) - As int
         */
        glm::vec4 front;
        glm::vec4 back;

        void set_primitive_count(int count) { std::memcpy(&front.w, &count, sizeof(int)); }

        void set_child_node_start(int index) { std::memcpy(&back.w, &index, sizeof(int)); }

        void set_primitive_first_id(int index) { std::memcpy(&back.w, &index, sizeof(int)); }

        bvh_node(const cr::bbox &bounds)
        {
            front[0] = bounds.min.x;
            front[1] = bounds.min.y;
            front[2] = bounds.min.z;

            back[0] = bounds.max.x;
            back[1] = bounds.max.y;
            back[2] = bounds.max.z;

            set_primitive_count(0);
        }
    };
}    // namespace cr
