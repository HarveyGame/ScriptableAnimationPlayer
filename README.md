# Scriptable Animation Player for the Godot engine

## A new 3D animation player
Intended to become a drop-in replacement for the Animation Tree in [Godot](https://github.com/godotengine/godot), the Scriptable Animation Player (SAP) aims to give you more control over the animations of your 3D creations.

## State of the extension
This project is in an incredibly early state, expect bugs, breaking changes, etc. Think carefully before committing to using it.

Current Support:
+ Position, Rotation & Scale of bones
+ Method Call tracks

Currently in Development:
+ Typical Blend Space 2D (I'm not super keen on the current radial option)
+ Root Motion
+ Wiki

Planned:
+ One shot automation
+ Audio tracks

## Compatibility
+ The GDExtension is built on Godot 4.3
+ Currently only releasing pre-builts for windows, source code is available for other platforms
+ If you require an earlier version, the register_types.cpp file will need some tweaks & a rebuild of the extension

## Current classes/types and their use
+ ScriptableAnimationPlayer - The main class and only actual node, attach to a character, point the skeleton & animation player variables to your target model to get started!
+ SAPPose - The pose of your character this frame, can be blended with other poses to achieve your desired final pose
+ SAPAnimation - The animation you will sample from
+ SAPBlendSpace1D - a 1 dimensional blend space, add points and sample between them
+ SAPBlendSpace2DRadial - a 2D version of the 1D blend space, add points in 2D and pass between multiple rings of 1D Blend Spaces, useful for movement

## How to use in GDScript:
+ Drop the "ScriptableAnimationPlayer" folder into your project folder
+ open your project 
+ add the "ScriptableAnimationPlayer" under your animated character's root object, same as you would an animation tree
+ In the Inspector, set the "Player Path" and "Skeleton Path" to point to your models "AnimationPlayer" and "Skeleton3D" nodes
+ Add a script & edit it for your character, here's an example script
```
extends ScriptableAnimationPlayer

@onready var attack1 := get_animation("AttackSwing1")
@onready var blend_space2D := get_blend_space2D_radial()

func _ready() -> void:	

	#quickly print out the list of animations
	print(get_animation_list())

	#add the animations at their respective positions
	blend_space2D.add_point(Vector2(0,0), "IdleHappy")
	blend_space2D.add_point(Vector2(0,1), "RunForward")
	blend_space2D.add_point(Vector2(0,-1), "RunBack")
	blend_space2D.add_point(Vector2(-1,0), "StrafeLeft")
	blend_space2D.add_point(Vector2(1,0), "StrafeRight")


func _process(delta: float) -> void:
	var joystick := Vector2(Input.get_axis("left", "right"),Input.get_axis("back", "fwd"))
	
	#Samples blend space 2D, at the joystick, pass delta into Samples to make them move
	var pose := blend_space2D.sample(joystick, delta)
	#Sample the animation, passing in the delta to update
	var attack_pose := attack1.sample(delta)
	
	#blend these 2 animations together
	pose = pose.blend(attack_pose, Input.is_action_pressed("Attack"))
	#finally, apply the pose
	apply_pose(pose)
```

## How to use in GDExtension (C++)
+ Replace the empty "godot-cpp" folder with the stable release of [Godot's provided one](https://github.com/godotengine/godot-cpp), 
+ The best method is to drop the "SAP" source code folder (in the src folder) into your own GDExtension's src folder
+ Either add the GDREGISTERs & includes to your own "register_types.cpp", or copy that as well
+ Ensure your SConstruct file's sources list includes the directory, or use an SConstruct file like the one included that automatically finds subfolders
+ Create a new class that inherits from the ScriptableAnimationPlayer class, example below (though you'd probably want to make a header & source file)
+ Import into "register_types.cpp" and register your new class with GDREGISTER_RUNTIME_CLASS()
+ Compile, add your new node to your animated character's root object, link "Player Path" & "Skeleton Path"
```
#pragma once
#include "SAP/ScriptableAnimationPlayer.hpp"

namespace godot
{
    class MySAP : public ScriptableAnimationPlayer
    {
    protected:
        GDCLASS(MySAP, ScriptableAnimationPlayer)
        static void _bind_methods() {};

    public:
        MySAP() {};
        ~MySAP() {};

        void _ready() override
        {
            runFwd = GetAnimation("RunForward");
            idle = GetAnimation("IdleHappy");
        }

        void _process(double delta) override
        {
            auto pose = idle->Sample(delta);
            pose = pose->Blend(runFwd->Sample(delta), .5);
            ApplyPose(pose);
        }

        SAPAnimation *runFwd, *idle;
    };
}
```
while you do have access to the internal versions of these classes (such as SAPPoseInt for SAPPose), it is recommended to not use those.