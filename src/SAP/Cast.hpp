#pragma once
#include <mutex>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#define PRINT(...) godot::UtilityFunctions::print(__VA_ARGS__);
#define PRINTERR(...) godot::UtilityFunctions::printerr(__VA_ARGS__);

#define MAKE_PROPGETSET(type, name)          \
    type get_##name() const { return name; } \
    void set_##name(type value) { name = value; }

#define MAKE_PROPGETSET_DEF(type, name) \
    type name;                          \
    MAKE_PROPGETSET(type, name);

#define MAKE_PROPBINDING(klass, name, v_type)                                \
    ClassDB::bind_method(D_METHOD("get_" #name), &klass::get_##name);        \
    ClassDB::bind_method(D_METHOD("set_" #name, #name), &klass::set_##name); \
    ADD_PROPERTY(PropertyInfo(v_type, #name), "set_" #name, "get_" #name);

namespace godot
{
    template <typename T>
    static T *Cast(Node *n)
    {
        return Object::cast_to<T>(n);
    }

    template <typename T>
    static T *CastErr(Node *n)
    {
        if (n == nullptr)
            PRINTERR("Cast Err recieved nullptr");
        return Object::cast_to<T>(n);
    }

    static Node *FindChild(Node *parent, const String &pattern)
    {
        Node *c;
        for (int i = 0; i < parent->get_child_count(); ++i)
        {
            c = parent->get_child(i);
            if (c->get_name().match(pattern))
                return c;
        }
        PRINTERR("Failed to find ", pattern);
        return nullptr;
    }

    template <typename T>
    static T *FindChild(Node *parent, const String &pattern)
    {
        return Object::cast_to<T>(FindChild(parent, pattern));
    }

    static std::vector<Node *> GetAllChildren(Node *parent)
    {
        auto res = std::vector<Node *>();
        for (int i = 0; i < parent->get_child_count(); ++i)
            res.push_back(parent->get_child(i));
        return res;
    }
}
