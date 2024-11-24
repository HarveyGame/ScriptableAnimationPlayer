#pragma once

#include "SAPAnimation.hpp"

namespace godot
{
    class SAPBlendSpace1D : public RefCounted
    {
    protected:
        GDCLASS(SAPBlendSpace1D, RefCounted);
        static void _bind_methods();

    public:
        SAPBlendSpace1D() {};
        ~SAPBlendSpace1D() {};
        int GetIndex() { return index; }
        void AddPoint(double pos, String animName);
        SAPPose *Sample(double pos, double delta = -1);

        int index = -1;
        class ScriptableAnimationPlayer *parent;
    };

    class SAPBlendSpace1DInt
    {
    public:
        struct Point
        {
            double pos;
            SAPAnimationInt anim;
            Point(double Pos, SAPAnimationInt Anim) : pos(Pos), anim(Anim) {}
        };

        struct PointDist
        {
            double dist;
            double posOverride;
            Point *point;
            PointDist(double Dist, double Pos, Point *Point) : dist(Dist), posOverride(Pos), point(Point) {};
            static bool compare(const PointDist &p1, const PointDist &p2)
            {
                return p1.dist < p2.dist;
            }
        };

        std::vector<Point> points;

        SAPBlendSpace1DInt() {};

        void AddPoint(double pos, SAPAnimationInt anim);
        // void AddPoint(double pos, Ref<Animation> anim);
        void Process(const double &delta);
        SAPPoseInt Sample(double pos, const double &delta = -1);

        /// @brief Samples for the 2D Radial Blend Space
        SAPPoseInt SampleRadial(double pos, const double &delta = -1);
    };
}