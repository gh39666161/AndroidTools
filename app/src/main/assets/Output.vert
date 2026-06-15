// ! /Engine/Private/MobileBasePassVertexShader.usf:Main
// Compiled by ShaderConductor
// @Inputs: f4;0:in_var_ATTRIBUTE0,f3;1:in_var_ATTRIBUTE1,f4;2:in_var_ATTRIBUTE2,f4;3:in_var_ATTRIBUTE3,f2;4:in_var_ATTRIBUTE4
// @Outputs: f4;10:out_var_TEXCOORD10,f4;11:out_var_TEXCOORD11,f4;0:out_var_TEXCOORD0,u1;3:out_var_PRIMITIVE_ID,f4;8:out_var_TEXCOORD8,f3;9:out_var_TEXCOORD9
// @UniformBlocks: BatchedPrimitive(1)
// @PackedUB: View(0): View_TranslatedWorldToClip(0,16),View_PreViewTranslationHigh(288,3),View_PreViewTranslationLow(292,3)
// @PackedUBCopies: 0:0-0:h:0:16,0:288-0:h:16:3,0:292-0:h:20:3
#version 320 es
#ifdef GL_ARB_shader_draw_parameters
#define in_var_ATTRIBUTE4 in_ATTRIBUTE4
#define in_var_ATTRIBUTE3 in_ATTRIBUTE3
#define in_var_ATTRIBUTE2 in_ATTRIBUTE2
#define in_var_ATTRIBUTE1 in_ATTRIBUTE1
#define in_var_ATTRIBUTE0 in_ATTRIBUTE0
#extension GL_ARB_shader_draw_parameters : enable
#endif

invariant gl_Position;

mat3 _85;

#define View_PreViewTranslationLow (vc0_h[5].xyz)
#define View_PreViewTranslationHigh (vc0_h[4].xyz)
#define View_TranslatedWorldToClip (mat4(vc0_h[0 + 0].xyzw,vc0_h[0 + 1].xyzw,vc0_h[0 + 2].xyzw,vc0_h[0 + 3].xyzw))
uniform highp vec4 vc0_h[6];


layout(std140) uniform vb1
{
    vec4 BatchedPrimitive_Datavb1[1024];
};



layout(location = 0) in vec4 in_var_ATTRIBUTE0;
layout(location = 1) in mediump vec3 in_var_ATTRIBUTE1;
layout(location = 2) in mediump vec4 in_var_ATTRIBUTE2;
layout(location = 3) in mediump vec4 in_var_ATTRIBUTE3;
layout(location = 4) in vec2 in_var_ATTRIBUTE4;
#ifdef GL_ARB_shader_draw_parameters
#define SPIRV_Cross_BaseInstance gl_BaseInstanceARB
#else
uniform int SPIRV_Cross_BaseInstance;
#endif
layout(location = 0) out mediump vec4 out_var_TEXCOORD10;
layout(location = 1) out mediump vec4 out_var_TEXCOORD11;
layout(location = 2) out vec4 out_var_TEXCOORD0[1];
layout(location = 3) flat out uint out_var_PRIMITIVE_ID;
layout(location = 4) out vec4 out_var_TEXCOORD8;
layout(location = 5) out vec3 out_var_TEXCOORD9;

void main()
{
    uint _100 = uint((gl_InstanceID + SPIRV_Cross_BaseInstance)) * 32u;
    mat4 _114 = transpose(mat4(BatchedPrimitive_Datavb1[_100 + 1u], BatchedPrimitive_Datavb1[_100 + 2u], BatchedPrimitive_Datavb1[_100 + 3u], vec4(0.0, 0.0, 0.0, 1.0)));
    uint _135 = _100 + 4u;
    mediump vec3 _28 = cross(in_var_ATTRIBUTE2.xyz, in_var_ATTRIBUTE1) * in_var_ATTRIBUTE2.w;
    mediump mat3 _46;
    _46[0] = cross(_28, in_var_ATTRIBUTE2.xyz) * in_var_ATTRIBUTE2.w;
    _46[1] = _28;
    _46[2] = in_var_ATTRIBUTE2.xyz;
    vec3 _138 = vec4(_114[0].xyz, 0.0).xyz;
    vec3 _139 = vec4(_114[1].xyz, 0.0).xyz;
    vec3 _140 = vec4(_114[2].xyz, 0.0).xyz;
    mediump mat3 _31 = mat3(_138, _139, _140);
    _31[0] = _138 * BatchedPrimitive_Datavb1[_135].x;
    _31[1] = _139 * BatchedPrimitive_Datavb1[_135].y;
    _31[2] = _140 * BatchedPrimitive_Datavb1[_135].z;
    mediump mat3 _38 = _31 * _46;
    precise vec3 _41 = BatchedPrimitive_Datavb1[_100].xyz - (-View_PreViewTranslationHigh);
    precise vec3 _42 = vec3(0.0) - (-View_PreViewTranslationLow);
    precise vec3 _43 = _41 + _42;
    vec4 _154 = vec4(fma(in_var_ATTRIBUTE0.xxx, _138, fma(in_var_ATTRIBUTE0.yyy, _139, in_var_ATTRIBUTE0.zzz * _140)) + (_43 + vec4(_114[3].xyz, 1.0).xyz), 1.0) * 1.0;
    vec4 _156 = vec4(_154.x, _154.y, _154.z, _154.w);
    precise vec4 _24 = View_TranslatedWorldToClip * _156;
    out_var_TEXCOORD0[0u] = vec4(in_var_ATTRIBUTE4.x, in_var_ATTRIBUTE4.y, vec4(0.0).z, vec4(0.0).w);
    _156.w = _24.w;
    out_var_TEXCOORD10 = vec4(_38[0], 0.0);
    out_var_TEXCOORD11 = vec4(_38[2], in_var_ATTRIBUTE2.w);
    out_var_PRIMITIVE_ID = uint((gl_InstanceID + SPIRV_Cross_BaseInstance));
    out_var_TEXCOORD8 = _156;
    out_var_TEXCOORD9 = _154.xyz;
    gl_Position = _24;
    gl_Position.z = 2.0 * gl_Position.z - gl_Position.w;
    gl_Position.y = -gl_Position.y;
}

