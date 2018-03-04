#ifndef __HSV_H__
#define __HSV_H__

float3 ConvertRGBtoHSV(float3 rgbColor)
{
	// What we're trying to find here
	float hue;
	float saturation;
	float value;

	float minChannel = min(rgbColor.r, min(rgbColor.g, rgbColor.b));
	float maxChannel = max(rgbColor.r, max(rgbColor.g, rgbColor.b));
	float delta = maxChannel - minChannel;

	// Hue
	if (delta <= 0.0){
		hue = 0.0;
	}
	else if (rgbColor.r == maxChannel){
		hue = (rgbColor.g - rgbColor.b) / delta;
		hue = fmod(hue, 6.0);
	}
	else if (rgbColor.g == maxChannel){
		hue = 2 + (rgbColor.b - rgbColor.r) / delta;
	}
	else{
		hue = 4 + (rgbColor.r - rgbColor.g) / delta;
	}

	hue *= 60;

	if (hue > 360){
		hue -= 360;
	}

	if (hue < 0){
		hue += 360;
	}

	// Saturation
	if (maxChannel != 0){
		saturation = delta / maxChannel;
	}
	else{
		saturation = 0;
	}

	// Value
	value = maxChannel;

	return float3(hue, saturation, value);
}

// Converts a HSV to RGB, I'm using built in float3 so hsv maps to xyz
float3 ConvertHSVtoRGB(float3 hsv)
{
	float hue = hsv.x;
	float saturation = hsv.y;
	float value = hsv.z;

	float c = value * saturation;

	float x = c * (1.0 - abs((fmod((hue / 60.0), 2.0)) - 1.0));
	float m = value - c;

	float3 color;

	if (hue < 60.0){
		color.r = c;
		color.g = x;
		color.b = 0;
	}
	else if (hue < 120.0){
		color.r = x;
		color.g = c;
		color.b = 0;
	}
	else if (hue < 180.0){
		color.r = 0;
		color.g = c;
		color.b = x;
	}
	else if (hue < 240.0){
		color.r = 0;
		color.g = x;
		color.b = c;
	}
	else if (hue < 300.0){
		color.r = x;
		color.g = 0;
		color.b = c;
	}
	else{
		color.r = c;
		color.g = 0;
		color.b = x;
	}

	return color + float3(m, m, m);
}

#endif