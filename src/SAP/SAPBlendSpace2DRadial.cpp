#include "SAPBlendSpace2DRadial.hpp"
#include "ScriptableAnimationPlayer.hpp"
#include "Cast.hpp"

#include <godot_cpp/classes/engine.hpp>

#include <algorithm>

using namespace godot;

void godot::SAPBlendSpace2DRadial::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("get_index"), &SAPBlendSpace2DRadial::GetIndex);
    ClassDB::bind_method(D_METHOD("add_point", "position", "animation name"), &SAPBlendSpace2DRadial::AddPoint, DEFVAL(Vector2()), DEFVAL(""));
    ClassDB::bind_method(D_METHOD("sample", "position", "delta"), &SAPBlendSpace2DRadial::Sample, DEFVAL(Vector2()), DEFVAL(-1));
}

void godot::SAPBlendSpace2DRadial::AddPoint(Vector2 pos, String animName)
{
    auto &inter = parent->blendSpace2DRs[index];
    auto ani = parent->GetAnimation(animName);
    if (ani->index < 0)
        return;
    inter.AddPoint(pos, parent->animations[ani->index]);
}

SAPPose *godot::SAPBlendSpace2DRadial::Sample(Vector2 pos, double delta)
{
    auto intPose = parent->blendSpace2DRs[index].Sample(pos, delta);
    auto pose = memnew(SAPPose);
    pose->index = parent->AddPose(intPose);
    pose->parent = parent;
    return pose;
}

SAPBlendSpace2DRadialInt godot::SAPBlendSpace2DRadialInt::Make(SAPAnimationInt center, double distanceMultiplier)
{
    SAPBlendSpace2DRadialInt res;
    res.distanceMult = distanceMultiplier;
    auto bs = SAPBlendSpace1DInt();
    bs.AddPoint(0, center);
    res.rings.push_back(Ring(0, bs));

    return res;
}

void godot::SAPBlendSpace2DRadialInt::AddPoint(Vector2 pos, SAPAnimationInt anim)
{
    double d = pos.length();
    double p = Math::fmod(Math::rad_to_deg(Vector2(0, 1).angle_to(pos)) + 360, 360);

    for (auto &r : rings)
    {
        if (Math::abs(d - r.distance) < .001)
        {
            anim.isPartOfSpace = true;
            r.space.AddPoint(p, anim);
            return;
        }
    }

    SAPBlendSpace1DInt bs;
    anim.isPartOfSpace = true;
    bs.AddPoint(p, anim);
    rings.push_back(Ring(d, bs));
    maxLength = Math::max(maxLength, d);
}

void godot::SAPBlendSpace2DRadialInt::Process(const double &delta)
{
    for (auto &r : rings)
        r.space.Process(delta);
}

SAPPoseInt godot::SAPBlendSpace2DRadialInt::Sample(Vector2 pos, const double &delta)
{
    if (delta >= 0)
        Process(delta);

    double d = Math::clamp(pos.length() * distanceMult, 0.0, maxLength);

    double p = Math::fmod(Math::rad_to_deg(Vector2(0, 1).angle_to(pos)) + 360, 360);
    switch (rings.size())
    {
    case 0:
        return SAPPoseInt();
    case 1:
        return rings[0].space.Sample(p, delta);
    }

    std::vector<std::pair<double, Ring *>> ringDist;
    for (auto &r : rings)
        ringDist.push_back(std::make_pair(Math::abs(r.distance - d), &r));

    std::sort(ringDist.begin(), ringDist.end(),
              [](const std::pair<double, Ring *> &r1, const std::pair<double, Ring *> &r2)
              { return r1.first < r2.first; });

    auto test = ringDist[0].first < ringDist[1].first;
    auto min = test ? ringDist[0].second : ringDist[1].second;
    auto max = test ? ringDist[1].second : ringDist[0].second;

    double lerp = Math::clamp((d - min->distance) / (max->distance - min->distance), 0.0, 1.0);
    return min->space.SampleRadial(p).Blend(max->space.SampleRadial(p), lerp);
}
