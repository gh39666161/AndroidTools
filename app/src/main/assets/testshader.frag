#version 320 es
#define fma(A, B, C) ((A) * (B) + (C))
#define UE_MRT_PLS 1


#ifdef UE_MRT_FRAMEBUFFER_FETCH
#define HLSLCC_DX11ClipSpace 1 


	#extension GL_EXT_shader_framebuffer_fetch : enable
	#define FBF_STORAGE_QUALIFIER inout
#elif defined(GL_ARM_shader_framebuffer_fetch)
	#extension GL_ARM_shader_framebuffer_fetch : enable
#endif
#ifdef GL_ARM_shader_framebuffer_fetch_depth_stencil
	#extension GL_ARM_shader_framebuffer_fetch_depth_stencil : enable
#endif
// end extensions

#ifdef UE_MRT_FRAMEBUFFER_FETCH
	#define _Globals_ARM_shader_framebuffer_fetch 0u
	#define FRAME_BUFFERFETCH_STORAGE_QUALIFIER inout
	#define _Globals_gl_FragColor out_var_SV_Target0
	#define _Globals_gl_LastFragColorARM vec4(0.0, 0.0, 0.0, 0.0)
#elif defined(GL_ARM_shader_framebuffer_fetch)
	#define _Globals_ARM_shader_framebuffer_fetch 1u
	#define FRAME_BUFFERFETCH_STORAGE_QUALIFIER out
	#define _Globals_gl_FragColor vec4(0.0, 0.0, 0.0, 0.0)
	#define _Globals_gl_LastFragColorARM gl_LastFragColorARM
#else
	#define FRAME_BUFFERFETCH_STORAGE_QUALIFIER out
	#define _Globals_ARM_shader_framebuffer_fetch 0u
	#define _Globals_gl_FragColor vec4(0.0, 0.0, 0.0, 0.0)
	#define _Globals_gl_LastFragColorARM vec4(0.0, 0.0, 0.0, 0.0)
#endif
#ifdef GL_ARM_shader_framebuffer_fetch_depth_stencil
	#define _Globals_ARM_shader_framebuffer_fetch_depth_stencil 1u
#else
	#define _Globals_ARM_shader_framebuffer_fetch_depth_stencil 0u
#endif

#if defined(GL_EXT_control_flow_attributes)
#define out_var_SV_Target0 out_Target0
#define in_var_TEXCOORD8 in_TEXCOORD8
#define in_var_PARTICLE_LOCAL_TO_WORLD in_PARTICLE_LOCAL_TO_WORLD
#define in_var_PARTICLE_VELOCITY in_PARTICLE_VELOCITY
#define in_var_COLOR1 in_COLOR1
#define in_var_COLOR0 in_COLOR0
#define in_var_TEXCOORD0 in_TEXCOORD0
#define in_var_TEXCOORD11 in_TEXCOORD11
#define in_var_TEXCOORD10 in_TEXCOORD10
#extension GL_EXT_control_flow_attributes : require
#define SPIRV_CROSS_FLATTEN [[flatten]]
#define SPIRV_CROSS_BRANCH [[dont_flatten]]
#define SPIRV_CROSS_UNROLL [[unroll]]
#define SPIRV_CROSS_LOOP [[dont_unroll]]
#else
#define SPIRV_CROSS_FLATTEN
#define SPIRV_CROSS_BRANCH
#define SPIRV_CROSS_UNROLL
#define SPIRV_CROSS_LOOP
#endif
precision mediump float;
precision highp int;

float _182;



#define View_PreExposure (pc0_h[8].x)
#define View_RelativePreViewTranslationTO (pc0_h[7].xyz)
#define View_RelativeWorldCameraOriginTO (pc0_h[6].xyz)
#define View_ViewTilePosition (pc0_h[5].xyz)
#define View_InvDeviceZToWorldZTransform (pc0_h[4])
#define View_ViewToClip (mat4(pc0_h[0 + 0].xyzw,pc0_h[0 + 1].xyzw,pc0_h[0 + 2].xyzw,pc0_h[0 + 3].xyzw))
uniform highp vec4 pc0_h[9];


#define Primitive_LocalToRelativeWorld (mat4(pc1_h[0 + 0].xyzw,pc1_h[0 + 1].xyzw,pc1_h[0 + 2].xyzw,pc1_h[0 + 3].xyzw))
uniform highp vec4 pc1_h[4];


#define Material_PreshaderBuffer_10 (pc2_h[10])
#define Material_PreshaderBuffer_9 (pc2_h[9])
#define Material_PreshaderBuffer_8 (pc2_h[8])
#define Material_PreshaderBuffer_7 (pc2_h[7])
#define Material_PreshaderBuffer_6 (pc2_h[6])
#define Material_PreshaderBuffer_5 (pc2_h[5])
#define Material_PreshaderBuffer_4 (pc2_h[4])
#define Material_PreshaderBuffer_3 (pc2_h[3])
#define Material_PreshaderBuffer_2 (pc2_h[2])
#define Material_PreshaderBuffer_1 (pc2_h[1])
#define Material_PreshaderBuffer_0 (pc2_h[0])
uniform highp vec4 pc2_h[11];


layout(location = 0) in vec4 in_var_TEXCOORD10;
layout(location = 1) in vec4 in_var_TEXCOORD11;
layout(location = 2) in highp vec4 in_var_TEXCOORD0[3];
layout(location = 5) in highp vec4 in_var_COLOR0;
layout(location = 6) flat in highp vec4 in_var_COLOR1;
layout(location = 7) flat in highp vec4 in_var_PARTICLE_VELOCITY;
layout(location = 8) flat in highp vec4 in_var_PARTICLE_LOCAL_TO_WORLD[3];
layout(location = 11) in highp vec4 in_var_TEXCOORD8;
layout(location = 0) out vec4 out_var_SV_Target0;


#if !defined(GL_ARM_shader_framebuffer_fetch_depth_stencil) && defined(GL_EXT_shader_framebuffer_fetch)
layout(location = 1) inout highp vec4 out_var_SV_Target1;
#endif
float GLFetchDepthBuffer()
{
	#if defined(GL_ARM_shader_framebuffer_fetch_depth_stencil)
	return gl_LastFragDepthARM;
	#elif defined(GL_EXT_shader_framebuffer_fetch)
	return out_var_SV_Target1.x;
	#else
	return 0.0f;
	#endif
}
void main()
{
    highp vec2 _187[5] = vec2[](vec2(0.0), vec2(0.0), vec2(0.0), vec2(0.0), vec2(0.0));
    int _207;
    _207 = 0;
    int _210;
    int _211;
    SPIRV_CROSS_UNROLL
    for (;;)
    {
        _210 = _207 * 2;
        _211 = _210 + 1;
        if (_211 < 5)
        {
            _187[_210] = in_var_TEXCOORD0[_207].xy;
            _187[_211] = in_var_TEXCOORD0[_207].wz;
            _207++;
            continue;
        }
        else
        {
            break;
        }
    }
    _187[4] = in_var_TEXCOORD0[2].xy;
    highp mat4 _225 = transpose(mat4(in_var_PARTICLE_LOCAL_TO_WORLD[0], in_var_PARTICLE_LOCAL_TO_WORLD[1], in_var_PARTICLE_LOCAL_TO_WORLD[2], vec4(0.0, 0.0, 0.0, 1.0)));
    highp vec4 _234 = vec4(_182, _182, gl_FragCoord.z, 1.0) * (1.0 / gl_FragCoord.w);
    highp vec3 _236 = in_var_TEXCOORD8.xyz - View_RelativePreViewTranslationTO;
    vec3 _39 = (Material_PreshaderBuffer_0.xyz * in_var_COLOR1.xyz) * vec3(Material_PreshaderBuffer_0.w);
    uint _278 = floatBitsToUint(Material_PreshaderBuffer_2.w);
    vec3 _110 = mix(mat3(_225[0].xyz, _225[1].xyz, _225[2].xyz) * Material_PreshaderBuffer_2.xyz, mat3(vec4(Primitive_LocalToRelativeWorld[0].xyz, 0.0).xyz, vec4(Primitive_LocalToRelativeWorld[1].xyz, 0.0).xyz, vec4(Primitive_LocalToRelativeWorld[2].xyz, 0.0).xyz) * Material_PreshaderBuffer_2.xyz, bvec3(int(((_278 >> 0u) & 1u) != 0u) == 0));
    float _41 = dot(_110, _110);
    vec3 _42 = normalize(_110);
    bool _45 = abs(_41 - 1.013278961181640625e-06) > 1.0013580322265625e-05;
    bool _46 = _41 >= 1.013278961181640625e-06;
    highp vec3 _294 = (View_ViewTilePosition - View_ViewTilePosition) * 2097152.0;
    vec3 _50 = vec4(_45 ? (_46 ? _42.x : 0.0) : 0.0, _45 ? (_46 ? _42.y : 0.0) : 0.0, _45 ? (_46 ? _42.z : 0.0) : 0.0, _182).xyz;
    highp vec3 _299 = _294 + (fma(-_50, vec3(dot(_50, _294 + (View_RelativeWorldCameraOriginTO - _236))), View_RelativeWorldCameraOriginTO) - _236);
    highp float _300 = dot(_299, _299);
    highp vec3 _301 = normalize(_299);
    bool _55 = abs(_300 - 1.013278961181640625e-06) > 1.0013580322265625e-05;
    bool _56 = _300 >= 1.013278961181640625e-06;
    float _62 = abs(dot(mat3(in_var_TEXCOORD10.xyz, cross(in_var_TEXCOORD11.xyz, in_var_TEXCOORD10.xyz) * in_var_TEXCOORD11.w, in_var_TEXCOORD11.xyz) * vec3(0.0, 0.0, 1.0), vec4(_55 ? (_56 ? _301.x : 0.0) : 0.0, _55 ? (_56 ? _301.y : 0.0) : 0.0, _55 ? (_56 ? _301.z : 1.0) : 1.0, _182).xyz));
    float _63 = 1.0 - _62;
    mediump int _97 = int(trunc(Material_PreshaderBuffer_4.y + 0.00100040435791015625));
    highp vec2 _345;
    if (_97 == 0)
    {
        _345 = _187[0];
    }
    else
    {
        highp vec2 _344;
        if (_97 == 1)
        {
            _344 = _187[1];
        }
        else
        {
            highp vec2 _343;
            if (_97 == 2)
            {
                _343 = _187[2];
            }
            else
            {
                _343 = mix(_187[4], _187[3], bvec2(_97 == 3));
            }
            _344 = _343;
        }
        _345 = _344;
    }
    highp float _348 = dot(_345, Material_PreshaderBuffer_4.zw);
    float _69 = fma(abs(((-0.5) + _348) * 2.0), Material_PreshaderBuffer_5.x, _348 * Material_PreshaderBuffer_5.y);
    float _71 = fma(_69, Material_PreshaderBuffer_5.z, (1.0 - _69) * Material_PreshaderBuffer_5.w);
    float _78 = clamp(Material_PreshaderBuffer_1.y * (in_var_COLOR1.w * ((mix(Material_PreshaderBuffer_4.x, 1.0, clamp(fma((_62 <= 2.9802329493122670101001858711243e-08) ? 0.0 : pow(_62, Material_PreshaderBuffer_3.x), Material_PreshaderBuffer_3.y, ((_63 <= 2.9802329493122670101001858711243e-08) ? 0.0 : pow(_63, Material_PreshaderBuffer_3.z)) * Material_PreshaderBuffer_3.w), 0.0, 1.0)) * clamp(mix(((_71 <= 2.9802329493122670101001858711243e-08) ? 0.0 : pow(_71, Material_PreshaderBuffer_6.x)) * Material_PreshaderBuffer_6.y, _71 * Material_PreshaderBuffer_6.z, Material_PreshaderBuffer_6.w), 0.0, 1.0)) * fma(dot(in_var_COLOR0, Material_PreshaderBuffer_7), Material_PreshaderBuffer_8.x, Material_PreshaderBuffer_8.y))), 0.0, 1.0);
    if ((_78 - Material_PreshaderBuffer_8.z) < 0.0)
    {
        discard;
    }
    highp vec3 _388 = mix(_39, _39 * vec3(_78), vec3(Material_PreshaderBuffer_8.w));
    vec3 _113;
    do
    {
        if (Material_PreshaderBuffer_9.x <= 0.0)
        {
            _113 = _388;
            break;
        }
        else
        {
            _113 = mix(_388, (((pow(_388, vec3(3.0)) * 3.447265625) - (pow(_388, vec3(2.0)) * 2.787109375)) + (_388 * 1.228515625)) - vec3(0.0055999755859375), vec3(Material_PreshaderBuffer_9.x));
            break;
        }
        break; // unreachable workaround
    } while(false);
    bool _402 = int(((_278 >> 1u) & 1u) != 0u) == 0;
    float _111;
    if (_402)
    {
        _111 = Material_PreshaderBuffer_9.z;
    }
    else
    {
        _111 = Material_PreshaderBuffer_9.y;
    }
    highp float _412 = _111 * Material_PreshaderBuffer_9.w;
    float _81 = max(_402 ? _412 : mix(_412, Material_PreshaderBuffer_10.x, _78), 1.0013580322265625e-05);
    highp float _439;
    do
    {
        SPIRV_CROSS_FLATTEN
        if (View_ViewToClip[3u].w < 1.0)
        {
            _439 = _234.w;
            break;
        }
        else
        {
            highp float _424 = _234.z;
            _439 = fma(_424, View_InvDeviceZToWorldZTransform.x, View_InvDeviceZToWorldZTransform.y) + (1.0 / fma(_424, View_InvDeviceZToWorldZTransform.z, -View_InvDeviceZToWorldZTransform.w));
            break;
        }
        break; // unreachable workaround
    } while(false);
    highp float _447 = (int(1u != 0u) == 0) ? 0.0 : GLFetchDepthBuffer();
    vec4 _29 = vec4(max(_113, vec3(0.0)) * 1.0, clamp(_78 * (clamp(clamp(((fma(_447, View_InvDeviceZToWorldZTransform.x, View_InvDeviceZToWorldZTransform.y) + (1.0 / fma(_447, View_InvDeviceZToWorldZTransform.z, -View_InvDeviceZToWorldZTransform.w))) - min(_439, 65504.0 - _81)) / _81, 0.0, 1.0), 0.0, 1.0) * clamp(_439 * Material_PreshaderBuffer_10.y, 0.0, 1.0)), 0.0, 1.0));
    highp vec3 _465 = _29.xyz * View_PreExposure;
    out_var_SV_Target0 = vec4(_465.x, _465.y, _465.z, _29.w);
    // out_var_SV_Target0 = vec4(_345.x, 0, 0, _447);
}