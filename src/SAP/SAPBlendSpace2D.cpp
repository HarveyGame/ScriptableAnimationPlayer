#include "SAPBlendSpace2D.hpp"
#include "Cast.hpp"
#include "ScriptableAnimationPlayer.hpp"

#include <godot_cpp/classes/geometry2d.hpp>

using namespace godot;

void godot::SAPBlendSpace2D::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("get_index"), &SAPBlendSpace2D::GetIndex);
    ClassDB::bind_method(D_METHOD("add_point", "position", "animation name"), &SAPBlendSpace2D::AddPoint, DEFVAL(Vector2()), DEFVAL(""));
    ClassDB::bind_method(D_METHOD("sample", "position", "delta"), &SAPBlendSpace2D::Sample, DEFVAL(Vector2()), DEFVAL(-1));
}

void godot::SAPBlendSpace2D::AddPoint(Vector2 pos, String animName)
{
    auto &inter = parent->blendSpace2Ds[index];
    auto ani = parent->GetAnimation(animName);
    if (ani->index < 0)
        return;
    inter.AddPoint(pos, parent->animations[ani->index]);
}

SAPPose *godot::SAPBlendSpace2D::Sample(Vector2 pos, double delta)
{
    auto intPose = parent->blendSpace2Ds[index].Sample(pos, delta);
    auto pose = memnew(SAPPose);
    pose->index = parent->AddPose(intPose);
    pose->parent = parent;
    return pose;
}

void godot::SAPBlendSpace2DInt::AddPoint(Vector2 pos, SAPAnimationInt anim)
{
    PRINT(pos);
    anim.isPartOfSpace = true;
    points.push_back(Point(pos, anim));
    isDirty = true;
}

void godot::SAPBlendSpace2DInt::Bake()
{
    tris.clear();
    PackedVector2Array tpoints;
    for (auto &p : points)
    {
        tpoints.push_back(p.pos);
    }
    // PRINT(tpoints)
    tris = Geometry2D::get_singleton()->triangulate_delaunay(tpoints);
    PRINT(tris)

    isDirty = false;
}

void godot::SAPBlendSpace2DInt::Process(const double &delta)
{
    for (auto &p : points)
        p.anim.Process(delta);
}

// Nicked & modified from Godot's animation_blend_space_2d
void blend_triangle(const Vector2 &p_pos, const Vector2 &p0, const Vector2 &p1, const Vector2 &p2, std::vector<float> &r_weights)
{
    if (p_pos.is_equal_approx(p0))
    {
        r_weights[0] = 1;
        r_weights[1] = 0;
        r_weights[2] = 0;
        return;
    }
    if (p_pos.is_equal_approx(p1))
    {
        r_weights[0] = 0;
        r_weights[1] = 1;
        r_weights[2] = 0;
        return;
    }
    if (p_pos.is_equal_approx(p2))
    {
        r_weights[0] = 0;
        r_weights[1] = 0;
        r_weights[2] = 1;
        return;
    }

    Vector2 v0 = p1 - p0;
    Vector2 v1 = p2 - p0;
    Vector2 v2 = p_pos - p0;

    float d00 = v0.dot(v0);
    float d01 = v0.dot(v1);
    float d11 = v1.dot(v1);
    float d20 = v2.dot(v0);
    float d21 = v2.dot(v1);
    float denom = (d00 * d11 - d01 * d01);
    if (denom == 0)
    {
        r_weights[0] = 1;
        r_weights[1] = 0;
        r_weights[2] = 0;
        return;
    }
    float v = (d11 * d20 - d01 * d21) / denom;
    float w = (d00 * d21 - d01 * d20) / denom;
    float u = 1.0f - v - w;

    r_weights[0] = u;
    r_weights[1] = v;
    r_weights[2] = w;
}

SAPPoseInt godot::SAPBlendSpace2DInt::SampleTri(Vector2 &pos, Point &p0, Point &p1, Point &p2)
{
    std::vector<float> weights;
    weights.resize(3);
    blend_triangle(pos, p0.pos, p1.pos, p2.pos, weights);

    if (weights[0] == 1)
        return p0.anim.Sample();
    if (weights[1] == 1)
        return p1.anim.Sample();
    if (weights[2] == 1)
        return p2.anim.Sample();

    auto s0 = p0.anim.Sample();
    auto s1 = p1.anim.Sample();
    auto s2 = p2.anim.Sample();
    // PRINT(p0.anim.anim->get_name(), ": ", weights[0], ", ", p1.anim.anim->get_name(), ": ", weights[1], ", ", p2.anim.anim->get_name(), ": ", weights[2])
    SAPPoseInt res = p0.anim.parent->GetDefaultPose();
    for (int b = 0; b < res.bones.size(); ++b)
    {
        auto w0 = 1 - (weights[0] / (weights[0] + weights[1]));
        res.bones[b].pos = s0.bones[b].pos.lerp(s1.bones[b].pos, w0).lerp(s2.bones[b].pos, weights[2]);
        res.bones[b].rot = s0.bones[b].rot.slerp(s1.bones[b].rot, w0).slerp(s2.bones[b].rot, weights[2]).normalized();
        res.bones[b].scale = s0.bones[b].scale.lerp(s1.bones[b].scale, w0).lerp(s2.bones[b].scale, weights[2]);
    }
    res.rootPosDelta = s0.rootPosDelta * weights[0] +
                       s1.rootPosDelta * weights[1] +
                       s2.rootPosDelta * weights[2];
    return res;
}

Vector2 GetClosestPointTri(const Vector2 &pos, const Vector2 &nearest, const Vector2 &p1, const Vector2 &p2)
{
    auto geo = Geometry2D::get_singleton();
    auto r1 = geo->get_closest_point_to_segment(pos, nearest, p1);
    auto r2 = geo->get_closest_point_to_segment(pos, nearest, p2);

    return (pos.distance_to(r1) < pos.distance_to(r2)) ? r1 : r2;
}

SAPPoseInt godot::SAPBlendSpace2DInt::Sample(Vector2 pos, const double &delta)
{
    if (isDirty)
        Bake();

    if (delta >= 0)
        Process(delta);

    if (points.size() == 0 || tris.size() == 0)
    {
        PRINTERR("Blend Space 2D: No points set");
        return SAPPoseInt();
    }

    // check if on top of a point, sample & return that if so.
    // save for line check if outside
    int closestIndex = 0;
    double closestDist = pos.distance_to(points[0].pos);
    for (int i = 1; i < points.size(); ++i)
    {
        double d = pos.distance_to(points[i].pos);
        if (d > closestDist)
            continue;
        closestDist = d;
        closestIndex = i;
    }
    if (closestDist <= CMP_EPSILON)
    {
        return points[closestIndex].anim.Sample();
    }

    auto geo = Geometry2D::get_singleton();
    // find if in triangle
    for (int t = 0; t < tris.size(); t += 3)
    {
        auto &p0 = points[tris[t]];
        auto &p1 = points[tris[t + 1]];
        auto &p2 = points[tris[t + 2]];
        if (geo->point_is_inside_triangle(pos, p0.pos, p1.pos, p2.pos))
        {
            return SampleTri(pos, p0, p1, p2);
            // geo->get_closest_point_to_segment()
        }
    }

    Segment closestSegment;
    closestSegment.distance = 999999;
    // outside of triangle, find nearest with line check
    for (int t = 0; t < tris.size(); t += 3)
    {
        for (int i = 0; i < 3; ++i)
        {
            if (tris[t + i] == closestIndex)
            {
                int p1, p2;
                switch (i)
                {
                case 0:
                    p1 = t + 1;
                    p2 = t + 2;
                    break;
                case 1:
                    p1 = t;
                    p2 = t + 2;
                    break;
                case 2:
                    p1 = t + 1;
                    p2 = t;
                    break;
                }

                Segment s;
                s.tri = t;
                s.closestPoint = GetClosestPointTri(pos, points[tris[t + i]].pos, points[tris[p1]].pos, points[tris[p2]].pos);
                s.distance = pos.distance_to(s.closestPoint);
                if (s.distance < closestSegment.distance)
                    closestSegment = s;
                break;
            }
        }
    }

    if (closestSegment.tri == -1)
    {
        PRINTERR("FAILED TO FIND A SEGMENT")
        return SAPPoseInt();
    }

    // Sample & return the closest tri
    return SampleTri(closestSegment.closestPoint,
                     points[tris[closestSegment.tri]],
                     points[tris[closestSegment.tri + 1]],
                     points[tris[closestSegment.tri + 2]]);
}
