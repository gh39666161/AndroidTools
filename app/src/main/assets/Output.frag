// ! /Engine/Private/MobileBasePassPixelShader.usf:Main
// Compiled by ShaderConductor
// @Inputs: f4;10:in_var_TEXCOORD10,f4;11:in_var_TEXCOORD11,f4;0:in_var_TEXCOORD0,u1;3:in_var_PRIMITIVE_ID,f4;8:in_var_TEXCOORD8,f3;9:in_var_TEXCOORD9
// @Outputs: f4;0:out_Target0,f1;1:out_Target1
// @UniformBlocks: BatchedPrimitive(5)
// @PackedUB: View(0): View_RelativeWorldToClip(16,16),View_ViewToClip(112,16),View_ViewOriginHigh(240,3),View_ViewForward(244,3),View_ViewTilePosition(436,3),View_RelativeWorldCameraOriginTO(440,3),View_RelativePreViewTranslationTO(448,3),View_ViewRectMin(540,4),View_ViewSizeAndInvSize(544,4),View_PreExposure(574,1),View_CullingSign(599,1),View_SkyLightColor(768,4),View_MobileSkyIrradianceEnvironmentMap_0(776,4),View_MobileSkyIrradianceEnvironmentMap_1(780,4),View_MobileSkyIrradianceEnvironmentMap_2(784,4),View_ReflectionCubemapMaxMip(810,1)
// @PackedUB: MobileReflectionCapture(1): MobileReflectionCapture_Params(0,4)
// @PackedUB: MobileBasePass(2): MobileBasePass_Forward_NumLocalLights(84,1),MobileBasePass_Forward_CulledGridSize(88,3),MobileBasePass_Forward_MaxCulledLightsPerCell(91,1),MobileBasePass_Forward_LightGridPixelSizeShift(92,1),MobileBasePass_Forward_LightGridZParams(96,3)
// @PackedUB: MobileDirectionalLight(3): MobileDirectionalLight_DirectionalLightColor(0,4),MobileDirectionalLight_DirectionalLightDirectionAndShadowTransition(4,4),MobileDirectionalLight_DirectionalLightDistanceFadeMADAndSpecularScale(12,4)
// @PackedUB: Material(4): Material_PreshaderBuffer_0(0,4),Material_PreshaderBuffer_1(4,4),Material_PreshaderBuffer_2(8,4),Material_PreshaderBuffer_3(12,4),Material_PreshaderBuffer_4(16,4),Material_PreshaderBuffer_5(20,4),Material_PreshaderBuffer_6(24,4)
// @PackedUBCopies: 0:16-0:h:0:16,0:112-0:h:16:16,0:240-0:h:32:3,0:244-0:m:0:3,0:436-0:h:36:3,0:440-0:h:40:3,0:448-0:h:44:3,0:540-0:m:4:4,0:544-0:h:48:4,0:574-0:h:52:1,0:599-0:h:56:1,0:768-0:h:60:4,0:776-0:h:64:12,0:810-0:m:8:1,1:0-1:h:0:4,2:84-2:u:0:1,2:88-2:i:0:3,2:91-2:u:4:1,2:92-2:u:8:1,2:96-2:h:0:3,3:0-3:m:0:8,3:12-3:m:8:4,4:0-4:h:0:28
// @Samplers: MobileReflectionCapture_Texture(0:1[MobileReflectionCapture_TextureSampler]),Material_Texture2D_0(1:1[Material_Texture2D_0Sampler]),Material_Texture2D_1(2:1[Material_Texture2D_1Sampler]),Material_Texture2D_2(3:1[Material_Texture2D_2Sampler]),Material_Texture2D_3(4:1[Material_Texture2D_3Sampler]),Material_Texture2D_4(5:1[Material_Texture2D_4Sampler])
// @UAVs: MobileBasePass_Forward_CulledLightDataGrid32Bit(0:1),MobileBasePass_Forward_NumCulledLightsGrid(1:1),MobileBasePass_Forward_ForwardLocalLightBuffer(2:1)
#version 320 es
#if defined(GL_EXT_control_flow_attributes)
#define out_var_SV_Target1 out_Target1
#define out_var_SV_Target0 out_Target0
#define in_var_TEXCOORD9 in_TEXCOORD9
#define in_var_TEXCOORD8 in_TEXCOORD8
#define in_var_PRIMITIVE_ID in_PRIMITIVE_ID
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

mat4 _359;
vec4 _360;
vec3 _361;

#define View_ReflectionCubemapMaxMip (pc0_m[2].x)
#define View_MobileSkyIrradianceEnvironmentMap_2 (pc0_h[18])
#define View_MobileSkyIrradianceEnvironmentMap_1 (pc0_h[17])
#define View_MobileSkyIrradianceEnvironmentMap_0 (pc0_h[16])
#define View_SkyLightColor (pc0_h[15])
#define View_CullingSign (pc0_h[14].x)
#define View_PreExposure (pc0_h[13].x)
#define View_ViewSizeAndInvSize (pc0_h[12])
#define View_ViewRectMin (pc0_m[1])
#define View_RelativePreViewTranslationTO (pc0_h[11].xyz)
#define View_RelativeWorldCameraOriginTO (pc0_h[10].xyz)
#define View_ViewTilePosition (pc0_h[9].xyz)
#define View_ViewForward (pc0_m[0].xyz)
#define View_ViewOriginHigh (pc0_h[8].xyz)
#define View_ViewToClip (mat4(pc0_h[4 + 0].xyzw,pc0_h[4 + 1].xyzw,pc0_h[4 + 2].xyzw,pc0_h[4 + 3].xyzw))
#define View_RelativeWorldToClip (mat4(pc0_h[0 + 0].xyzw,pc0_h[0 + 1].xyzw,pc0_h[0 + 2].xyzw,pc0_h[0 + 3].xyzw))
#define SPIRV_Cross_CombinedMobileReflectionCapture_TextureMobileReflectionCapture_TextureSampler ps0
#define SPIRV_Cross_CombinedMaterial_Texture2D_0Material_Texture2D_0Sampler ps1
#define SPIRV_Cross_CombinedMaterial_Texture2D_1Material_Texture2D_1Sampler ps2
#define SPIRV_Cross_CombinedMaterial_Texture2D_2Material_Texture2D_2Sampler ps3
#define SPIRV_Cross_CombinedMaterial_Texture2D_3Material_Texture2D_3Sampler ps4
#define SPIRV_Cross_CombinedMaterial_Texture2D_4Material_Texture2D_4Sampler ps5
uniform mediump vec4 pc0_m[3];
uniform highp vec4 pc0_h[19];


#define MobileReflectionCapture_Params (pc1_h[0])
uniform highp vec4 pc1_h[1];


layout(std140) uniform pb5
{
    highp vec4 BatchedPrimitive_Datapb5[1024];
};



#define MobileBasePass_Forward_LightGridZParams (pc2_h[0].xyz)
#define MobileBasePass_Forward_LightGridPixelSizeShift (pc2_u[2].x)
#define MobileBasePass_Forward_MaxCulledLightsPerCell (pc2_u[1].x)
#define MobileBasePass_Forward_CulledGridSize (pc2_i[0].xyz)
#define MobileBasePass_Forward_NumLocalLights (pc2_u[0].x)
uniform highp vec4 pc2_h[1];
uniform ivec4 pc2_i[1];
uniform uvec4 pc2_u[3];


layout(binding = 2, std430) readonly buffer pi2_VAR
{
    highp vec4 pi2[];
};

layout(binding = 1, std430) readonly buffer pi1_VAR
{
    uint pi1[];
};

layout(binding = 0, std430) readonly buffer pi0_VAR
{
    uint pi0[];
};

#define MobileDirectionalLight_DirectionalLightDistanceFadeMADAndSpecularScale (pc3_m[2])
#define MobileDirectionalLight_DirectionalLightDirectionAndShadowTransition (pc3_m[1])
#define MobileDirectionalLight_DirectionalLightColor (pc3_m[0])
uniform mediump vec4 pc3_m[3];


#define Material_PreshaderBuffer_6 (pc4_h[6])
#define Material_PreshaderBuffer_5 (pc4_h[5])
#define Material_PreshaderBuffer_4 (pc4_h[4])
#define Material_PreshaderBuffer_3 (pc4_h[3])
#define Material_PreshaderBuffer_2 (pc4_h[2])
#define Material_PreshaderBuffer_1 (pc4_h[1])
#define Material_PreshaderBuffer_0 (pc4_h[0])
uniform highp vec4 pc4_h[7];


uniform highp sampler2D SPIRV_Cross_CombinedMaterial_Texture2D_0Material_Texture2D_0Sampler;
uniform highp sampler2D SPIRV_Cross_CombinedMaterial_Texture2D_1Material_Texture2D_1Sampler;
uniform highp sampler2D SPIRV_Cross_CombinedMaterial_Texture2D_2Material_Texture2D_2Sampler;
uniform highp sampler2D SPIRV_Cross_CombinedMaterial_Texture2D_3Material_Texture2D_3Sampler;
uniform highp sampler2D SPIRV_Cross_CombinedMaterial_Texture2D_4Material_Texture2D_4Sampler;
uniform highp samplerCube SPIRV_Cross_CombinedMobileReflectionCapture_TextureMobileReflectionCapture_TextureSampler;

layout(location = 0) in vec4 in_var_TEXCOORD10;
layout(location = 1) in vec4 in_var_TEXCOORD11;
layout(location = 2) in highp vec4 in_var_TEXCOORD0[1];
layout(location = 3) flat in uint in_var_PRIMITIVE_ID;
layout(location = 4) in highp vec4 in_var_TEXCOORD8;
layout(location = 5) in highp vec3 in_var_TEXCOORD9;
layout(location = 0) out vec4 out_var_SV_Target0;
layout(location = 1) out highp float out_var_SV_Target1;

void main()
{
    uint _401 = in_var_PRIMITIVE_ID * 32u;
    precise highp vec3 _74 = BatchedPrimitive_Datapb5[_401].xyz / vec3(2097152.0);
    highp vec3 _421 = roundEven(_74);
    precise highp vec3 _76 = fma(_421, vec3(-2097152.0), BatchedPrimitive_Datapb5[_401].xyz) + vec3(0.0);
    highp mat4 _434 = transpose(mat4(BatchedPrimitive_Datapb5[_401 + 12u], BatchedPrimitive_Datapb5[_401 + 13u], BatchedPrimitive_Datapb5[_401 + 14u], vec4(0.0, 0.0, 0.0, 1.0)));
    bool _436 = View_ViewToClip[3].w >= 1.0;
    vec3 _72 = -View_ViewForward;
    highp vec3 _438 = normalize(-in_var_TEXCOORD8.xyz);
    highp vec3 _448 = vec3(_436 ? _72.x : _438.x, _436 ? _72.y : _438.y, _436 ? _72.z : _438.z);
    uint _450 = floatBitsToUint(BatchedPrimitive_Datapb5[_401].w);
    highp vec4 _460 = texture(SPIRV_Cross_CombinedMaterial_Texture2D_0Material_Texture2D_0Sampler, vec2(in_var_TEXCOORD0[0].x, in_var_TEXCOORD0[0].y));
    vec2 _147 = fma(_460.xw, vec2(2.0), vec2(-1.0));
    vec3 _78 = normalize(mat3(in_var_TEXCOORD10.xyz, cross(in_var_TEXCOORD11.xyz, in_var_TEXCOORD10.xyz) * in_var_TEXCOORD11.w, in_var_TEXCOORD11.xyz) * vec4(_147, sqrt(clamp(1.0 - dot(_147, _147), 0.0, 1.0)), 1.0).xyz) * ((View_CullingSign * (((_450 & 64u) != 0u) ? (-1.0) : 1.0)) * float(gl_FrontFacing ? (-1) : 1));
    highp float _465 = dot(_78, _448);
    highp vec3 _468 = (-_448) + ((_78 * _465) * 2.0);
    highp vec4 _472 = texture(SPIRV_Cross_CombinedMaterial_Texture2D_1Material_Texture2D_1Sampler, vec2(in_var_TEXCOORD0[0].x, in_var_TEXCOORD0[0].y));
    float _84 = max(abs(1.0 - max(0.0, _465)), 0.00010001659393310546875);
    bool _478 = _84 <= 2.9802329493122670101001858711243e-08;
    highp vec3 _492 = (fma(BatchedPrimitive_Datapb5[_401].xyz, vec3(-4.76837158203125e-07), View_ViewTilePosition) * 2097152.0) + (in_var_TEXCOORD9 - View_RelativePreViewTranslationTO);
    highp vec3 _517 = (vec4(transpose(mat4(BatchedPrimitive_Datapb5[_401 + 1u], BatchedPrimitive_Datapb5[_401 + 2u], BatchedPrimitive_Datapb5[_401 + 3u], vec4(0.0, 0.0, 0.0, 1.0)))[3].xyz, 1.0).xyz + _76).xyz;
    highp vec2 _524 = gl_FragCoord.xy - View_ViewRectMin.xy;
    highp mat4 _533;
    _533[0] = vec4(1.0, 0.0, 0.0, 0.0);
    _533[1] = vec4(0.0, 1.0, 0.0, 0.0);
    _533[2] = vec4(0.0, 0.0, 1.0, 0.0);
    _533[3] = vec4(-View_ViewOriginHigh, 1.0);
    highp vec4 _542 = (View_RelativeWorldToClip * _533) * vec4((_421 * 2097152.0) + _517, 1.0);
    vec2 _101 = _542.xy / vec2(_542.w);
    vec2 _116 = (vec2(length(((View_ViewTilePosition - _421) * 2097152.0) + (View_RelativeWorldCameraOriginTO - _517))) * fma(_524, View_ViewSizeAndInvSize.zw, -(fma(vec2(fma(_101.x, 0.5, 0.5), fma(_101.y, -0.5, 0.5)), View_ViewSizeAndInvSize.xy, vec2(0.5)) * View_ViewSizeAndInvSize.zw))) * vec2(View_ViewSizeAndInvSize.x / View_ViewSizeAndInvSize.y, 1.0);
    highp vec4 _571 = texture(SPIRV_Cross_CombinedMaterial_Texture2D_2Material_Texture2D_2Sampler, mix(vec2((((fma(_492.zzz, _434[2].xyz, fma(_492.xxx, _434[0].xyz, _492.yyy * _434[1].xyz)) + _434[3].xyz) - BatchedPrimitive_Datapb5[_401 + 10u].xyz) / (BatchedPrimitive_Datapb5[_401 + 11u].xyz - BatchedPrimitive_Datapb5[_401 + 10u].xyz)).xz), vec2(max(texture(SPIRV_Cross_CombinedMaterial_Texture2D_2Material_Texture2D_2Sampler, fma(_116, Material_PreshaderBuffer_2.zw, vec2(0.0025005340576171875))).x, max(max(texture(SPIRV_Cross_CombinedMaterial_Texture2D_2Material_Texture2D_2Sampler, fma(_116, Material_PreshaderBuffer_3.xy, vec2(0.005001068115234375))).x, texture(SPIRV_Cross_CombinedMaterial_Texture2D_2Material_Texture2D_2Sampler, fma(_116, Material_PreshaderBuffer_3.zw, vec2(0.00749969482421875))).x), texture(SPIRV_Cross_CombinedMaterial_Texture2D_3Material_Texture2D_3Sampler, fma(_116, Material_PreshaderBuffer_4.xy, vec2(0.00125026702880859375))).x))), vec2(Material_PreshaderBuffer_4.z)));
    float _129 = _571.y;
    float _130 = ceil(_129 - Material_PreshaderBuffer_4.w);
    highp vec4 _586 = texture(SPIRV_Cross_CombinedMaterial_Texture2D_4Material_Texture2D_4Sampler, vec2(in_var_TEXCOORD0[0].x, in_var_TEXCOORD0[0].y));
    if (fma(min(_586.w, 1.0 - clamp(fma(-length(((View_ViewTilePosition - View_ViewTilePosition) * 2097152.0) + ((in_var_TEXCOORD8.xyz - View_RelativePreViewTranslationTO) - View_RelativeWorldCameraOriginTO)), Material_PreshaderBuffer_6.x, 1.0) * uintBitsToFloat(0x7f800000u /* inf */), 0.0, 1.0)), _130, -0.333251953125) < 0.0)
    {
        discard;
    }
    float _158 = clamp(_472.x, 0.0, 1.0);
    vec3 _54 = max(_586.xyz, vec3(0.0));
    float _159 = clamp(_472.z, 0.0, 1.0);
    highp float _597 = max(0.015625, clamp(_472.y, 0.0, 1.0));
    vec3 _162 = mix(vec3(0.01599609293043613433837890625), _54, vec3(_159));
    vec3 _56 = _54 - (_54 * _159);
    highp vec4 _602 = vec4(_78, 1.0);
    highp vec3 _606;
    _606.x = dot(View_MobileSkyIrradianceEnvironmentMap_0, _602);
    _606.y = dot(View_MobileSkyIrradianceEnvironmentMap_1, _602);
    _606.z = dot(View_MobileSkyIrradianceEnvironmentMap_2, _602);
    highp float _622 = max(0.0, dot(_78, normalize(_448 + MobileDirectionalLight_DirectionalLightDirectionAndShadowTransition.xyz)));
    vec3 _176 = _56 * 0.318407952785491943359375;
    highp vec3 _623 = vec3(1.0) * max(0.0, dot(_78, MobileDirectionalLight_DirectionalLightDirectionAndShadowTransition.xyz));
    float _177 = _597 * _597;
    highp float _627 = _622 * _177;
    highp float _629 = _177 / fma(_627, _627, fma(-_622, _622, 1.0));
    float _181 = fma(_597, 0.25, 0.25);
    vec4 _191 = (vec4(-1.0, -0.027496337890625, -0.57177734375, 0.022003173828125) * _597) + vec4(1.0, 0.042510986328125, 1.0400390625, -0.040008544921875);
    float _192 = _191.x;
    vec2 _201 = (vec2(-1.0400390625, 1.0400390625) * fma(min(_192 * _192, exp2((-9.28125) * clamp(abs(dot(_78, _448)) + 1.0013580322265625e-05, 0.0, 1.0))), _192, _191.y)) + _191.zw;
    highp vec3 _632 = (_162 * _201.x) + vec3(clamp(50.0 * _162.y, 0.0, 1.0) * _201.y);
    uvec2 _665 = uvec2(uint(_524.x), uint(_524.y)) >> (uvec2(MobileBasePass_Forward_LightGridPixelSizeShift) & uvec2(31u));
    vec3 _223;
    if (MobileReflectionCapture_Params.y > 0.0)
    {
        _223 = textureLod(SPIRV_Cross_CombinedMobileReflectionCapture_TextureMobileReflectionCapture_TextureSampler, _468, (MobileReflectionCapture_Params.y - 1.0) - fma(-1.2001953125, log2(max(_597, 0.00100040435791015625)), 1.0)).xyz * View_SkyLightColor.xyz;
    }
    else
    {
        _223 = textureLod(SPIRV_Cross_CombinedMobileReflectionCapture_TextureMobileReflectionCapture_TextureSampler, _468, (View_ReflectionCubemapMaxMip - 1.0) - fma(-1.2001953125, log2(max(_597, 0.00100040435791015625)), 1.0)).xyz * MobileReflectionCapture_Params.w;
    }
    uint _695 = ((((min(uint(max(0.0, log2(fma(1.0 / gl_FragCoord.w, MobileBasePass_Forward_LightGridZParams.x, MobileBasePass_Forward_LightGridZParams.y)) * MobileBasePass_Forward_LightGridZParams.z)), uint(MobileBasePass_Forward_CulledGridSize.z - 1)) * uint(MobileBasePass_Forward_CulledGridSize.y)) + _665.y) * uint(MobileBasePass_Forward_CulledGridSize.x)) + _665.x) * 2u;
    uint _701 = _695 + 1u;
    uint _716 = (uint((_450 & 2048u) != 0u) | (uint((_450 & 4096u) != 0u) << 1u)) | (uint((_450 & 8192u) != 0u) << 2u);
    uint _719 = min(min(pi1[_695], MobileBasePass_Forward_NumLocalLights), MobileBasePass_Forward_MaxCulledLightsPerCell);
    highp vec3 _721;
    _721 = fma(_223 * _158, _632, fma((max(vec3(0.0), _606) * View_SkyLightColor.xyz) * _56, vec3(_158), fma(((_176 * _623) * 1.0) + ((_623 * (_632 * (min((0.318407952785491943359375 * _629) * _629, 2048.0) * _181))) * MobileDirectionalLight_DirectionalLightDistanceFadeMADAndSpecularScale.z), MobileDirectionalLight_DirectionalLightColor.xyz * 1.0, vec3(0.0))));
    highp vec3 _722;
    SPIRV_CROSS_LOOP
    for (uint _724 = 0u; _724 < _719; _721 = _722, _724++)
    {
        uint _732 = pi0[pi1[_701] + _724] * 6u;
        uint _735 = _732 + 1u;
        uint _738 = _732 + 2u;
        uint _741 = _732 + 3u;
        highp float _745 = pi2[_732].w * pi2[_732].w;
        highp vec3 _747 = in_var_TEXCOORD8.xyz - pi2[_732].xyz;
        if (!((dot(_747, _747) * _745) <= 1.0))
        {
            _722 = _721;
            continue;
        }
        uint _755 = floatBitsToUint(pi2[_738].w);
        if ((((_755 >> 8u) & 7u) & _716) == 0u)
        {
            _722 = _721;
            continue;
        }
        uint _763 = (_755 >> 16u) & 3u;
        uint _765 = floatBitsToUint(pi2[_735].y);
        highp vec3 _783 = pi2[_732].xyz - in_var_TEXCOORD8.xyz;
        highp float _784 = dot(_783, _783);
        highp vec3 _786 = _783 * inversesqrt(_784);
        highp float _803;
        if (pi2[_735].w == 0.0)
        {
            highp float _795 = _784 * _745;
            highp float _798 = clamp(fma(-_795, _795, 1.0), 0.0, 1.0);
            _803 = (_798 * _798) * (1.0 / (_784 + 1.0));
        }
        else
        {
            highp vec3 _790 = _783 * pi2[_732].w;
            _803 = pow(1.0 - clamp(dot(_790, _790), 0.0, 1.0), pi2[_735].w);
        }
        highp float _814;
        if ((_763 == 2u) || (_763 == 3u))
        {
            highp float _811 = clamp((dot(_786, pi2[_738].xyz) - pi2[_741].x) * pi2[_741].y, 0.0, 1.0);
            _814 = _803 * (_811 * _811);
        }
        else
        {
            _814 = _803;
        }
        highp vec3 _836;
        SPIRV_CROSS_BRANCH
        if (_814 > 0.0)
        {
            highp float _822 = max(0.0, dot(_78, normalize(_448 + _786)));
            highp vec3 _823 = vec3(1.0) * max(0.0, dot(_78, _786));
            highp float _827 = _822 * _177;
            highp float _829 = _177 / fma(_827, _827, fma(-_822, _822, 1.0));
            _836 = fma(((_176 * _823) * 1.0) + ((_823 * (_632 * (min((0.318407952785491943359375 * _829) * _829, 2048.0) * _181))) * 1.0), ((vec3(float((_765 >> 0u) & 1023u), float((_765 >> 10u) & 1023u), float((_765 >> 20u) & 1023u)) * pi2[_735].x) * _814) * 1.0, vec3(0.0));
        }
        else
        {
            _836 = vec3(0.0);
        }
        _722 = _721 + _836;
    }
    vec3 _59 = (_721 + max(max(fma(vec3(_478 ? 0.0 : pow(_84, 5.0)) * vec3(1.5, 1.0, 0.0), vec3(Material_PreshaderBuffer_2.x), fma(Material_PreshaderBuffer_0.xyz, vec3(_472.w), (vec3(_478 ? 0.0 : pow(_84, Material_PreshaderBuffer_0.w)) * Material_PreshaderBuffer_1.xyz) * vec3(Material_PreshaderBuffer_1.w))), vec3(_130 - ceil(fma(_129, Material_PreshaderBuffer_5.x, -Material_PreshaderBuffer_4.w))) * Material_PreshaderBuffer_5.yzw), vec3(0.0))) * 1.0;
    vec4 _60 = vec4(_59.x, _59.y, _59.z, _360.w);
    _60.w = 0.0;
    highp vec3 _841 = min((_60.xyz * View_PreExposure).xyz, vec3(32512.0, 32512.0, 32256.0));
    out_var_SV_Target0 = vec4(_841.x, _841.y, _841.z, _60.w);
    out_var_SV_Target1 = gl_FragCoord.z;
}

