#pragma once

#include "SAPBone.hpp"

#include <godot_cpp/classes/node3d.hpp>

namespace godot
{
    class SAPPose : public RefCounted
    {
    protected:
        GDCLASS(SAPPose, RefCounted);
        static void _bind_methods();

    public:
        SAPPose() {};
        ~SAPPose() {};
        int GetIndex() { return index; }
        SAPPose *Blend(SAPPose *other, double weight);

        int index = -1;
        class ScriptableAnimationPlayer *parent;
    };

    struct SAPPoseInt
    {
        std::vector<SAPBone> bones;

        SAPPoseInt Duplicate();
        SAPPoseInt Blend(const SAPPoseInt &other, double weight);

        SAPBone &operator[](int index) { return bones[index]; }
    };
}