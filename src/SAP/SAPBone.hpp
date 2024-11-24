#pragma once

#include <godot_cpp/classes/node3d.hpp>

namespace godot
{
    struct SAPBone
    {
        SAPBone() {};
        ~SAPBone() {};
        String name;
        Vector3 pos;
        Quaternion rot;
        Vector3 scale = Vector3(1, 1, 1);

        SAPBone Duplicate();
    };

}