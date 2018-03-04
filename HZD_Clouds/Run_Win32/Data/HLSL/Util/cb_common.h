#ifndef __CB_COMMMON__
#define __CB_COMMON__

struct midi_t
{
	float KNOB_0;
	float KNOB_1;
	float KNOB_2;
	float KNOB_3;
	float KNOB_4;
	float KNOB_5;
	float KNOB_6;
	float KNOB_7;
	float KNOB_8;

	float SLIDER_0;
	float SLIDER_1;
	float SLIDER_2;
	float SLIDER_3;
	float SLIDER_4;
	float SLIDER_5;
	float SLIDER_6;
	float SLIDER_7;
	float SLIDER_8;

	float2 _padding;
};

cbuffer midi_board : register(b3)
{
	midi_t MIDI;
}

#endif