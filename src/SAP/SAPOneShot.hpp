#pragma once

#include "SAPAnimation.hpp"

namespace godot
{
    class SAPOneShot : public RefCounted
    {
    protected:
        GDCLASS(SAPOneShot, RefCounted);
        static void _bind_methods();

    public:
        SAPOneShot() {};
        ~SAPOneShot() {};
        int GetIndex() { return index; }
        // void SetTime(double newTime, bool clearUpdateFrame = false);
        // void SetLoopMode(Animation::LoopMode newLoopMode);
        void AddMethodKey(double time, NodePath path, StringName method, Array parameters);
        void SetStartPause(double t);
        void SetEndPause(double t);

        bool IsActive();
        double GetWeight();
        double GetTime();
        double GetEndTime();

        SAPPoseWeighted *Sample(double delta = -1);
        void Fire();
        void Abort();
        void FadeOutEarly();

        int index = -1;
        class ScriptableAnimationPlayer *parent;
    };

    class SAPOneShotInt
    {
    public:
        SAPOneShotInt() {};
        SAPPoseInt Sample(double delta = -1);
        double CalcStartTime();
        double CalcEndTime();

        void Fire();

        SAPAnimationInt anim;

        double fadeInTime = 0.1;
        bool fadeInInternal = true;
        double startPause = 0.0;

        double fadeOutTime = 0.1;
        bool fadeOutInternal = true;
        double endPause = 0.0;

        bool active = false;
        bool fadeOutEarly = false;
        double weight = 0;
    };
}