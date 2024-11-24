#pragma once

#include "SAPAnimation.hpp"
#include "SAPBlendSpace1D.hpp"
#include "SAPBlendSpace2DRadial.hpp"

// #include "Cast.hpp"

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/animation.hpp>
#include <godot_cpp/classes/slider.hpp>
#include <godot_cpp/classes/curve.hpp>
#include <godot_cpp/classes/curve2d.hpp>

#include <map>
#include <unordered_set>

#define MAKE_PROPGETSET(type, name)          \
    type get_##name() const { return name; } \
    void set_##name(type value) { name = value; }

#define MAKE_PROPGETSET_DEF(type, name) \
    type name;                          \
    MAKE_PROPGETSET(type, name);

namespace godot
{
    class ScriptableAnimationPlayer : public Node3D
    {
    protected:
        GDCLASS(ScriptableAnimationPlayer, Node3D)
        static void _bind_methods();

    public:
        ScriptableAnimationPlayer() {};
        ~ScriptableAnimationPlayer() {};

        void Init();
        bool initialised = false;

        PackedStringArray GetAnimationList();

        SAPAnimation *GetAnimation(String animName);
        std::vector<SAPAnimationInt> animations;

        SAPBlendSpace1D *GetBlendSpace1D();
        std::vector<SAPBlendSpace1DInt> blendSpace1Ds;

        SAPBlendSpace2DRadial *GetBlendSpace2DRadial();
        std::vector<SAPBlendSpace2DRadialInt> blendSpace2DRs;

        int AddPose(SAPPoseInt pose);
        int nextPoseIndex = 0;
        std::vector<SAPPoseInt> poses;

        void ApplyInternalPose(const SAPPoseInt &pose);
        void ApplyPose(SAPPose *pose);

        SAPPoseInt GetDefaultPose();
        SAPPoseInt defaultPose;

        MAKE_PROPGETSET_DEF(NodePath, playerPath)
        MAKE_PROPGETSET_DEF(NodePath, skeletonPath)

        class Skeleton3D *skele;
        class AnimationPlayer *player;
    };
}