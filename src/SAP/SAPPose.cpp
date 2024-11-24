#include "SAPPose.hpp"
#include "ScriptableAnimationPlayer.hpp"
using namespace godot;

void godot::SAPPose::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("get_index"), &SAPPose::GetIndex);
    ClassDB::bind_method(D_METHOD("blend", "other", "weight"), &SAPPose::Blend);
}

SAPPose *godot::SAPPose::Blend(SAPPose *other, double weight)
{
    if (index < 0)
        return other;
    if (other->index < 0)
        return this;

    auto inter = parent->poses[index].Blend(parent->poses[other->index], weight);
    auto res = memnew(SAPPose);
    res->parent = parent;
    res->index = parent->AddPose(inter);
    return res;
}

SAPPoseInt godot::SAPPoseInt::Duplicate()
{
    SAPPoseInt res;

    res.bones.resize(bones.size());
    for (int i = 0; i < bones.size(); ++i)
        res[i] = bones[i].Duplicate();

    return res;
}

SAPPoseInt godot::SAPPoseInt::Blend(const SAPPoseInt &other, double weight)
{
    weight = Math::clamp(weight, 0.0, 1.0);
    SAPPoseInt res;
    res.bones.resize(bones.size());
    for (int i = 0; i < res.bones.size(); ++i)
    {
        SAPBone &b = res.bones[i];
        b.pos = bones[i].pos.lerp(other.bones[i].pos, weight);
        b.rot = bones[i].rot.slerp(other.bones[i].rot, weight);
        b.scale = bones[i].scale.lerp(other.bones[i].scale, weight);
    }
    return res;
}