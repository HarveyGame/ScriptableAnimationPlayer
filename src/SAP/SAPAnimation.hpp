#pragma once

// #include "Cast.hpp"
#include "SAPPose.hpp"

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/animation.hpp>

namespace godot
{
    class SAPAnimation : public RefCounted
    {
    protected:
        GDCLASS(SAPAnimation, RefCounted);
        static void _bind_methods();

    public:
        SAPAnimation() {};
        ~SAPAnimation() {};
        int GetIndex() { return index; }
        void SetTime(double newTime, bool clearUpdateFrame = false);
        void SetLoopMode(Animation::LoopMode newLoopMode);
        void AddMethodKey(double time, NodePath path, StringName method, Array parameters);
        SAPPose *Sample(double delta = -1);

        int index = -1;
        class ScriptableAnimationPlayer *parent;
    };

    class SAPAnimationInt
    {
    public:
        struct MethodKey
        {
            NodePath path;
            StringName method;
            Array parameters;
            double time;
        };

        SAPAnimationInt() {};
        static SAPAnimationInt Make(Ref<Animation> animation, class ScriptableAnimationPlayer *parent);
        static SAPAnimationInt Make(String animName, class ScriptableAnimationPlayer *parent);
        ~SAPAnimationInt() {};

        void Process(const double &delta);
        void SetTime(double newTime, bool clearUpdateFrame = false);

        double GetLoopedTime(const double &t);

        SAPPoseInt Sample(const double &delta = -1);

        Ref<Animation> anim;
        double time = 0;
        unsigned char lastUpdateFrame = 128;
        Animation::LoopMode loopmode;
        class ScriptableAnimationPlayer *parent;

        std::vector<MethodKey> methodKeys;
    };

}