/* paul/math.c -- https://github.com/takeiteasy/paul

 Copyright (C) 2025  George Watson

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

/*******************************************************************************************
*
*   reasings - raylib easings library, based on Robert Penner library
*
*   Useful easing functions for values animation
*
*   This header uses:
*       #define REASINGS_STATIC_INLINE      // Inlines all functions code, so it runs faster.
*                                           // This requires lots of memory on system.
*   How to use:
*   The four inputs t,b,c,d are defined as follows:
*   t = current time (in any unit measure, but same unit as duration)
*   b = starting value to interpolate
*   c = the total change in value of b that needs to occur
*   d = total time it should take to complete (duration)
*
*   Example:
*
*   int currentTime = 0;
*   int duration = 100;
*   float startPositionX = 0.0f;
*   float finalPositionX = 30.0f;
*   float currentPositionX = startPositionX;
*
*   while (currentPositionX < finalPositionX)
*   {
*       currentPositionX = EaseSineIn(currentTime, startPositionX, finalPositionX - startPositionX, duration);
*       currentTime++;
*   }
*
*   A port of Robert Penner's easing equations to C (http://robertpenner.com/easing/)
*
*   Robert Penner License
*   ---------------------------------------------------------------------------------
*   Open source under the BSD License.
*
*   Copyright (c) 2001 Robert Penner. All rights reserved.
*
*   Redistribution and use in source and binary forms, with or without modification,
*   are permitted provided that the following conditions are met:
*
*       - Redistributions of source code must retain the above copyright notice,
*         this list of conditions and the following disclaimer.
*       - Redistributions in binary form must reproduce the above copyright notice,
*         this list of conditions and the following disclaimer in the documentation
*         and/or other materials provided with the distribution.
*       - Neither the name of the author nor the names of contributors may be used
*         to endorse or promote products derived from this software without specific
*         prior written permission.
*
*   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
*   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
*   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
*   IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
*   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
*   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
*   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
*   OF THE POSSIBILITY OF SUCH DAMAGE.
*   ---------------------------------------------------------------------------------
*
*   Copyright (c) 2015-2024 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#include "paul.h"

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

float paul_ease(enum paul_easing_fn fn, enum paul_easing_t type, float t, float b, float c, float d) {
    switch (fn) {
        default:
        case PAUL_EASING_LINEAR:
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
        case PAUL_EASING_SINE:
            switch (type) {
                case EASE_IN:
                    return ease_sine_in(t, b, c, d);
                case EASE_OUT:
                    return ease_sine_out(t, b, c, d);
                case EASE_INOUT:
                    return ease_sine_inout(t, b, c, d);
            };
        case PAUL_EASING_CIRCULAR:
            switch (type) {
                case EASE_IN:
                    return ease_circ_in(t, b, c, d);
                case EASE_OUT:
                    return ease_circ_out(t, b, c, d);
                case EASE_INOUT:
                    return ease_circ_inout(t, b, c, d);
            };
        case PAUL_EASING_CUBIC:
            switch (type) {
                case EASE_IN:
                    return ease_cubic_in(t, b, c, d);
                case EASE_OUT:
                    return ease_cubic_out(t, b, c, d);
                case EASE_INOUT:
                    return ease_cubic_inout(t, b, c, d);
            };
        case PAUL_EASING_QUAD:
            switch (type) {
                case EASE_IN:
                    return ease_quad_in(t, b, c, d);
                case EASE_OUT:
                    return ease_quad_out(t, b, c, d);
                case EASE_INOUT:
                    return ease_quad_inout(t, b, c, d);
            };
        case PAUL_EASING_EXPONENTIAL:
            switch (type) {
                case EASE_IN:
                    return ease_expo_in(t, b, c, d);
                case EASE_OUT:
                    return ease_expo_out(t, b, c, d);
                case EASE_INOUT:
                    return ease_expo_inout(t, b, c, d);
            };
        case PAUL_EASING_BACK:
            switch (type) {
                case EASE_IN:
                    return ease_back_in(t, b, c, d);
                case EASE_OUT:
                    return ease_back_out(t, b, c, d);
                case EASE_INOUT:
                    return ease_back_inout(t, b, c, d);
            };
        case PAUL_EASING_BOUNCE:
            switch (type) {
                case EASE_IN:
                    return ease_bounce_in(t, b, c, d);
                case EASE_OUT:
                    return ease_bounce_out(t, b, c, d);
                case EASE_INOUT:
                    return ease_bounce_inout(t, b, c, d);
            };
        case PAUL_EASING_ELASTIC:
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

bool paul_float_cmp(float a, float b) {
    return fabsf(a - b) <= _EPSILON * fmaxf(1.f, fmaxf(fabsf(a), fabsf(b)));
}

bool paul_double_cmp(double a, double b) {
    return fabs(a - b) <= _EPSILON * fmax(1.f, fmax(fabs(a), fabs(b)));
}
