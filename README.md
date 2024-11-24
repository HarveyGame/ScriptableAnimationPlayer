# Scriptable Animation Player for the Godot engine

## A new 3D animation player
Intended to become a drop-in replacement for the Animation Tree in [Godot](https://github.com/godotengine/godot), the Scriptable Animation Player (SAP) aims to give you more control over the animations of your 3D creations.

## Compatibility
+ The GDExtension is built on Godot 4.3
+ Currently only releasing pre-builts for windows, source code is available for other platforms
+ If you require an earlier version, the register_types.cpp file will need some tweeks & a rebuild of the extension

## Current classes/types and their use
+ ScriptableAnimationPlayer - The main class and only actual node, attach to a character, point the skeleton & animation player variables to your target model to get started!
+ SAPPose - The pose of your character this frame, can be blended with other poses to achieve your desired final pose
+ SAPAnimation - The animation you will sample from
+ SAPBlendSpace1D - a 1 dimensional blend space, add points and sample between them
+ SAPBlendSpace2DRadial - a 2D version of the 1D blend space, add points in 2D and pass between multiple rings of 1D Blend Spaces, useful for movement

## How to use
