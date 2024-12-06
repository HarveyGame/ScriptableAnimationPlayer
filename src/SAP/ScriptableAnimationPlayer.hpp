#pragma once

#include "SAPAnimation.hpp"
#include "SAPBlendSpace1D.hpp"
#include "SAPBlendSpace2DRadial.hpp"
#include "SAPBlendSpace2D.hpp"
#include "SAPOneShot.hpp"

// #include "Cast.hpp"

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/animation.hpp>
#include <godot_cpp/classes/slider.hpp>
#include <godot_cpp/classes/curve.hpp>
#include <godot_cpp/classes/curve2d.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/hash_set.hpp>

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
        enum TranslationType : unsigned char
        {
            TT_Position,
            TT_Rotation,
            TT_Scale,
        };

        ScriptableAnimationPlayer() {};
        ~ScriptableAnimationPlayer() {};

        void Init();
        bool initialised = false;

        void _ready() override;
        void _process(double delta) override;

        void _input(Ref<InputEvent> event);

        PackedStringArray GetAnimationList();

        SAPAnimation *GetAnimation(String animName);
        std::vector<SAPAnimationInt> animations;

        SAPOneShot *GetOneShot(String animName, double fadeInTime = .1, double fadeOutTime = .1, bool fadeInInternal = true, bool fadeOutInternal = true);
        std::vector<SAPOneShotInt> oneShosts;

        SAPBlendSpace1D *GetBlendSpace1D();
        std::vector<SAPBlendSpace1DInt> blendSpace1Ds;

        SAPBlendSpace2DRadial *GetBlendSpace2DRadial();
        std::vector<SAPBlendSpace2DRadialInt> blendSpace2DRs;

        SAPBlendSpace2D *GetBlendSpace2D();
        std::vector<SAPBlendSpace2DInt> blendSpace2Ds;

        int AddPose(SAPPoseInt pose);
        int nextPoseIndex = 0;
        std::vector<SAPPoseInt> poses;

        Vector3 lastRMBPos = Vector3();
        void ApplyInternalPose(const SAPPoseInt &pose);
        void ApplyPose(SAPPose *pose);

        SAPPoseInt GetDefaultPose();
        SAPPoseInt defaultPose;

        MAKE_PROPGETSET_DEF(NodePath, playerPath)
        MAKE_PROPGETSET_DEF(NodePath, skeletonPath)
        MAKE_PROPGETSET_DEF(String, rootMotionBoneName)
        int rootMotionBone = -1;
        MAKE_PROPGETSET(int, rootMotionBone)

        class Skeleton3D *skele;
        class AnimationPlayer *player;
        String bonePath = "";
        int bonePathLength = 0;
        HashMap<String, int> boneIdTable;

        SAPBlendSpace2DInt bs2dTest;

        SAPOneShot *os;
    };
}