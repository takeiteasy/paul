/* jeff/math.c -- https://github.com/takeiteasy/jeff

 Copyright (C) 2024  George Watson

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>. */

#include "jeff.h"

static float ease_linear_None(float t, float b, float c, float d) {
    return (c * t / d + b);
}

static float ease_linear_in(float t, float b, float c, float d) {
    return (c * t / d + b);
}

static float ease_linear_out(float t, float b, float c, float d) {
    return (c * t / d + b);
}

static float ease_linear_inout(float t, float b, float c, float d) {
    return (c * t / d + b);
}

static float ease_sine_in(float t, float b, float c, float d) {
    return (-c * cosf(t / d * (M_PI / 2.0f)) + c + b);
}

static float ease_sine_out(float t, float b, float c, float d) {
    return (c * sinf(t / d * (M_PI / 2.0f)) + b);
}

static float ease_sine_inout(float t, float b, float c, float d) {
    return (-c / 2.0f * (cosf(M_PI * t / d) - 1.0f) + b);
}

static float ease_circ_in(float t, float b, float c, float d) {
    t /= d;
    return (-c * (sqrtf(1.0f - t * t) - 1.0f) + b);
}

static float ease_circ_out(float t, float b, float c, float d) {
    t = t / d - 1.0f;
    return (c * sqrtf(1.0f - t * t) + b);
}

static float ease_circ_inout(float t, float b, float c, float d) {
    if ((t /= d / 2.0f) < 1.0f)
        return (-c / 2.0f * (sqrtf(1.0f - t * t) - 1.0f) + b);
    t -= 2.0f;
    return (c / 2.0f * (sqrtf(1.0f - t * t) + 1.0f) + b);
}

static float ease_cubic_in(float t, float b, float c, float d) {
    t /= d;
    return (c * t * t * t + b);
}

static float ease_cubic_out(float t, float b, float c, float d) {
    t = t / d - 1.0f;
    return (c * (t * t * t + 1.0f) + b);
}

static float ease_cubic_inout(float t, float b, float c, float d) {
    if ((t /= d / 2.0f) < 1.0f)
        return (c / 2.0f * t * t * t + b);
    t -= 2.0f;
    return (c / 2.0f * (t * t * t + 2.0f) + b);
}

static float ease_quad_in(float t, float b, float c, float d) {
    t /= d;
    return (c * t * t + b);
}

static float ease_quad_out(float t, float b, float c, float d) {
    t /= d;
    return (-c * t * (t - 2.0f) + b);
}

static float ease_quad_inout(float t, float b, float c, float d) {
    if ((t /= d / 2) < 1)
        return (((c / 2) * (t * t)) + b);
    return (-c / 2.0f * (((t - 1.0f) * (t - 3.0f)) - 1.0f) + b);
}

static float ease_expo_in(float t, float b, float c, float d) {
    return (t == 0.0f) ? b : (c * powf(2.0f, 10.0f * (t / d - 1.0f)) + b);
}

static float ease_expo_out(float t, float b, float c, float d) {
    return (t == d) ? (b + c) : (c * (-powf(2.0f, -10.0f * t / d) + 1.0f) + b);
}

static float ease_expo_inout(float t, float b, float c, float d) {
    if (t == 0.0f)
        return b;
    if (t == d)
        return (b + c);
    if ((t /= d / 2.0f) < 1.0f)
        return (c / 2.0f * powf(2.0f, 10.0f * (t - 1.0f)) + b);

    return (c / 2.0f * (-powf(2.0f, -10.0f * (t - 1.0f)) + 2.0f) + b);
}

static float ease_back_in(float t, float b, float c, float d) {
    float s = 1.70158f;
    float postFix = t /= d;
    return (c * (postFix)*t * ((s + 1.0f) * t - s) + b);
}

static float ease_back_out(float t, float b, float c, float d) {
    float s = 1.70158f;
    t = t / d - 1.0f;
    return (c * (t * t * ((s + 1.0f) * t + s) + 1.0f) + b);
}

static float ease_back_inout(float t, float b, float c, float d) {
    float s = 1.70158f;
    if ((t /= d / 2.0f) < 1.0f) {
        s *= 1.525f;
        return (c / 2.0f * (t * t * ((s + 1.0f) * t - s)) + b);
    }

    float postFix = t -= 2.0f;
    s *= 1.525f;
    return (c / 2.0f * ((postFix)*t * ((s + 1.0f) * t + s) + 2.0f) + b);
}

static float ease_bounce_out(float t, float b, float c, float d) {
    if ((t /= d) < (1.0f / 2.75f)) {
        return (c * (7.5625f * t * t) + b);
    } else if (t < (2.0f / 2.75f)) {
        float postFix = t -= (1.5f / 2.75f);
        return (c * (7.5625f * (postFix)*t + 0.75f) + b);
    } else if (t < (2.5 / 2.75)) {
        float postFix = t -= (2.25f / 2.75f);
        return (c * (7.5625f * (postFix)*t + 0.9375f) + b);
    } else {
        float postFix = t -= (2.625f / 2.75f);
        return (c * (7.5625f * (postFix)*t + 0.984375f) + b);
    }
}

static float ease_bounce_in(float t, float b, float c, float d) {
    return (c - ease_bounce_out(d - t, 0.0f, c, d) + b);

}

static float ease_bounce_inout(float t, float b, float c, float d) {
    if (t < d / 2.0f)
        return (ease_bounce_in(t * 2.0f, 0.0f, c, d) * 0.5f + b);
    else
        return (ease_bounce_out(t * 2.0f - d, 0.0f, c, d) * 0.5f + c * 0.5f + b);
}

static float ease_elastic_in(float t, float b, float c, float d) {
    if (t == 0.0f)
        return b;
    if ((t /= d) == 1.0f)
        return (b + c);

    float p = d * 0.3f;
    float a = c;
    float s = p / 4.0f;
    float postFix = a * powf(2.0f, 10.0f * (t -= 1.0f));

    return (-(postFix * sinf((t * d - s) * (2.0f * M_PI) / p)) + b);
}

static float ease_elastic_out(float t, float b, float c, float d) {
    if (t == 0.0f)
        return b;
    if ((t /= d) == 1.0f)
        return (b + c);

    float p = d * 0.3f;
    float a = c;
    float s = p / 4.0f;

    return (a * powf(2.0f, -10.0f * t) * sinf((t * d - s) * (2.0f * M_PI) / p) + c + b);
}

static float ease_elastic_inout(float t, float b, float c, float d) {
    if (t == 0.0f)
        return b;
    if ((t /= d / 2.0f) == 2.0f)
        return (b + c);

    float p = d * (0.3f * 1.5f);
    float a = c;
    float s = p / 4.0f;

    if (t < 1.0f) {
        float postFix = a * powf(2.0f, 10.0f * (t -= 1.0f));
        return -0.5f * (postFix * sinf((t * d - s) * (2.0f * M_PI) / p)) + b;
    }

    float postFix = a * powf(2.0f, -10.0f * (t -= 1.0f));

    return (postFix * sinf((t * d - s) * (2.0f * M_PI) / p) * 0.5f + c + b);
}

float easing(enum jeff_easing_fn fn, enum jeff_easing_t type, float t, float b, float c, float d) {
    switch (fn) {
        default:
        case JEFF_EASING_LINEAR:
            switch ((int)type) {
                default:
                case 0:
                    return ease_linear_None(t, b, c, d);
                case EASE_IN:
                    return ease_linear_in(t, b, c, d);
                case EASE_OUT:
                    return ease_linear_out(t, b, c, d);
                case EASE_INOUT:
                    return ease_linear_inout(t, b, c, d);
            }
        case JEFF_EASING_SINE:
            switch (type) {
                case EASE_IN:
                    return ease_sine_in(t, b, c, d);
                case EASE_OUT:
                    return ease_sine_out(t, b, c, d);
                case EASE_INOUT:
                    return ease_sine_inout(t, b, c, d);
            };
        case JEFF_EASING_CIRCULAR:
            switch (type) {
                case EASE_IN:
                    return ease_circ_in(t, b, c, d);
                case EASE_OUT:
                    return ease_circ_out(t, b, c, d);
                case EASE_INOUT:
                    return ease_circ_inout(t, b, c, d);
            };
        case JEFF_EASING_CUBIC:
            switch (type) {
                case EASE_IN:
                    return ease_cubic_in(t, b, c, d);
                case EASE_OUT:
                    return ease_cubic_out(t, b, c, d);
                case EASE_INOUT:
                    return ease_cubic_inout(t, b, c, d);
            };
        case JEFF_EASING_QUAD:
            switch (type) {
                case EASE_IN:
                    return ease_quad_in(t, b, c, d);
                case EASE_OUT:
                    return ease_quad_out(t, b, c, d);
                case EASE_INOUT:
                    return ease_quad_inout(t, b, c, d);
            };
        case JEFF_EASING_EXPONENTIAL:
            switch (type) {
                case EASE_IN:
                    return ease_expo_in(t, b, c, d);
                case EASE_OUT:
                    return ease_expo_out(t, b, c, d);
                case EASE_INOUT:
                    return ease_expo_inout(t, b, c, d);
            };
        case JEFF_EASING_BACK:
            switch (type) {
                case EASE_IN:
                    return ease_back_in(t, b, c, d);
                case EASE_OUT:
                    return ease_back_out(t, b, c, d);
                case EASE_INOUT:
                    return ease_back_inout(t, b, c, d);
            };
        case JEFF_EASING_BOUNCE:
            switch (type) {
                case EASE_IN:
                    return ease_bounce_in(t, b, c, d);
                case EASE_OUT:
                    return ease_bounce_out(t, b, c, d);
                case EASE_INOUT:
                    return ease_bounce_inout(t, b, c, d);
            };
        case JEFF_EASING_ELASTIC:
            switch (type) {
                case EASE_IN:
                    return ease_elastic_in(t, b, c, d);
                case EASE_OUT:
                    return ease_elastic_out(t, b, c, d);
                case EASE_INOUT:
                    return ease_elastic_inout(t, b, c, d);
            };
    }
}

bool float_cmp(float a, float b) {
    return fabsf(a - b) <= EPSILON * fmaxf(1.f, fmaxf(fabsf(a), fabsf(b)));
}

bool double_cmp(double a, double b) {
    return fabs(a - b) <= EPSILON * fmax(1.f, fmax(fabs(a), fabs(b)));
}
