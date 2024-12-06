#pragma once

#include "SAPAnimation.hpp"
#include <godot_cpp/classes/node3d.hpp>

namespace godot
{
    class SAPBlendSpace2D : public RefCounted
    {
    protected:
        GDCLASS(SAPBlendSpace2D, RefCounted);
        static void _bind_methods();

    public:
        SAPBlendSpace2D() {};
        ~SAPBlendSpace2D() {};
        int GetIndex() { return index; }
        void AddPoint(Vector2 pos, String animName);
        SAPPose *Sample(Vector2 pos, double delta = -1);

        int index = -1;
        class ScriptableAnimationPlayer *parent;
    };

    class SAPBlendSpace2DInt
    {

    public:
        struct Point
        {
            Vector2 pos;
            SAPAnimationInt anim;
            Point(Vector2 Pos, SAPAnimationInt Anim) : pos(Pos), anim(Anim) {};
        };

        struct Segment
        {
            int tri = -1;
            // std::vector<Vector2> seg1, seg2;
            Vector2 closestPoint;
            double distance;
        };

        std::vector<Point> points;
        PackedInt32Array tris;

        SAPBlendSpace2DInt() {};

        void AddPoint(Vector2 pos, SAPAnimationInt anim);
        void Bake();

        void Process(const double &delta);

        SAPPoseInt SampleTri(Vector2 &pos, Point &p0, Point &p1, Point &p2);
        SAPPoseInt Sample(Vector2 pos, const double &delta = -1);

        bool isDirty;
    };
}