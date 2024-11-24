#include "register_types.h"

#include "SAP/ScriptableAnimationPlayer.hpp"
#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_SAP_module(ModuleInitializationLevel p_level)
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
    {
        return;
    }

    GDREGISTER_RUNTIME_CLASS(ScriptableAnimationPlayer);
    GDREGISTER_RUNTIME_CLASS(SAPAnimation);
    GDREGISTER_RUNTIME_CLASS(SAPPose);
    GDREGISTER_RUNTIME_CLASS(SAPBlendSpace1D);
    GDREGISTER_RUNTIME_CLASS(SAPBlendSpace2DRadial);
}

void uninitialize_SAP_module(ModuleInitializationLevel p_level)
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
    {
        return;
    }
}

extern "C"
{
    // Initialization.
    GDExtensionBool GDE_EXPORT SAP_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization)
    {
        godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

        init_obj.register_initializer(initialize_SAP_module);
        init_obj.register_terminator(uninitialize_SAP_module);
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

        return init_obj.init();
    }
}
