#include "ScriptableAnimationPlayer.hpp"

#include "Cast.hpp"

#include <godot_cpp/classes/animation_player.hpp>
#include <godot_cpp/classes/animation_tree.hpp>
#include <godot_cpp/classes/animation_library.hpp>
#include <godot_cpp/classes/animation.hpp>
#include <godot_cpp/classes/skeleton3d.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/core/math.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/classes/geometry2d.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/engine.hpp>

using namespace godot;

void godot::ScriptableAnimationPlayer::_bind_methods()
{
    MAKE_PROPBINDING(ScriptableAnimationPlayer, playerPath, Variant::NODE_PATH);
    MAKE_PROPBINDING(ScriptableAnimationPlayer, skeletonPath, Variant::NODE_PATH);

    ClassDB::bind_method(D_METHOD("get_animation_list"), &ScriptableAnimationPlayer::GetAnimationList);

    ClassDB::bind_method(D_METHOD("get_animation", "animation name"), &ScriptableAnimationPlayer::GetAnimation);
    ClassDB::bind_method(D_METHOD("get_blend_space1D"), &ScriptableAnimationPlayer::GetBlendSpace1D);
    ClassDB::bind_method(D_METHOD("get_blend_space2D_radial"), &ScriptableAnimationPlayer::GetBlendSpace2DRadial);
    ClassDB::bind_method(D_METHOD("apply_pose", "pose"), &ScriptableAnimationPlayer::ApplyPose);
}

void godot::ScriptableAnimationPlayer::Init()
{
    if (initialised)
        return;
    initialised = true;

    player = get_node<AnimationPlayer>(playerPath);
    player->set_active(false);

    skele = get_node<Skeleton3D>(skeletonPath);
    skele->reset_bone_poses();

    for (int b = 0; b < skele->get_bone_count(); b++)
    {
        SAPBone bone;
        bone.name = skele->get_bone_name(b);
        bone.pos = skele->get_bone_pose_position(b);
        bone.rot = skele->get_bone_pose_rotation(b);
        bone.scale = skele->get_bone_pose_scale(b);
        defaultPose.bones.push_back(bone);
    }
}

SAPPoseInt godot::ScriptableAnimationPlayer::GetDefaultPose()
{
    return defaultPose.Duplicate();
}

void godot::ScriptableAnimationPlayer::ApplyInternalPose(const SAPPoseInt &pose)
{
    for (int i = 0; i < pose.bones.size(); ++i)
    {
        auto &b = pose.bones[i];
        skele->set_bone_pose_position(i, b.pos);
        skele->set_bone_pose_rotation(i, b.rot);
        skele->set_bone_pose_scale(i, b.scale);
    }
}

void godot::ScriptableAnimationPlayer::ApplyPose(SAPPose *pose)
{
    Init();
    if (pose->index < 0)
    {
        PRINTERR("Pose not valid");
        return;
    }

    ApplyInternalPose(poses[pose->index]);

    // cleanup for next frame
    nextPoseIndex = 0;
}

PackedStringArray godot::ScriptableAnimationPlayer::GetAnimationList()
{
    Init();

    return player->get_animation_list();
}

SAPAnimation *godot::ScriptableAnimationPlayer::GetAnimation(String animName)
{
    Init();

    if (!player->has_animation(animName))
    {
        PRINTERR("Player doesn't have animation '", animName, "'")
        // return anim with -1 index
        SAPAnimation *res = memnew(SAPAnimation);
        res->parent = this;
        return res;
    }
    animations.push_back(SAPAnimationInt::Make(animName, this));
    SAPAnimation *res = memnew(SAPAnimation);
    res->index = animations.size() - 1;
    res->parent = this;
    return res;
}

SAPBlendSpace1D *godot::ScriptableAnimationPlayer::GetBlendSpace1D()
{
    Init();

    blendSpace1Ds.push_back(SAPBlendSpace1DInt());
    SAPBlendSpace1D *res = memnew(SAPBlendSpace1D);
    res->index = blendSpace1Ds.size() - 1;
    res->parent = this;
    return res;
}

SAPBlendSpace2DRadial *godot::ScriptableAnimationPlayer::GetBlendSpace2DRadial()
{
    Init();

    blendSpace2DRs.push_back(SAPBlendSpace2DRadialInt());
    SAPBlendSpace2DRadial *res = memnew(SAPBlendSpace2DRadial);
    res->index = blendSpace2DRs.size() - 1;
    res->parent = this;
    return res;
}

int godot::ScriptableAnimationPlayer::AddPose(SAPPoseInt pose)
{
    if (nextPoseIndex < poses.size())
    {
        poses[nextPoseIndex] = pose;
        nextPoseIndex++;
        return nextPoseIndex - 1;
    }
    poses.push_back(pose);
    return poses.size() - 1;
}