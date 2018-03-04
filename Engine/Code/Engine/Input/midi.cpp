#include "Engine/Input/midi.h"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Common.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/log.h"

#include <windows.h>

#pragma comment(lib, "winmm.lib")

/* 
    This is for the Worlde Easy-Control 9
    Since I'm not trying to support every MIDI controller in the world, I'm just
    going to code against the only device I have
*/

// these are the ids that get passed from the device to the app
#define MIDI_SLIDER_MIN_ID 3
#define MIDI_SLIDER_MAX_ID 11
#define MIDI_KNOB_MIN_ID 14
#define MIDI_KNOB_MAX_ID 22
#define MIDI_BUTTON_MIN_ID 23
#define MIDI_BUTTON_MAX_ID 31 

// these are used for how we store binding internally
#define ARES_CONTROL_MIN_ID 0
#define ARES_CONTROL_MAX_ID 8
#define ARES_SLIDER_START_INDEX 0
#define ARES_KNOB_START_INDEX 9
#define ARES_BUTTON_START_INDEX 18
#define ARES_NUM_TOTAL_CONTROLS 27

enum class MidiBindingType : u8
{
    BOOL,
    INT,
    FLOAT,
    NUM_BINDING_TYPES
};

struct midi_binding_t
{
    midi_binding_t* m_next;
    midi_binding_t* m_prev;

    MidiBindingType m_type;

    union{
        float* m_bound_float;
        int* m_bound_int;
        bool* m_bound_bool;
    };

    union{
        float m_float_min;
        int m_int_min;
    };

    union{
        float m_float_max;
        int m_int_max;
    };
};

global_variable midi_binding_t** g_bindings = nullptr;
global_variable bool g_midi_is_connected = false;

internal_function
u8 slider_id_to_control_id(mslider_id sid)
{
    return sid;
}

internal_function
u8 knob_id_to_control_id(mslider_id sid)
{
    return 9 + sid;
}

internal_function
u8 button_id_to_control_id(mslider_id sid)
{
    return 18 + sid;
}

internal_function
void insert_binding(u8 control_id, midi_binding_t* new_binding)
{
    midi_binding_t* binding_head = g_bindings[control_id];

    if(nullptr == binding_head){
        g_bindings[control_id] = new_binding;
    } else{
        midi_binding_t* prev_binding = binding_head->m_prev;

        prev_binding->m_next = new_binding;
        new_binding->m_prev = prev_binding;

        new_binding->m_next = binding_head;
        binding_head->m_prev = new_binding;
    }
}

internal_function
void unbind(u8 control_id, void* value)
{
    midi_binding_t* binding_head = g_bindings[control_id];
    if(nullptr == binding_head){
        return;
    }

    // loop through until we find the binding
    midi_binding_t* iter = binding_head;
    do{
        // we found our bound value
        if((void*)iter->m_bound_float == value){
            // fix up next and prev pointers
            midi_binding_t* prev = iter->m_prev;
            midi_binding_t* next = iter->m_next;
            prev->m_next = next;
            next->m_prev = prev;

            // if the binding we are removing is our head, then we need
            // to make sure we move where our head points
            if(iter == binding_head){
                // if iter == next, we only have one binding
                g_bindings[control_id] = (iter == next) ? nullptr : next;
            }

            // delete and return
            SAFE_DELETE(iter);
            return;
        }

        iter = iter->m_next;
    }while(iter != binding_head);
}

internal_function
void update_bound_values(u8 control_id, u8 new_value)
{
	if(!g_midi_is_connected){
		return;
	}

    midi_binding_t* binding_head = g_bindings[control_id];
    if(nullptr == binding_head){
        return;
    }

    // loop through until we find the binding
    midi_binding_t* iter = binding_head;
    do{
        if(MidiBindingType::BOOL == iter->m_type){
            bool button_pressed = (new_value > 64);
            *iter->m_bound_bool = button_pressed;
        }

        if(MidiBindingType::INT == iter->m_type){
            float value_as_float = (float)new_value;
            float mapped_value = MapFloatToRange(value_as_float, 
                                                 0.0f, 
                                                 127.0f, 
                                                 (float)iter->m_int_min, 
                                                 (float)iter->m_int_max);
            *iter->m_bound_int = (int)mapped_value;
        }

        if(MidiBindingType::FLOAT == iter->m_type){
            float value_as_float = (float)new_value;
            float mapped_value = MapFloatToRange(value_as_float, 
                                                 0.0f, 
                                                 127.0f, 
                                                 (float)iter->m_float_min, 
                                                 (float)iter->m_float_max);
            *iter->m_bound_float = mapped_value;
        }

        iter = iter->m_next;
    }while(iter != binding_head);
}

internal_function
void add_bindingf(u8 control_id, float* value, float min, float max)
{

    midi_binding_t* new_binding = new midi_binding_t();
    new_binding->m_type = MidiBindingType::FLOAT;
    new_binding->m_bound_float = value;
    new_binding->m_float_min = min;
    new_binding->m_float_max = max;
    new_binding->m_next = new_binding;
    new_binding->m_prev = new_binding;

    insert_binding(control_id, new_binding);
}

internal_function
void add_bindingi(u8 control_id, int* value, int min, int max)
{
    midi_binding_t* new_binding = new midi_binding_t();
    new_binding->m_type = MidiBindingType::INT;
    new_binding->m_bound_int = value;
    new_binding->m_int_min = min;
    new_binding->m_int_max = max;
    new_binding->m_next = new_binding;
    new_binding->m_prev = new_binding;

    insert_binding(control_id, new_binding);
}

internal_function
void add_bindingb(u8 control_id, bool* value)
{
    midi_binding_t* new_binding = new midi_binding_t();
    new_binding->m_type = MidiBindingType::BOOL;
    new_binding->m_bound_bool = value;
    new_binding->m_next = new_binding;
    new_binding->m_prev = new_binding;

    insert_binding(control_id, new_binding);
}

internal_function
void CALLBACK midi_cb(HMIDIIN   midi_handle,
                      UINT      msg_id,
                      DWORD_PTR user_data,
                      DWORD_PTR message,
                      DWORD_PTR timestamp)
{
    UNUSED(midi_handle);
    UNUSED(user_data);
    UNUSED(timestamp);

    switch(msg_id){
        case MIM_OPEN:{
            g_midi_is_connected = true;
        } break;

        case MIM_CLOSE:{
            g_midi_is_connected = false;
        } break;

        case MIM_DATA:{
            WORD low_word = LOWORD(message);
            WORD hi_word = HIWORD(message);
            u8 control_id = (u8)HIBYTE(low_word); // control id
            u8 new_value = (u8)LOBYTE(hi_word); // control value

            if(control_id >= MIDI_SLIDER_MIN_ID && control_id <= MIDI_SLIDER_MAX_ID){
                control_id -= MIDI_SLIDER_MIN_ID;
                control_id += ARES_SLIDER_START_INDEX;
            }

            if(control_id >= MIDI_KNOB_MIN_ID && control_id <= MIDI_KNOB_MAX_ID){
                control_id -= MIDI_KNOB_MIN_ID;
                control_id += ARES_KNOB_START_INDEX;
            }

            if(control_id >= MIDI_BUTTON_MIN_ID && control_id <= MIDI_BUTTON_MAX_ID){
                control_id -= MIDI_BUTTON_MIN_ID;
                control_id += ARES_BUTTON_START_INDEX;
            }

            update_bound_values(control_id, new_value);

        } break;
    }
}

bool midi_init()
{
    UINT num_devices = midiInGetNumDevs();
    if(num_devices > 0){
        MIDIINCAPSA dev_caps;
        UINT caps_size = sizeof(dev_caps);

        // get the device name and log it
        MMRESULT result = midiInGetDevCapsA(0, &dev_caps, caps_size);
        if(MMSYSERR_NOERROR == result){
            log_printf("Midi Device Found: %s", dev_caps.szPname);
        } else{
            log_printf("MIDI Error: Failed to query device %i capabilities", 0);
            return false;
        }

        // open the midi device
        HMIDIIN midi_handle;
        result = midiInOpen(&midi_handle, 0, (DWORD_PTR)&midi_cb, NULL, CALLBACK_FUNCTION);

        // start reading input from midi device
        if(MMSYSERR_NOERROR == result){
            midiInStart(midi_handle);
        } else{
            log_printf("MIDI Error: Failed to open device %i", 0);
            return false;
        }
    } else{
        log_printf("No MIDI devices found.");
        return false;
    }

    // setup binding slots
    g_bindings = (midi_binding_t**)malloc(sizeof(midi_binding_t*) * ARES_NUM_TOTAL_CONTROLS);
    MemZeroArray(g_bindings, ARES_NUM_TOTAL_CONTROLS);

    return true;
}

void midi_shutdown()
{
    if(nullptr == g_bindings){
        return;
    }

    // for each control slot
    for(uint bidx = 0; bidx < ARES_NUM_TOTAL_CONTROLS; ++bidx){
        midi_binding_t* binding_head = g_bindings[bidx];
        if(nullptr == binding_head){
            continue;
        }

        // for each binding
        midi_binding_t* iter = binding_head;
        do{
            midi_binding_t* next = iter->m_next;
            SAFE_DELETE(iter);
            iter = next;
        }while(iter != binding_head);
    }

    free(g_bindings);
}

bool midi_is_connected()
{
    return g_midi_is_connected;
}

bool midi_bindf_slider(mslider_id id, float* value, float min, float max)
{
    ASSERT_RETURN_VALUE((id >= ARES_CONTROL_MIN_ID && id <= ARES_CONTROL_MAX_ID), false);
    ASSERT_RETURN_VALUE(g_midi_is_connected, false);
    u8 control_id = slider_id_to_control_id(id);
    add_bindingf(control_id, value, min, max);
    return true;
}

bool midi_bindi_slider(mslider_id id, int* value, int min, int max)
{
    ASSERT_RETURN_VALUE((id >= ARES_CONTROL_MIN_ID && id <= ARES_CONTROL_MAX_ID), false);
    ASSERT_RETURN_VALUE(g_midi_is_connected, false);
    u8 control_id = slider_id_to_control_id(id);
    add_bindingi(control_id, value, min, max);
    return true;
}

bool midi_bindf_knob(mknob_id id, float* value, float min, float max)
{
    ASSERT_RETURN_VALUE((id >= ARES_CONTROL_MIN_ID && id <= ARES_CONTROL_MAX_ID), false);
    ASSERT_RETURN_VALUE(g_midi_is_connected, false);
    u8 control_id = knob_id_to_control_id(id);
    add_bindingf(control_id, value, min, max);
    return true;
}

bool midi_bindi_knob(mknob_id id, int* value, int min, int max)
{
    ASSERT_RETURN_VALUE((id >= ARES_CONTROL_MIN_ID && id <= ARES_CONTROL_MAX_ID), false);
    ASSERT_RETURN_VALUE(g_midi_is_connected, false);
    u8 control_id = knob_id_to_control_id(id);
    add_bindingi(control_id, value, min, max);
    return true;
}

bool midi_bind_button(mbutton_id id, bool* value)
{
    ASSERT_RETURN_VALUE((id >= ARES_CONTROL_MIN_ID && id <= ARES_CONTROL_MAX_ID), false);
    ASSERT_RETURN_VALUE(g_midi_is_connected, false);
    u8 control_id = button_id_to_control_id(id);
    add_bindingb(control_id, value);
    return true;
}

bool midi_unbindf_slider(mslider_id id, float* value)
{
    ASSERT_RETURN_VALUE((id >= ARES_CONTROL_MIN_ID && id <= ARES_CONTROL_MAX_ID), false);
    ASSERT_RETURN_VALUE(g_midi_is_connected, false);
    u8 control_id = slider_id_to_control_id(id);
    unbind(control_id, value);
    return true;
}

bool midi_unbindi_slider(mslider_id id, int* value)
{
    ASSERT_RETURN_VALUE((id >= ARES_CONTROL_MIN_ID && id <= ARES_CONTROL_MAX_ID), false);
    ASSERT_RETURN_VALUE(g_midi_is_connected, false);
    u8 control_id = slider_id_to_control_id(id);
    unbind(control_id, value);
    return true;
}

bool midi_unbindf_knob(mknob_id id, float* value)
{
    ASSERT_RETURN_VALUE((id >= ARES_CONTROL_MIN_ID && id <= ARES_CONTROL_MAX_ID), false);
    ASSERT_RETURN_VALUE(g_midi_is_connected, false);
    u8 control_id = knob_id_to_control_id(id);
    unbind(control_id, value);
    return true;
}

bool midi_unbindi_knob(mknob_id id, int* value)
{
    ASSERT_RETURN_VALUE((id >= ARES_CONTROL_MIN_ID && id <= ARES_CONTROL_MAX_ID), false);
    ASSERT_RETURN_VALUE(g_midi_is_connected, false);
    u8 control_id = knob_id_to_control_id(id);
    unbind(control_id, value);
    return true;
}

bool midi_unbind_button(mbutton_id id, bool* value)
{
    ASSERT_RETURN_VALUE((id >= ARES_CONTROL_MIN_ID && id <= ARES_CONTROL_MAX_ID), false);
    ASSERT_RETURN_VALUE(g_midi_is_connected, false);
    u8 control_id = button_id_to_control_id(id);
    unbind(control_id, value);
    return true;
}