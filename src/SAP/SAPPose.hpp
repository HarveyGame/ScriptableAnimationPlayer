#pragma once

#include "SAPBone.hpp"

#include <godot_cpp/classes/node3d.hpp>

namespace godot
{
    class SAPPoseWeighted : public RefCounted
    {
    protected:
        GDCLASS(SAPPoseWeighted, RefCounted);
        static void _bind_methods()
        {
            ClassDB::bind_method(D_METHOD("get_weight"), &SAPPoseWeighted::GetWeight);
        };

    public:
        SAPPoseWeighted() {};
        ~SAPPoseWeighted() {};
        int GetIndex() { return index; }
        double GetWeight() { return weight; }

        int index = -1;
        double weight = 0;
        class ScriptableAnimationPlayer *parent;
    };

    class SAPPose : public RefCounted
    {
    protected:
        GDCLASS(SAPPose, RefCounted);
        static void _bind_methods();

    public:
        SAPPose() {};
        ~SAPPose() {};
        int GetIndex() { return index; }
        SAPPose *BlendPose(SAPPose *other, double weight);
        SAPPose *BlendWeighted(SAPPoseWeighted *other);
        SAPPose *Blend(RefCounted *in, double weight);

        int index = -1;
        class ScriptableAnimationPlayer *parent;
    };

    struct SAPPoseInt
    {
        std::vector<SAPBone> bones;

        SAPPoseInt Duplicate();
        SAPPoseInt Blend(const SAPPoseInt &other, double weight);

        Vector3 rootPosDelta;

        SAPBone &operator[](int index) { return bones[index]; }
    };
}