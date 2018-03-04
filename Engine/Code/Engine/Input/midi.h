#pragma once

#include "Engine/Core/types.h"

typedef u8 mslider_id;
typedef u8 mknob_id;
typedef u8 mbutton_id;

bool midi_init();
void midi_shutdown();
bool midi_is_connected();

bool midi_bindf_slider(mslider_id id, float* value, float min, float max);
bool midi_bindi_slider(mslider_id id, int* value, int min, int max);
bool midi_bindf_knob(mknob_id id, float* value, float min, float max);
bool midi_bindi_knob(mknob_id id, int* value, int min, int max);
bool midi_bind_button(mbutton_id id, bool* value);

bool midi_unbindf_slider(mslider_id id, float* value);
bool midi_unbindi_slider(mslider_id id, int* value);
bool midi_unbindf_knob(mknob_id id, float* value);
bool midi_unbindi_knob(mknob_id id, int* value);
bool midi_unbind_button(mbutton_id id, bool* value);