#include "SAPOneShot.hpp"
#include "Cast.hpp"
#include "ScriptableAnimationPlayer.hpp"

using namespace godot;

void godot::SAPOneShot::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("sample", "delta"), &SAPOneShot::Sample, DEFVAL(-1));
    ClassDB::bind_method(D_METHOD("is_active"), &SAPOneShot::IsActive);
    ClassDB::bind_method(D_METHOD("get_weight"), &SAPOneShot::GetWeight);
    ClassDB::bind_method(D_METHOD("get_time"), &SAPOneShot::GetTime);
    ClassDB::bind_method(D_METHOD("get_end_time"), &SAPOneShot::GetEndTime);

    ClassDB::bind_method(D_METHOD("add_method_key", "time", "path from animation player", "method name", "parameters"), &SAPOneShot::AddMethodKey,
                         DEFVAL(0.0), DEFVAL("../"), DEFVAL(""), DEFVAL(Array()));

    ClassDB::bind_method(D_METHOD("set_start_pause", "pause time"), &SAPOneShot::SetStartPause);
    ClassDB::bind_method(D_METHOD("set_end_pause", "pause time"), &SAPOneShot::SetEndPause);

    ClassDB::bind_method(D_METHOD("fire"), &SAPOneShot::Fire);
    ClassDB::bind_method(D_METHOD("abort"), &SAPOneShot::Abort);
    ClassDB::bind_method(D_METHOD("fade_out_early"), &SAPOneShot::FadeOutEarly);
}

void godot::SAPOneShot::AddMethodKey(double time, NodePath path, StringName method, Array parameters)
{
    SAPAnimationInt::MethodKey key;
    key.time = time;
    key.path = path;
    key.method = method;
    key.parameters = parameters;
    parent->oneShosts[index].anim.methodKeys.push_back(key);
}

void godot::SAPOneShot::SetStartPause(double t)
{
    parent->oneShosts[index].startPause = t;
}

void godot::SAPOneShot::SetEndPause(double t)
{
    parent->oneShosts[index].endPause = t;
}

SAPPoseWeighted *godot::SAPOneShot::Sample(double delta)
{
    auto &shot = parent->oneShosts[index];
    auto intPose = shot.Sample(delta);

    auto pose = memnew(SAPPoseWeighted);
    pose->weight = shot.weight;
    pose->index = parent->AddPose(intPose);
    pose->parent = parent;
    return pose;
}

bool godot::SAPOneShot::IsActive()
{
    return parent->oneShosts[index].active;
}

double godot::SAPOneShot::GetWeight()
{
    return parent->oneShosts[index].weight;
}

double godot::SAPOneShot::GetTime()
{
    return parent->oneShosts[index].anim.time;
}

double godot::SAPOneShot::GetEndTime()
{
    return parent->oneShosts[index].CalcEndTime();
}

void godot::SAPOneShot::Fire()
{
    auto &shot = parent->oneShosts[index];
    shot.weight = 0;
    shot.fadeOutEarly = false;
    shot.Fire();
}

void godot::SAPOneShot::Abort()
{
    auto &shot = parent->oneShosts[index];
    shot.active = false;
}

void godot::SAPOneShot::FadeOutEarly()
{
    parent->oneShosts[index].fadeOutEarly = true;
}

SAPPoseInt godot::SAPOneShotInt::Sample(double delta)
{
    if (active)
    {
        // find range
        double start = CalcStartTime();
        double end = CalcEndTime();
        auto &time = anim.time;

        // calc target
        char targetWeight = (time >= start && time < end - fadeOutTime) ? 1 : 0;
        if (fadeOutEarly)
            targetWeight = 0;
        double mult = delta / ((targetWeight >= weight) ? fadeInTime : fadeOutTime);
        weight = Math::clamp(weight + Math::sign(targetWeight - weight) * mult, 0.0, 1.0);
        if (time > end)
            active = false;

        if (fadeOutEarly && Math::is_zero_approx(weight))
            active = false;
    }
    else
    {
        weight = 0;
        delta = 0;
    }
    return anim.Sample(delta);
}

double godot::SAPOneShotInt::CalcStartTime()
{
    return (fadeInInternal ? 0.0 : -fadeInTime) - startPause;
}

double godot::SAPOneShotInt::CalcEndTime()
{
    return anim.anim->get_length() + (fadeOutInternal ? 0.0 : fadeOutTime) + endPause;
}

void godot::SAPOneShotInt::Fire()
{
    if (active)
        return;

    anim.time = CalcStartTime();
    active = true;
}
