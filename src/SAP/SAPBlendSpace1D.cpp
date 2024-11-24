#include "SAPBlendSpace1D.hpp"
#include "ScriptableAnimationPlayer.hpp"
#include "Cast.hpp"

#include <godot_cpp/classes/engine.hpp>

#include <algorithm>

using namespace godot;

void godot::SAPBlendSpace1D::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("get_index"), &SAPBlendSpace1D::GetIndex);
    ClassDB::bind_method(D_METHOD("add_point", "position", "animation name"), &SAPBlendSpace1D::AddPoint, DEFVAL(0), DEFVAL(""));
    ClassDB::bind_method(D_METHOD("sample", "position", "delta"), &SAPBlendSpace1D::Sample, DEFVAL(0), DEFVAL(-1));
}

void godot::SAPBlendSpace1D::AddPoint(double pos, String animName)
{
    auto &inter = parent->blendSpace1Ds[index];
    auto ani = parent->GetAnimation(animName);
    if (ani->index < 0)
        return;
    inter.AddPoint(pos, parent->animations[ani->index]);
}

SAPPose *godot::SAPBlendSpace1D::Sample(double pos, double delta)
{
    auto intPose = parent->blendSpace1Ds[index].Sample(pos, delta);
    auto pose = memnew(SAPPose);
    pose->index = parent->AddPose(intPose);
    pose->parent = parent;
    return pose;
}

void godot::SAPBlendSpace1DInt::AddPoint(double pos, SAPAnimationInt anim)
{
    points.push_back(Point(pos, anim));
}

void godot::SAPBlendSpace1DInt::Process(const double &delta)
{
    for (auto &p : points)
        p.anim.Process(delta);
}

SAPPoseInt godot::SAPBlendSpace1DInt::Sample(double pos, const double &delta)
{
    if (delta >= 0)
        Process(delta);

    switch (points.size())
    {
    case 0:
        PRINTERR("SAP Blend Space 1D: No Points Set!")
        return SAPPoseInt();
    case 1:
        return points[0].anim.Sample();
    }
    std::vector<std::pair<double, Point *>> closest;

    for (int i = 0; i < points.size(); ++i)
    {
        double dist = Math::abs(pos - points[i].pos);
        bool inserted = false;
        for (int c = 0; c < closest.size(); ++c)
        {
            if (dist < closest[c].first)
            {
                closest.insert(closest.begin() + c, std::make_pair(dist, &points[i]));
                inserted = true;
                break;
            }
        }
        if (!inserted)
            closest.push_back(std::make_pair(dist, &points[i]));
        if (closest.size() > 2)
            closest.pop_back();
    }

    Point *min, *max;
    if (closest[0].second->pos < closest[1].second->pos)
    {
        min = closest[0].second;
        max = closest[1].second;
    }
    else
    {
        min = closest[1].second;
        max = closest[0].second;
    }

    double p = Math::clamp((pos - min->pos) / (max->pos - min->pos), 0.0, 1.0);
    return min->anim.Sample().Blend(max->anim.Sample(), p);
}

SAPPoseInt godot::SAPBlendSpace1DInt::SampleRadial(double pos, const double &delta)
{
    if (delta >= 0)
        Process(delta);

    std::vector<PointDist> pointDists;
    // pointDists.resize(points.size() * 2);
    double off = pos >= 180 ? 360 : -360;
    for (int i = 0; i < points.size(); ++i)
    {
        auto &p = points[i];
        pointDists.push_back(PointDist(Math::abs(pos - p.pos), p.pos, &p));
        pointDists.push_back(PointDist(Math::abs(pos - (p.pos + off)), p.pos + off, &p));
    }

    std::sort(pointDists.begin(), pointDists.end(), PointDist::compare);
    bool test = pointDists[0].posOverride < pointDists[1].posOverride;
    auto &min = test ? pointDists[0] : pointDists[1];
    auto &max = test ? pointDists[1] : pointDists[0];

    double lerp = Math::clamp((pos - min.posOverride) / (max.posOverride - min.posOverride), 0.0, 1.0);
    return min.point->anim.Sample().Blend(max.point->anim.Sample(), lerp);
}
