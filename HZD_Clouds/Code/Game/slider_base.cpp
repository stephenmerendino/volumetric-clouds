#include "Game/slider_base.h"

AABB2 SliderBase::calc_bounds(const Vector2& top_left)
{
    Vector2 mins(top_left.x, top_left.y - get_height());
    Vector2 maxs = mins + Vector2(get_width(), get_height());
    return AABB2(mins, maxs);
}