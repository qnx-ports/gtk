#ifndef _PATTERN_
#define _PATTERN_

#include "common.glsl"
#include "gradient.glsl"
#include "rect.glsl"

#ifdef GSK_FRAGMENT_SHADER

vec4 stack[GSK_GPU_PATTERN_STACK_SIZE];
uint stack_size = 0u;

void
stack_push (vec4 data)
{
  stack[stack_size] = data;
  stack_size++;
}

vec4
stack_pop (void)
{
  stack_size--;
  return stack[stack_size];
}

uint
read_uint (inout uint reader)
{
  uint result = gsk_get_uint (reader);
  reader++;
  return result;
}

float
read_float (inout uint reader)
{
  float result = gsk_get_float (reader);
  reader++;
  return result;
}

vec2
read_vec2 (inout uint reader)
{
  return vec2 (read_float (reader), read_float (reader));
}

vec4
read_vec4 (inout uint reader)
{
  return vec4 (read_float (reader), read_float (reader), read_float (reader), read_float (reader));
}

Rect
read_rect (inout uint reader)
{
  return rect_from_gsk (read_vec4 (reader));
}

mat4
read_mat4 (inout uint reader)
{
  mat4 result = mat4 (gsk_get_float (reader + 0u), gsk_get_float (reader + 1u),
                      gsk_get_float (reader + 2u), gsk_get_float (reader + 3u),
                      gsk_get_float (reader + 4u), gsk_get_float (reader + 5u),
                      gsk_get_float (reader + 6u), gsk_get_float (reader + 7u),
                      gsk_get_float (reader + 8u), gsk_get_float (reader + 9u),
                      gsk_get_float (reader + 10u), gsk_get_float (reader + 11u),
                      gsk_get_float (reader + 12u), gsk_get_float (reader + 13u),
                      gsk_get_float (reader + 14u), gsk_get_float (reader + 15u));
  reader += 16u;
  return result;
}

Gradient
read_gradient (inout uint reader)
{
  Gradient gradient = gradient_new (reader);
  reader += gradient_get_size (gradient);

  return gradient;
}

void
clip_pattern (inout uint reader,
              inout vec4 color,
              vec2       pos)
{
  Rect clip = read_rect (reader);
  float alpha = rect_coverage (clip, pos);

  color *= alpha;
}

void
opacity_pattern (inout uint reader,
                 inout vec4 color,
                 vec2       pos)
{
  float opacity = read_float (reader);

  color *= opacity;
}

void
color_matrix_pattern (inout uint reader,
                      inout vec4 color,
                      vec2       pos)
{
  mat4 matrix = read_mat4 (reader);
  vec4 offset = read_vec4 (reader);

  color = color_unpremultiply (color);

  color = matrix * color + offset;
  color = clamp(color, 0.0, 1.0);

  color = color_premultiply (color);
}

void
repeat_push_pattern (inout uint reader,
                     inout vec2 pos)
{
  stack_push (vec4 (pos, 0.0, 0.0));
  Rect bounds = read_rect (reader);

  vec2 size = rect_size (bounds);
  pos = mod (pos - bounds.bounds.xy, size);
  /* make sure we have a positive result */
  pos = mix (pos, pos + size, lessThan (pos, vec2 (0.0)));
  pos += bounds.bounds.xy;
}

void
position_pop_pattern (inout uint reader,
                      inout vec2 pos)
{
  pos = stack_pop ().xy;
}

void
cross_fade_pattern (inout uint reader,
                    inout vec4 color)
{
  vec4 start = stack_pop ();
  float progress = read_float (reader);

  color = mix (start, color, progress);
}

void
mask_alpha_pattern (inout uint reader,
                    inout vec4 color)
{
  vec4 source = stack_pop ();

  color = source * color.a;
}

void
mask_inverted_alpha_pattern (inout uint reader,
                             inout vec4 color)
{
  vec4 source = stack_pop ();

  color = source * (1.0 - color.a);
}

void
mask_luminance_pattern (inout uint reader,
                        inout vec4 color)
{
  vec4 source = stack_pop ();

  color = source * luminance (color.rgb);
}

void
mask_inverted_luminance_pattern (inout uint reader,
                                 inout vec4 color)
{
  vec4 source = stack_pop ();

  color = source * (color.a - luminance (color.rgb));
}

vec4
glyphs_pattern (inout uint reader,
                vec2       pos)
{
  float opacity = 0.0;
  vec4 color = color_premultiply (read_vec4 (reader));
  uint num_glyphs = read_uint (reader);
  uint i;

  for (i = 0u; i < num_glyphs; i++)
    {
      uint tex_id = read_uint (reader);
      Rect glyph_bounds = read_rect (reader);
      vec4 tex_rect = read_vec4 (reader);

      float coverage = rect_coverage (glyph_bounds, pos);
      if (coverage > 0.0)
        opacity += coverage * gsk_texture (tex_id, (pos - push.scale * tex_rect.xy) / (push.scale * tex_rect.zw)).a;
    }

  return color * opacity;
}

vec4
texture_pattern (inout uint reader,
                 vec2       pos)
{
  uint tex_id = read_uint (reader);
  vec4 tex_rect = read_vec4 (reader);

  return gsk_texture (tex_id, (pos - push.scale * tex_rect.xy) / (push.scale * tex_rect.zw));
}

vec4
linear_gradient_pattern (inout uint reader,
                         vec2       pos,
                         bool       repeating)
{
  vec2 start = read_vec2 (reader) * push.scale;
  vec2 end = read_vec2 (reader) * push.scale;
  Gradient gradient = read_gradient (reader);

  vec2 line = end - start;
  float line_length = dot (line, line);
  float offset = dot (pos - start, line) / line_length;
  float d_offset = 0.5 * fwidth (offset);

  if (repeating)
    return gradient_get_color_repeating (gradient, offset - d_offset, offset + d_offset);
  else
    return gradient_get_color (gradient, offset - d_offset, offset + d_offset);
}

vec4
radial_gradient_pattern (inout uint reader,
                         vec2       pos,
                         bool       repeating)
{
  vec2 center = read_vec2 (reader) * push.scale;
  vec2 radius = read_vec2 (reader) * push.scale;
  float start = read_float (reader);
  float end = read_float (reader);
  Gradient gradient = read_gradient (reader);

  float offset = length ((pos - center) / radius);
  offset = (offset - start) / (end - start);
  float d_offset = 0.5 * fwidth (offset);

  if (repeating)
    return gradient_get_color_repeating (gradient, offset - d_offset, offset + d_offset);
  else
    return gradient_get_color (gradient, offset - d_offset, offset + d_offset);
}

vec4
conic_gradient_pattern (inout uint reader,
                        vec2       pos)
{
  vec2 center = read_vec2 (reader);
  float angle = read_float (reader);
  Gradient gradient = read_gradient (reader);

  /* scaling modifies angles, so be sure to use right coordinate system */
  pos = pos / push.scale - center;
  float offset = atan (pos.y, pos.x);
  offset = degrees (offset + angle) / 360.0;
  float overflow = fract (offset + 0.5);
  offset = fract (offset);
  float d_offset = max (0.00001, 0.5 * min (fwidth (offset), fwidth (overflow)));

  return gradient_get_color_repeating (gradient, offset - d_offset, offset + d_offset);
}

vec4
color_pattern (inout uint reader)
{
  vec4 color = read_vec4 (reader);

  return color_premultiply (color);
}

vec4
pattern (uint reader,
         vec2 pos)
{
  vec4 color = vec4 (1.0, 0.0, 0.8, 1.0); /* pink */

  for(;;)
    {
      uint type = read_uint (reader);
      switch (type)
      {
        default:
        case GSK_GPU_PATTERN_DONE:
          return color;
        case GSK_GPU_PATTERN_COLOR:
          color = color_pattern (reader);
          break;
        case GSK_GPU_PATTERN_TEXTURE:
          color = texture_pattern (reader, pos);
          break;
        case GSK_GPU_PATTERN_GLYPHS:
          color = glyphs_pattern (reader, pos);
          break;
        case GSK_GPU_PATTERN_COLOR_MATRIX:
          color_matrix_pattern (reader, color, pos);
          break;
        case GSK_GPU_PATTERN_OPACITY:
          opacity_pattern (reader, color, pos);
          break;
        case GSK_GPU_PATTERN_LINEAR_GRADIENT:
          color = linear_gradient_pattern (reader, pos, false);
          break;
        case GSK_GPU_PATTERN_REPEATING_LINEAR_GRADIENT:
          color = linear_gradient_pattern (reader, pos, true);
          break;
        case GSK_GPU_PATTERN_RADIAL_GRADIENT:
          color = radial_gradient_pattern (reader, pos, false);
          break;
        case GSK_GPU_PATTERN_REPEATING_RADIAL_GRADIENT:
          color = radial_gradient_pattern (reader, pos, true);
          break;
        case GSK_GPU_PATTERN_CONIC_GRADIENT:
          color = conic_gradient_pattern (reader, pos);
          break;
        case GSK_GPU_PATTERN_CLIP:
          clip_pattern (reader, color, pos);
          break;
        case GSK_GPU_PATTERN_REPEAT_PUSH:
          repeat_push_pattern (reader, pos);
          break;
        case GSK_GPU_PATTERN_POSITION_POP:
          position_pop_pattern (reader, pos);
          break;
        case GSK_GPU_PATTERN_PUSH_COLOR:
          stack_push (color);
          color = vec4 (0.0);
          break;
        case GSK_GPU_PATTERN_POP_CROSS_FADE:
          cross_fade_pattern (reader, color);
          break;
        case GSK_GPU_PATTERN_POP_MASK_ALPHA:
          mask_alpha_pattern (reader, color);
          break;
        case GSK_GPU_PATTERN_POP_MASK_INVERTED_ALPHA:
          mask_inverted_alpha_pattern (reader, color);
          break;
        case GSK_GPU_PATTERN_POP_MASK_LUMINANCE:
          mask_luminance_pattern (reader, color);
          break;
        case GSK_GPU_PATTERN_POP_MASK_INVERTED_LUMINANCE:
          mask_inverted_luminance_pattern (reader, color);
          break;
      }
    }
}

#endif

#endif