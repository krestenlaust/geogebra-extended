/*
This file is part of Renoir, a modern graphics library.

Copyright (c) 2012-2016 Coherent Labs AD and/or its licensors. All
rights reserved in all media.

The coded instructions, statements, computer programs, and/or related
material (collectively the "Data") in these files contain confidential
and unpublished information proprietary Coherent Labs and/or its
licensors, which is protected by United States of America federal
copyright law and by international treaties.

This software or source code is supplied under the terms of a license
agreement and nondisclosure agreement with Coherent Labs AD and may
not be copied, disclosed, or exploited except in accordance with the
terms of that agreement. The Data may not be disclosed or distributed to
third parties, in whole or in part, without the prior written consent of
Coherent Labs AD.

COHERENT LABS MAKES NO REPRESENTATION ABOUT THE SUITABILITY OF THIS
SOURCE CODE FOR ANY PURPOSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT
HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER, ITS AFFILIATES,
PARENT COMPANIES, LICENSORS, SUPPLIERS, OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OR PERFORMANCE OF THIS SOFTWARE OR SOURCE CODE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
// Formulas for color mixing from https://www.w3.org/TR/compositing-1/#blending
const char* CohColorMixingPS =
"float Lum(vec3 color)                                                       \n"
"{                                                                           \n"
"	return 0.3 * color.r + 0.59 * color.g + 0.11 * color.b;                  \n"
"}                                                                           \n"
"                                                                            \n"
"vec3 ClipColor(vec3 color)                                                  \n"
"{                                                                           \n"
"	float L = Lum(color);                                                    \n"
"	float fmin = min(min(color.r, color.g), color.b);                        \n"
"	float fmax = max(max(color.r, color.g), color.b);                        \n"
"                                                                            \n"
"	color = mix(color,                                                       \n"
"		L + (((color - L) * L) / (L - fmin)),                                \n"
"		float(fmin < 0.0));                                                  \n"
"                                                                            \n"
"	color = mix(color,                                                       \n"
"		L + (((color - L) * (1.0 - L)) / (fmax - L)),                        \n"
"		float(fmax > 1.0));                                                  \n"
"                                                                            \n"
"	return color;                                                            \n"
"}                                                                           \n"
"                                                                            \n"
"vec3 SetLum(vec3 color, float lum)                                          \n"
"{                                                                           \n"
"	float d = lum - Lum(color);                                              \n"
"	color += d;                                                              \n"
"	return ClipColor(color);                                                 \n"
"}                                                                           \n"
"                                                                            \n"
"float Sat(vec3 color)                                                       \n"
"{                                                                           \n"
"	return max(max(color.r, color.g), color.b) -                             \n"
"		min(min(color.r, color.g), color.b);                                 \n"
"}                                                                           \n"
"                                                                            \n"
"vec3 SetSatInner(vec3 color, float sat)                                     \n"
"{                                                                           \n"
"	// Here the |color| values are in ascending order, i.e.                  \n"
"	// color.x <= color.y <= color.z                                         \n"
"                                                                            \n"
"	if (color.z > color.x)                                                   \n"
"	{                                                                        \n"
"		color.y = (((color.y - color.x) * sat) / (color.z - color.x));       \n"
"		color.z = sat;                                                       \n"
"	}                                                                        \n"
"	else                                                                     \n"
"	{                                                                        \n"
"		color.yz = vec2(0.0, 0.0);                                           \n"
"	}                                                                        \n"
"	return vec3(0.0, color.y, color.z);                                      \n"
"}                                                                           \n"
"                                                                            \n"
"vec3 SetSat(vec3 color, float sat)                                          \n"
"{                                                                           \n"
"	if (color.r <= color.g)                                                  \n"
"	{                                                                        \n"
"		if (color.g <= color.b)                                              \n"
"		{                                                                    \n"
"			color.rgb = SetSatInner(color.rgb, sat);                         \n"
"		}                                                                    \n"
"		else if (color.r <= color.b)                                         \n"
"		{                                                                    \n"
"			color.rbg = SetSatInner(color.rbg, sat);                         \n"
"		}                                                                    \n"
"		else                                                                 \n"
"		{                                                                    \n"
"			color.brg = SetSatInner(color.brg, sat);                         \n"
"		}                                                                    \n"
"	}                                                                        \n"
"	else if (color.r <= color.b)                                             \n"
"	{                                                                        \n"
"		color.grb = SetSatInner(color.grb, sat);                             \n"
"	}                                                                        \n"
"	else if (color.g <= color.b)                                             \n"
"	{                                                                        \n"
"		color.gbr = SetSatInner(color.gbr, sat);                             \n"
"	}                                                                        \n"
"	else                                                                     \n"
"	{                                                                        \n"
"		color.bgr = SetSatInner(color.bgr, sat);                             \n"
"	}                                                                        \n"
"	return color;                                                            \n"
"}                                                                           \n"
"                                                                            \n"
"vec3 ColorMixScreen(vec3 backdrop, vec3 source)                             \n"
"{                                                                           \n"
"	return backdrop + source - backdrop * source;                            \n"
"}                                                                           \n"
"                                                                            \n"
"vec3 ColorMixMultiply(vec3 backdrop, vec3 source)                           \n"
"{                                                                           \n"
"	return backdrop * source;                                                \n"
"}                                                                           \n"
"                                                                            \n"
"vec3 ColorMixHardLight(vec3 backdrop, vec3 source)                          \n"
"{                                                                           \n"
"	vec3 coef = step(0.5, source);                                           \n"
"	return mix(ColorMixScreen(backdrop, 2.0 * source - 1.0),                 \n"
"		ColorMixMultiply(backdrop, 2.0 * source), coef);                     \n"
"}                                                                           \n"
"                                                                            \n"
"vec3 ColorMixSoftLight(vec3 backdrop, vec3 source)                          \n"
"{                                                                           \n"
"	// TODO: Check if branching perf is better, diffuseB calc is heavy       \n"
"	vec3 diffuseBCoef = step(0.25, backdrop);                                \n"
"	vec3 diffuseB = mix(sqrt(backdrop),                                      \n"
"		((16.0 * backdrop - 12.0) * backdrop + 4.0) * backdrop,              \n"
"		diffuseBCoef);                                                       \n"
"                                                                            \n"
"	vec3 coef = step(0.5, source);                                           \n"
"	return mix(backdrop + (2.0 * source - 1.0) * (diffuseB - backdrop),      \n"
"		backdrop - (1.0 - 2.0 * source) * backdrop * (1.0 - backdrop),       \n"
"		coef);                                                               \n"
"}                                                                           \n"
"                                                                            \n"
"vec3 BlendFunction(vec3 backdrop, vec3 source, int mode)                    \n"
"{                                                                           \n"
"	// Switch not supported in SM3                                           \n"
"	if (mode == 0)                                                           \n"
"	{                                                                        \n"
"		// Normal mode                                                       \n"
"		return source;                                                       \n"
"	}                                                                        \n"
"	else if (mode == 1)                                                      \n"
"	{                                                                        \n"
"		// Multiply                                                          \n"
"		return ColorMixMultiply(backdrop, source);                           \n"
"	}                                                                        \n"
"	else if (mode == 2)                                                      \n"
"	{                                                                        \n"
"		// Screen                                                            \n"
"		return ColorMixScreen(backdrop, source);                             \n"
"	}                                                                        \n"
"	else if (mode == 3)                                                      \n"
"	{                                                                        \n"
"		// Overlay                                                           \n"
"		return ColorMixHardLight(source, backdrop); // Inverted hard-light   \n"
"	}                                                                        \n"
"	else if (mode == 4)                                                      \n"
"	{                                                                        \n"
"		// Darken                                                            \n"
"		return min(source, backdrop);                                        \n"
"	}                                                                        \n"
"	else if (mode == 5)                                                      \n"
"	{                                                                        \n"
"		// Lighten                                                           \n"
"		return max(source, backdrop);                                        \n"
"	}                                                                        \n"
"	else if (mode == 6)                                                      \n"
"	{                                                                        \n"
"		// Color dodge                                                       \n"
"		return min(vec3(1.0), backdrop / max(1.0 - source, 0.0001));         \n"
"	}                                                                        \n"
"	else if (mode == 7)                                                      \n"
"	{                                                                        \n"
"		// Color burn                                                        \n"
"		return 1.0 - min(vec3(1.0), (1.0 - backdrop) / max(source, 0.0001)); \n"
"	}                                                                        \n"
"	else if (mode == 8)                                                      \n"
"	{                                                                        \n"
"		// Hard light                                                        \n"
"		return ColorMixHardLight(backdrop, source);                          \n"
"	}                                                                        \n"
"	else if (mode == 9)                                                      \n"
"	{                                                                        \n"
"		// Soft light                                                        \n"
"		return ColorMixSoftLight(backdrop, source);                          \n"
"	}                                                                        \n"
"	else if (mode == 10)                                                     \n"
"	{                                                                        \n"
"		// Difference                                                        \n"
"		return abs(backdrop - source);                                       \n"
"	}                                                                        \n"
"	else if (mode == 11)                                                     \n"
"	{                                                                        \n"
"		// Exclusion                                                         \n"
"		return backdrop + source - 2.0 * backdrop * source;                  \n"
"	}                                                                        \n"
"	else if (mode == 12)                                                     \n"
"	{                                                                        \n"
"		// Hue                                                               \n"
"		return SetLum(SetSat(source, Sat(backdrop)), Lum(backdrop));         \n"
"	}                                                                        \n"
"	else if (mode == 13)                                                     \n"
"	{                                                                        \n"
"		// Saturation                                                        \n"
"		return SetLum(SetSat(backdrop, Sat(source)), Lum(backdrop));         \n"
"	}                                                                        \n"
"	else if (mode == 14)                                                     \n"
"	{                                                                        \n"
"		// Color                                                             \n"
"		return SetLum(source, Lum(backdrop));                                \n"
"	}                                                                        \n"
"	else if (mode == 15)                                                     \n"
"	{                                                                        \n"
"		// Luminosity                                                        \n"
"		return SetLum(backdrop, Lum(source));                                \n"
"	}                                                                        \n"
"	else                                                                     \n"
"	{                                                                        \n"
"		return vec3(0.0);                                                    \n"
"	}                                                                        \n"
"}                                                                           \n"
"                                                                            \n"
"void main()                                                                 \n"
"{                                                                           \n"
"	vec4 backdrop = texture(txBuffer, PSAdditional.zw);                      \n"
"	vec4 source = texture(txBuffer1, PSAdditional.xy);                       \n"
"                                                                            \n"
"	vec3 backdropUnprem = backdrop.rgb / max(backdrop.a, 0.0001);            \n"
"	vec3 sourceUnprem = source.rgb / max(source.a, 0.0001);                  \n"
"	vec4 result =                                                            \n"
"		(1.0 - backdrop.a) * source +                                        \n"
"		source.a * backdrop.a * vec4(                                        \n"
"			clamp(BlendFunction(backdropUnprem, sourceUnprem,                \n"
"				int(PrimProps0.x)), 0.0, 1.0), 1.0) +                        \n"
"		(1.0 - source.a) * backdrop;                                         \n"
"	outColor = result;                                                       \n"
"}\n";
