#include "SAPBone.hpp"

using namespace godot;

SAPBone godot::SAPBone::Duplicate()
{
    SAPBone res;
    res.name = name;
    res.pos = pos;
    res.rot = rot;
    res.scale = scale;
    return res;
}