#pragma once

#include <godot_cpp/classes/curve.hpp>
#include "SAPBlendSpace1D.hpp"

namespace godot
{
    class SAPBlendSpace2DRadial : public RefCounted
    {
    protected:
        GDCLASS(SAPBlendSpace2DRadial, RefCounted);
        static void _bind_methods();

    public:
        SAPBlendSpace2DRadial() {};
        ~SAPBlendSpace2DRadial() {};
        int GetIndex() { return index; }
        void AddPoint(Vector2 pos, String animName);
        SAPPose *Sample(Vector2 pos, double delta = -1);

        int index = -1;
        class ScriptableAnimationPlayer *parent;
    };

    class SAPBlendSpace2DRadialInt
    {
    public:
        struct Ring
        {
            double distance;
            SAPBlendSpace1DInt space;
            Ring(double Distance, SAPBlendSpace1DInt Space) : distance(Distance), space(Space) {};

            static bool compare(const Ring &r1, const Ring &r2)
            {
                return r1.distance < r2.distance;
            }
        };
        std::vector<Ring> rings;

        SAPBlendSpace2DRadialInt() {};
        static SAPBlendSpace2DRadialInt Make(SAPAnimationInt center, double distanceMultiplier = 1.0 / .7);

        void AddPoint(Vector2 pos, SAPAnimationInt anim);

        void Process(const double &delta);

        SAPPoseInt Sample(Vector2 pos, const double &delta = -1);

        double distanceMult = 1;
        double maxLength = 0;
    };
}