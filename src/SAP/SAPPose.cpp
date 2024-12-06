#include "SAPPose.hpp"
#include "Cast.hpp"
#include "ScriptableAnimationPlayer.hpp"
using namespace godot;

void godot::SAPPose::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("get_index"), &SAPPose::GetIndex);
    ClassDB::bind_method(D_METHOD("blend", "other", "weight"), &SAPPose::Blend, DEFVAL(nullptr), DEFVAL(0.0));
}

SAPPose *godot::SAPPose::BlendPose(SAPPose *other, double weight)
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

SAPPose *godot::SAPPose::BlendWeighted(SAPPoseWeighted *other)
{
    if (other->index < 0)
        return this;

    auto inter = parent->poses[index].Blend(parent->poses[other->index], other->weight);
    auto res = memnew(SAPPose);
    res->parent = parent;
    res->index = parent->AddPose(inter);
    return res;
}

SAPPose *godot::SAPPose::Blend(RefCounted *in, double weight)
{
    if (in == nullptr)
        return this;

    auto classname = in->get_class();
    if (classname.nocasecmp_to("SAPPose") == 0)
        return BlendPose((SAPPose *)in, weight);
    if (classname.nocasecmp_to("SAPPoseWeighted") == 0)
        return BlendWeighted((SAPPoseWeighted *)in);

    return this;
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

    res.rootPosDelta = rootPosDelta.lerp(other.rootPosDelta, weight);
    return res;
}