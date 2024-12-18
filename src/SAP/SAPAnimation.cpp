#include "SAPAnimation.hpp"
#include "ScriptableAnimationPlayer.hpp"

#include "Cast.hpp"

#include <godot_cpp/classes/skeleton3d.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/animation_player.hpp>
#include <godot_cpp/classes/scene_tree.hpp>

using namespace godot;

void godot::SAPAnimation::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("get_index"), &SAPAnimation::GetIndex);
    ClassDB::bind_method(D_METHOD("set_time", "new time", "clear update frame"), &SAPAnimation::SetTime, DEFVAL(0), DEFVAL(false));
    ClassDB::bind_method(D_METHOD("set_loop_mode"), &SAPAnimation::SetLoopMode);
    ClassDB::bind_method(D_METHOD("add_method_key", "time", "path from animation player", "method name", "parameters"), &SAPAnimation::AddMethodKey,
                         DEFVAL(0.0), DEFVAL("../"), DEFVAL(""), DEFVAL(Array()));

    ClassDB::bind_method(D_METHOD("sample", "delta"), &SAPAnimation::Sample, DEFVAL(-1));
}

void godot::SAPAnimation::SetTime(double newTime, bool clearUpdateFrame)
{
    parent->animations[index].SetTime(newTime, clearUpdateFrame);
}

void godot::SAPAnimation::SetLoopMode(Animation::LoopMode newLoopMode)
{
    parent->animations[index].loopmode = newLoopMode;
}

void godot::SAPAnimation::AddMethodKey(double time, NodePath path, StringName method, Array parameters)
{
    SAPAnimationInt::MethodKey key;
    key.time = time;
    key.path = path;
    key.method = method;
    key.parameters = parameters;
    parent->animations[index].methodKeys.push_back(key);
}

SAPPose *godot::SAPAnimation::Sample(double delta)
{
    auto inter = parent->animations[index].Sample(delta);
    auto pose = memnew(SAPPose);
    pose->index = parent->AddPose(inter);
    pose->parent = parent;
    return pose;
}

SAPAnimationInt godot::SAPAnimationInt::Make(Ref<Animation> animation, ScriptableAnimationPlayer *parent)
{
    SAPAnimationInt res;
    res.anim = animation;
    res.loopmode = res.anim->get_loop_mode();
    res.parent = parent;

    for (int track = 0; track < animation->get_track_count(); ++track)
    {
        switch (animation->track_get_type(track))
        {
        case Animation::TYPE_METHOD:
            auto path = animation->track_get_path(track);
            for (int i = 0; i < animation->track_get_key_count(track); ++i)
            {
                MethodKey key;
                key.time = animation->track_get_key_time(track, i);
                key.path = path;
                key.method = animation->method_track_get_name(track, i);
                key.parameters = animation->method_track_get_params(track, i);
                res.methodKeys.push_back(key);
            }
            break;

            // case Animation::TYPE_
        }
    }

    return res;
}

SAPAnimationInt godot::SAPAnimationInt::Make(String animName, ScriptableAnimationPlayer *parent)
{
    return Make(parent->player->get_animation(animName), parent);
}

void godot::SAPAnimationInt::Process(const double &delta)
{
    // protect from double processing
    unsigned char currframe = Engine::get_singleton()->get_frames_drawn();
    if (lastUpdateFrame == currframe)
    {
        // PRINT("Prevented duplicate process in SAPAnimation")
        return;
    }
    lastUpdateFrame = currframe;

    double lastTime = time;
    time += delta;

    if (methodKeys.size() == 0)
        return;

    // avoid clamping looptimes
    double looptime = loopmode == Animation::LOOP_NONE ? time : GetLoopedTime(time);
    double lastlooptime = loopmode == Animation::LOOP_NONE ? lastTime : GetLoopedTime(lastTime);

    for (const auto &mkey : methodKeys)
    {
        if ((int)Math::sign(lastlooptime - mkey.time) == (int)Math::sign(looptime - mkey.time))
            continue;

        Node *n = parent->player->get_parent()->get_node<Node>(mkey.path);
        if (n == nullptr)
        {
            PRINTERR("Animation failed to get node ", mkey.path, " for function call ", mkey.method)
            continue;
        }

        // I assume there's a macro for this array to parameter nonsense out there... somewhere...
        auto &par = mkey.parameters;
        switch (par.size())
        {
        case 0:
            n->call(mkey.method);
            break;
        case 1:
            n->call(mkey.method, par[0]);
            break;
        case 2:
            n->call(mkey.method, par[0], par[1]);
            break;
        case 3:
            n->call(mkey.method, par[0], par[1], par[2]);
            break;
        default:
            n->call(mkey.method, par[0], par[1], par[2], par[3]);
            break;
        }
    }
}

void godot::SAPAnimationInt::SetTime(double newTime, bool clearUpdateFrame)
{
    time = newTime;
    if (clearUpdateFrame)
        lastUpdateFrame += 100;
}

double godot::SAPAnimationInt::GetLoopedTime(const double &t)
{
    switch (loopmode)
    {
    case Animation::LOOP_LINEAR:
        return Math::fmod(t, anim->get_length());

    case Animation::LOOP_PINGPONG:
        return Math::pingpong(t, anim->get_length());

    case Animation::LOOP_NONE:
        return Math::clamp(t, 0.0, anim->get_length());

    default:
        return t;
    }
}

SAPPoseInt godot::SAPAnimationInt::Sample(const double &delta)
{

    if (delta >= 0)
        Process(delta);

    if (!anim.is_valid())
    {
        PRINTERR("Anim is not valid");
        return parent->GetDefaultPose();
    }

    // apply loop modes to time
    double ttime = GetLoopedTime(time);

    SAPPoseInt pose = parent->GetDefaultPose();
    if (pose.bones.size() == 0)
        return SAPPoseInt();

    int rootBoneId = parent->rootMotionBone;

    Vector3 npos;
    int boneID;
    for (int trackNo = 0; trackNo < anim->get_track_count(); ++trackNo)
    {
        auto trackPath = (String)anim->track_get_path(trackNo);

        // boneID = parent->skele->find_bone(trackPath.substr(bonePathLength, trackPath.length() - bonePathLength));
        boneID = parent->boneIdTable[trackPath] - 1; // subtract 1 for error catching (now boneID -1 = error!)

        if (boneID == -1) // if not a valid bone track
        {
            switch (anim->track_get_type(trackNo))
            {
            case Animation::TrackType::TYPE_POSITION_3D:
            case Animation::TrackType::TYPE_ROTATION_3D:
            case Animation::TrackType::TYPE_SCALE_3D:

                // PRINTERR("Anim: Failed to identify bone ", trackPath);
                break;
                // default:
                //     // is either handled at processing (like method) or unhandled atm
                //     break;
            }
            continue;
        }

        switch (anim->track_get_type(trackNo))
        {
        case Animation::TrackType::TYPE_POSITION_3D:
            if (boneID != rootBoneId)
            {
                pose.bones[boneID].pos = anim->position_track_interpolate(trackNo, ttime);
                break;
            }

            // root motion bone
            if (firstSample)
                lastRMBPos = anim->position_track_interpolate(trackNo, 0);

            npos = anim->position_track_interpolate(trackNo, ttime);
            if (lastTime > ttime)
            {
                rootPosDelta = anim->position_track_interpolate(trackNo, anim->get_length()) - anim->position_track_interpolate(trackNo, lastTime);
                rootPosDelta += npos - anim->position_track_interpolate(trackNo, 0);
            }
            else
                rootPosDelta = npos - lastRMBPos;

            pose.rootPosDelta = rootPosDelta;
            lastRMBPos = npos;
            break;

        case Animation::TrackType::TYPE_ROTATION_3D:
            pose.bones[boneID].rot = anim->rotation_track_interpolate(trackNo, ttime);
            break;

        case Animation::TrackType::TYPE_SCALE_3D:
            pose.bones[boneID].scale = anim->scale_track_interpolate(trackNo, ttime);
            break;
        }
    }

    firstSample = false;
    lastTime = GetLoopedTime(time);
    return pose;
}