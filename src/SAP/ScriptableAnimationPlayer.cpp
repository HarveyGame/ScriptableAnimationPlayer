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
#include <godot_cpp/classes/rich_text_label.hpp>

using namespace godot;

void godot::ScriptableAnimationPlayer::_bind_methods()
{
    MAKE_PROPBINDING(ScriptableAnimationPlayer, playerPath, Variant::NODE_PATH);
    MAKE_PROPBINDING(ScriptableAnimationPlayer, skeletonPath, Variant::NODE_PATH);

    ADD_GROUP("Root Motion", "");
    MAKE_PROPBINDING(ScriptableAnimationPlayer, rootMotionBoneName, Variant::STRING);
    MAKE_PROPBINDING(ScriptableAnimationPlayer, rootMotionBone, Variant::INT);

    ClassDB::bind_method(D_METHOD("get_animation_list"), &ScriptableAnimationPlayer::GetAnimationList);

    ClassDB::bind_method(D_METHOD("get_animation", "animation name"), &ScriptableAnimationPlayer::GetAnimation);
    ClassDB::bind_method(D_METHOD("get_one_shot", "animation name", "fade in time", "fade out time", "fade in internal", "fade out internal"), &ScriptableAnimationPlayer::GetOneShot,
                         DEFVAL(""), DEFVAL(.1), DEFVAL(.1), DEFVAL(true), DEFVAL(true));
    ClassDB::bind_method(D_METHOD("get_blend_space1D"), &ScriptableAnimationPlayer::GetBlendSpace1D);
    ClassDB::bind_method(D_METHOD("get_blend_space2D_radial"), &ScriptableAnimationPlayer::GetBlendSpace2DRadial);
    ClassDB::bind_method(D_METHOD("get_blend_space2D"), &ScriptableAnimationPlayer::GetBlendSpace2D);
    ClassDB::bind_method(D_METHOD("apply_pose", "pose"), &ScriptableAnimationPlayer::ApplyPose);
}

void godot::ScriptableAnimationPlayer::Init()
{
    if (initialised)
        return;
    initialised = true;

    player = get_node<AnimationPlayer>(playerPath);
    if (player == nullptr)
    {
        PRINTERR("Player not found, check path in ScriptableAnimationPlayer under ", get_parent()->get_name(), " and try again");
        return;
    }
    player->set_active(false);

    skele = get_node<Skeleton3D>(skeletonPath);
    if (skele == nullptr)
    {
        PRINTERR("Skeleton not found, check path in ScriptableAnimationPlayer under ", get_parent()->get_name(), " and try again");
        return;
    }
    skele->reset_bone_poses();

    bonePath = skele->get_parent()->get_name() + String("/") + skele->get_name() + ":";
    bonePathLength = bonePath.length();

    if (rootMotionBoneName.length() > 0)
    {
        for (int b = 0; b < skele->get_bone_count(); b++)
        {
            if (skele->get_bone_name(b).nocasecmp_to(rootMotionBoneName) != 0)
                continue;
            rootMotionBone = b;
            PRINT("Root Motion bone set to ", rootMotionBone, " / ", rootMotionBoneName)
            break;
        }
    }

    for (int b = 0; b < skele->get_bone_count(); b++)
    {
        SAPBone bone;
        bone.name = skele->get_bone_name(b);
        bone.pos = skele->get_bone_pose_position(b);
        bone.rot = skele->get_bone_pose_rotation(b);
        bone.scale = skele->get_bone_pose_scale(b);
        defaultPose.bones.push_back(bone);
        boneIdTable[bonePath + bone.name] = skele->find_bone(bone.name) + 1; // add 1 for error checking (0 = error)
        // PRINT("ADDED ", bonePath + bone.name, " = ", skele->find_bone(bone.name))
    }
}

void godot::ScriptableAnimationPlayer::_ready()
{
    Init();
}

void godot::ScriptableAnimationPlayer::_process(double delta)
{
}

void godot::ScriptableAnimationPlayer::_input(Ref<InputEvent> event)
{
    // if (event->is_class("InputEventKey"))
    // {
    //     Ref<InputEventKey> ev_key = event;
    //     switch (ev_key->get_keycode())
    //     {
    //     case Key::KEY_F2:
    //         queue_free();
    //         break;
    //     }
    // }
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

    if (rootMotionBone >= 0)
        skele->set_position(skele->get_position() + pose.rootPosDelta);
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

SAPOneShot *godot::ScriptableAnimationPlayer::GetOneShot(String animName, double fadeInTime, double fadeOutTime, bool fadeInInternal, bool fadeOutInternal)
{
    Init();

    if (!player->has_animation(animName))
    {
        PRINTERR("Player doesn't have animation '", animName, "' for oneshot")
        SAPOneShot *res = memnew(SAPOneShot);
        res->parent = this;
        return res;
    }

    oneShosts.push_back(SAPOneShotInt());
    SAPOneShotInt *os = &oneShosts[oneShosts.size() - 1];
    os->anim = SAPAnimationInt::Make(animName, this);
    os->anim.time = -1;
    os->anim.loopmode = Animation::LOOP_NONE;
    os->fadeInTime = fadeInTime;
    os->fadeInInternal = fadeInInternal;
    os->fadeOutTime = fadeOutTime;
    os->fadeOutInternal = fadeOutInternal;

    SAPOneShot *res = memnew(SAPOneShot);
    res->index = oneShosts.size() - 1;
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

SAPBlendSpace2D *godot::ScriptableAnimationPlayer::GetBlendSpace2D()
{
    Init();

    blendSpace2Ds.push_back(SAPBlendSpace2DInt());
    SAPBlendSpace2D *res = memnew(SAPBlendSpace2D);
    res->index = blendSpace2Ds.size() - 1;
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