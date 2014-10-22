/**
 * MojoShader; generate shader programs from bytecode of compiled
 *  Direct3D shaders.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

#ifndef MOJOSHADER_EFFECTS_H
#define MOJOSHADER_EFFECTS_H

#ifdef MOJOSHADER_EFFECT_SUPPORT

typedef enum MOJOSHADER_renderStateType
{
    /* Note that we are NOT using the actual RS values from D3D here.
     * For some reason, in the binary data, it's 0-based.
     * Even worse, it doesn't even seem to be in order.
     * Here is the list of changes compared to the real D3DRS enum:
     * - All of the RS_WRAP values are in a row, not separate!
     *
     * -flibit
     */
    MOJOSHADER_RS_ZENABLE,
    MOJOSHADER_RS_FILLMODE,
    MOJOSHADER_RS_SHADEMODE,
    MOJOSHADER_RS_ZWRITEENABLE,
    MOJOSHADER_RS_ALPHATESTENABLE,
    MOJOSHADER_RS_LASTPIXEL,
    MOJOSHADER_RS_SRCBLEND,
    MOJOSHADER_RS_DESTBLEND,
    MOJOSHADER_RS_CULLMODE,
    MOJOSHADER_RS_ZFUNC,
    MOJOSHADER_RS_ALPHAREF,
    MOJOSHADER_RS_ALPHAFUNC,
    MOJOSHADER_RS_DITHERENABLE,
    MOJOSHADER_RS_ALPHABLENDENABLE,
    MOJOSHADER_RS_FOGENABLE,
    MOJOSHADER_RS_SPECULARENABLE,
    MOJOSHADER_RS_FOGCOLOR,
    MOJOSHADER_RS_FOGTABLEMODE,
    MOJOSHADER_RS_FOGSTART,
    MOJOSHADER_RS_FOGEND,
    MOJOSHADER_RS_FOGDENSITY,
    MOJOSHADER_RS_RANGEFOGENABLE,
    MOJOSHADER_RS_STENCILENABLE,
    MOJOSHADER_RS_STENCILFAIL,
    MOJOSHADER_RS_STENCILZFAIL,
    MOJOSHADER_RS_STENCILPASS,
    MOJOSHADER_RS_STENCILFUNC,
    MOJOSHADER_RS_STENCILREF,
    MOJOSHADER_RS_STENCILMASK,
    MOJOSHADER_RS_STENCILWRITEMASK,
    MOJOSHADER_RS_TEXTUREFACTOR,
    MOJOSHADER_RS_WRAP0,
    MOJOSHADER_RS_WRAP1,
    MOJOSHADER_RS_WRAP2,
    MOJOSHADER_RS_WRAP3,
    MOJOSHADER_RS_WRAP4,
    MOJOSHADER_RS_WRAP5,
    MOJOSHADER_RS_WRAP6,
    MOJOSHADER_RS_WRAP7,
    MOJOSHADER_RS_WRAP8,
    MOJOSHADER_RS_WRAP9,
    MOJOSHADER_RS_WRAP10,
    MOJOSHADER_RS_WRAP11,
    MOJOSHADER_RS_WRAP12,
    MOJOSHADER_RS_WRAP13,
    MOJOSHADER_RS_WRAP14,
    MOJOSHADER_RS_WRAP15,
    MOJOSHADER_RS_CLIPPING,
    MOJOSHADER_RS_LIGHTING,
    MOJOSHADER_RS_AMBIENT,
    MOJOSHADER_RS_FOGVERTEXMODE,
    MOJOSHADER_RS_COLORVERTEX,
    MOJOSHADER_RS_LOCALVIEWER,
    MOJOSHADER_RS_NORMALIZENORMALS,
    MOJOSHADER_RS_DIFFUSEMATERIALSOURCE,
    MOJOSHADER_RS_SPECULARMATERIALSOURCE,
    MOJOSHADER_RS_AMBIENTMATERIALSOURCE,
    MOJOSHADER_RS_EMISSIVEMATERIALSOURCE,
    MOJOSHADER_RS_VERTEXBLEND,
    MOJOSHADER_RS_CLIPPLANEENABLE,
    MOJOSHADER_RS_POINTSIZE,
    MOJOSHADER_RS_POINTSIZE_MIN,
    MOJOSHADER_RS_POINTSPRITEENABLE,
    MOJOSHADER_RS_POINTSCALEENABLE,
    MOJOSHADER_RS_POINTSCALE_A,
    MOJOSHADER_RS_POINTSCALE_B,
    MOJOSHADER_RS_POINTSCALE_C,
    MOJOSHADER_RS_MULTISAMPLEANTIALIAS,
    MOJOSHADER_RS_MULTISAMPLEMASK,
    MOJOSHADER_RS_PATCHEDGESTYLE,
    MOJOSHADER_RS_DEBUGMONITORTOKEN,
    MOJOSHADER_RS_POINTSIZE_MAX,
    MOJOSHADER_RS_INDEXEDVERTEXBLENDENABLE,
    MOJOSHADER_RS_COLORWRITEENABLE,
    MOJOSHADER_RS_TWEENFACTOR,
    MOJOSHADER_RS_BLENDOP,
    MOJOSHADER_RS_POSITIONDEGREE,
    MOJOSHADER_RS_NORMALDEGREE,
    MOJOSHADER_RS_SCISSORTESTENABLE,
    MOJOSHADER_RS_SLOPESCALEDEPTHBIAS,
    MOJOSHADER_RS_ANTIALIASEDLINEENABLE,
    MOJOSHADER_RS_MINTESSELLATIONLEVEL,
    MOJOSHADER_RS_MAXTESSELLATIONLEVEL,
    MOJOSHADER_RS_ADAPTIVETESS_X,
    MOJOSHADER_RS_ADAPTIVETESS_Y,
    MOJOSHADER_RS_ADAPTIVETESS_Z,
    MOJOSHADER_RS_ADAPTIVETESS_W,
    MOJOSHADER_RS_ENABLEADAPTIVETESSELLATION,
    MOJOSHADER_RS_TWOSIDEDSTENCILMODE,
    MOJOSHADER_RS_CCW_STENCILFAIL,
    MOJOSHADER_RS_CCW_STENCILZFAIL,
    MOJOSHADER_RS_CCW_STENCILPASS,
    MOJOSHADER_RS_CCW_STENCILFUNC,
    MOJOSHADER_RS_COLORWRITEENABLE1,
    MOJOSHADER_RS_COLORWRITEENABLE2,
    MOJOSHADER_RS_COLORWRITEENABLE3,
    MOJOSHADER_RS_BLENDFACTOR,
    MOJOSHADER_RS_SRGBWRITEENABLE,
    MOJOSHADER_RS_DEPTHBIAS,
    MOJOSHADER_RS_SEPARATEALPHABLENDENABLE,
    MOJOSHADER_RS_SRCBLENDALPHA,
    MOJOSHADER_RS_DESTBLENDALPHA,
    MOJOSHADER_RS_BLENDOPALPHA
} MOJOSHADER_renderStateType;

typedef enum MOJOSHADER_zBufferType
{
    MOJOSHADER_ZB_FALSE,
    MOJOSHADER_ZB_TRUE,
    MOJOSHADER_ZB_USEW
} MOJOSHADER_zBufferType;

typedef enum MOJOSHADER_fillMode
{
    MOJOSHADER_FILL_POINT     = 1,
    MOJOSHADER_FILL_WIREFRAME = 2,
    MOJOSHADER_FILL_SOLID     = 3
} MOJOSHADER_fillMode;

typedef enum MOJOSHADER_shadeMode
{
    MOJOSHADER_SHADE_FLAT    = 1,
    MOJOSHADER_SHADE_GOURAUD = 2,
    MOJOSHADER_SHADE_PHONG   = 3,
} MOJOSHADER_shadeMode;

typedef enum MOJOSHADER_blendMode
{
    MOJOSHADER_BLEND_ZERO            = 1,
    MOJOSHADER_BLEND_ONE             = 2,
    MOJOSHADER_BLEND_SRCCOLOR        = 3,
    MOJOSHADER_BLEND_INVSRCCOLOR     = 4,
    MOJOSHADER_BLEND_SRCALPHA        = 5,
    MOJOSHADER_BLEND_INVSRCALPHA     = 6,
    MOJOSHADER_BLEND_DESTALPHA       = 7,
    MOJOSHADER_BLEND_INVDESTALPHA    = 8,
    MOJOSHADER_BLEND_DESTCOLOR       = 9,
    MOJOSHADER_BLEND_INVDESTCOLOR    = 10,
    MOJOSHADER_BLEND_SRCALPHASAT     = 11,
    MOJOSHADER_BLEND_BOTHSRCALPHA    = 12,
    MOJOSHADER_BLEND_BOTHINVSRCALPHA = 13,
    MOJOSHADER_BLEND_BLENDFACTOR     = 14,
    MOJOSHADER_BLEND_INVBLENDFACTOR  = 15,
    MOJOSHADER_BLEND_SRCCOLOR2       = 16,
    MOJOSHADER_BLEND_INVSRCCOLOR2    = 17
} MOJOSHADER_blendMode;

typedef enum MOJOSHADER_cullMode
{
    MOJOSHADER_CULL_NONE = 1,
    MOJOSHADER_CULL_CW   = 2,
    MOJOSHADER_CULL_CCW  = 3
} MOJOSHADER_cullMode;

typedef enum MOJOSHADER_compareFunc
{
    MOJOSHADER_CMP_NEVER        = 1,
    MOJOSHADER_CMP_LESS         = 2,
    MOJOSHADER_CMP_EQUAL        = 3,
    MOJOSHADER_CMP_LESSEQUAL    = 4,
    MOJOSHADER_CMP_GREATER      = 5,
    MOJOSHADER_CMP_NOTEQUAL     = 6,
    MOJOSHADER_CMP_GREATEREQUAL = 7,
    MOJOSHADER_CMP_ALWAYS       = 8
} MOJOSHADER_compareFunc;

typedef enum MOJOSHADER_fogMode
{
    MOJOSHADER_FOG_NONE,
    MOJOSHADER_FOG_EXP,
    MOJOSHADER_FOG_EXP2,
    MOJOSHADER_FOG_LINEAR
} MOJOSHADER_fogMode;

typedef enum MOJOSHADER_stencilOp
{
    MOJOSHADER_STENCILOP_KEEP    = 1,
    MOJOSHADER_STENCILOP_ZERO    = 2,
    MOJOSHADER_STENCILOP_REPLACE = 3,
    MOJOSHADER_STENCILOP_INCRSAT = 4,
    MOJOSHADER_STENCILOP_DECRSAT = 5,
    MOJOSHADER_STENCILOP_INVERT  = 6,
    MOJOSHADER_STENCILOP_INCR    = 7,
    MOJOSHADER_STENCILOP_DECR    = 8
} MOJOSHADER_stencilOp;

typedef enum MOJOSHADER_materialColorSource
{
    MOJOSHADER_MCS_MATERIAL,
    MOJOSHADER_MCS_COLOR1,
    MOJOSHADER_MCS_COLOR2
} MOJOSHADER_materialColorSource;

typedef enum MOJOSHADER_vertexBlendFlags
{
    MOJOSHADER_VBF_DISABLE  = 0,
    MOJOSHADER_VBF_1WEIGHTS = 1,
    MOJOSHADER_VBF_2WEIGHTS = 2,
    MOJOSHADER_VBF_3WEIGHTS = 3,
    MOJOSHADER_VBF_TWEENING = 255,
    MOJOSHADER_VBF_0WEIGHTS = 256,
} MOJOSHADER_vertexBlendFlags;

typedef enum MOJOSHADER_patchedEdgeStyle
{
    MOJOSHADER_PATCHEDGE_DISCRETE,
    MOJOSHADER_PATCHEDGE_CONTINUOUS
} MOJOSHADER_patchedEdgeStyle;

typedef enum MOJOSHADER_debugMonitorTokens
{
    MOJOSHADER_DMT_ENABLE,
    MOJOSHADER_DMT_DISABLE
} MOJOSHADER_debugMonitorTokens;

typedef enum MOJOSHADER_blendOp
{
    MOJOSHADER_BLENDOP_ADD         = 1,
    MOJOSHADER_BLENDOP_SUBTRACT    = 2,
    MOJOSHADER_BLENDOP_REVSUBTRACT = 3,
    MOJOSHADER_BLENDOP_MIN         = 4,
    MOJOSHADER_BLENDOP_MAX         = 5
} MOJOSHADER_blendOp;

typedef enum MOJOSHADER_degreeType
{
    MOJOSHADER_DEGREE_LINEAR    = 1,
    MOJOSHADER_DEGREE_QUADRATIC = 2,
    MOJOSHADER_DEGREE_CUBIC     = 3,
    MOJOSHADER_DEGREE_QUINTIC   = 5
} MOJOSHADER_degreeType;

typedef struct MOJOSHADER_effectState
{
    MOJOSHADER_renderStateType type;
    union
    {
        unsigned int                   value; /* Raw value */
        MOJOSHADER_zBufferType         valueZBT;
        MOJOSHADER_fillMode            valueFiM;
        MOJOSHADER_shadeMode           valueSM;
        MOJOSHADER_blendMode           valueBM;
        MOJOSHADER_cullMode            valueCM;
        MOJOSHADER_compareFunc         valueCF;
        MOJOSHADER_fogMode             valueFoM;
        MOJOSHADER_stencilOp           valueSO;
        MOJOSHADER_materialColorSource valueMCS;
        MOJOSHADER_vertexBlendFlags    valueVBF;
        MOJOSHADER_patchedEdgeStyle    valuePES;
        MOJOSHADER_debugMonitorTokens  valueDMT;
        MOJOSHADER_blendOp             valueBO;
        MOJOSHADER_degreeType          valueDT;
    };
} MOJOSHADER_effectState;

typedef enum MOJOSHADER_samplerStateType
{
    MOJOSHADER_SAMP_ADDRESSU      = 1,
    MOJOSHADER_SAMP_ADDRESSV      = 2,
    MOJOSHADER_SAMP_ADDRESSW      = 3,
    MOJOSHADER_SAMP_BORDERCOLOR   = 4,
    MOJOSHADER_SAMP_MAGFILTER     = 5,
    MOJOSHADER_SAMP_MINFILTER     = 6,
    MOJOSHADER_SAMP_MIPFILTER     = 7,
    MOJOSHADER_SAMP_MIPMAPLODBIAS = 8,
    MOJOSHADER_SAMP_MAXMIPLEVEL   = 9,
    MOJOSHADER_SAMP_MAXANISOTROPY = 10,
    MOJOSHADER_SAMP_SRGBTEXTURE   = 11,
    MOJOSHADER_SAMP_ELEMENTINDEX  = 12,
    MOJOSHADER_SAMP_DMAPOFFSET    = 13
} MOJOSHADER_samplerStateType;

typedef enum MOJOSHADER_textureAddress
{
    MOJOSHADER_TADDRESS_WRAP       = 1,
    MOJOSHADER_TADDRESS_MIRROR     = 2,
    MOJOSHADER_TADDRESS_CLAMP      = 3,
    MOJOSHADER_TADDRESS_BORDER     = 4,
    MOJOSHADER_TADDRESS_MIRRORONCE = 5
} MOJOSHADER_textureAddress;

typedef enum MOJOSHADER_textureFilterType
{
    MOJOSHADER_TEXTUREFILTER_NONE,
    MOJOSHADER_TEXTUREFILTER_POINT,
    MOJOSHADER_TEXTUREFILTER_LINEAR,
    MOJOSHADER_TEXTUREFILTER_ANISOTROPIC,
    MOJOSHADER_TEXTUREFILTER_PYRAMIDALQUAD,
    MOJOSHADER_TEXTUREFILTER_GAUSSIANQUAD,
    MOJOSHADER_TEXTUREFILTER_CONVOLUTIONMONO
} MOJOSHADER_textureFilterType;

typedef struct MOJOSHADER_effectSamplerState
{
    MOJOSHADER_samplerStateType type;
    union
    {
        unsigned int                   valueI;
        float                          valueF;
        MOJOSHADER_textureAddress      valueTA;
        MOJOSHADER_textureFilterType   valueTFT;
    };
} MOJOSHADER_effectSamplerState;

typedef struct MOJOSHADER_effectAnnotation
{
    const char *name;
    const char *semantic;
    unsigned int row_count;
    unsigned int column_count;
    MOJOSHADER_symbolClass anno_class;
    MOJOSHADER_symbolType anno_type;
} MOJOSHADER_effectAnnotation;

typedef struct MOJOSHADER_effectParam MOJOSHADER_effectParam;
struct MOJOSHADER_effectParam
{
    const char *name;
    const char *semantic;
    unsigned int row_count;
    unsigned int column_count;
    MOJOSHADER_symbolClass param_class;
    MOJOSHADER_symbolType param_type;
    union
    {
        unsigned int element_count;
        unsigned int member_count;
    };
    union
    {
        MOJOSHADER_effectParam *elements;
        MOJOSHADER_effectParam *members;
    };
    unsigned int annotation_count;
    MOJOSHADER_effectAnnotation* annotations;
    union
    {
        unsigned int sampler_state_count;
        unsigned int value_count;
    };
    union
    {
        MOJOSHADER_effectSamplerState *sampler_states;
        void *values;
    };
};

typedef struct MOJOSHADER_effectPass
{
    const char *name;
    unsigned int state_count;
    MOJOSHADER_effectState *states;
    unsigned int annotation_count;
    MOJOSHADER_effectAnnotation* annotations;
} MOJOSHADER_effectPass;

typedef struct MOJOSHADER_effectTechnique
{
    const char *name;
    unsigned int pass_count;
    MOJOSHADER_effectPass *passes;
    unsigned int annotation_count;
    MOJOSHADER_effectAnnotation* annotations;
} MOJOSHADER_effectTechnique;

typedef struct MOJOSHADER_effectTexture
{
    unsigned int param;
    const char *name;
} MOJOSHADER_effectTexture;

typedef struct MOJOSHADER_effectShader
{
    unsigned int technique;
    unsigned int pass;
    const MOJOSHADER_parseData *shader;
} MOJOSHADER_effectShader;

/*
 * Structure used to return data from parsing of an effect file...
 */
/* !!! FIXME: most of these ints should be unsigned. */
typedef struct MOJOSHADER_effect
{
    /*
     * The number of elements pointed to by (errors).
     */
    int error_count;

    /*
     * (error_count) elements of data that specify errors that were generated
     *  by parsing this shader.
     * This can be NULL if there were no errors or if (error_count) is zero.
     */
    MOJOSHADER_error *errors;

    /*
     * The name of the profile used to parse the shader. Will be NULL on error.
     */
    const char *profile;

    /*
     * The number of params pointed to by (params).
     */
    int param_count;

    /*
     * (param_count) elements of data that specify parameter bind points for
     *  this effect.
     * This can be NULL on error or if (param_count) is zero.
     */
    MOJOSHADER_effectParam *params;

    /*
     * The number of elements pointed to by (techniques).
     */
    int technique_count;

    /*
     * (technique_count) elements of data that specify techniques used in
     *  this effect. Each technique contains a series of passes, and each pass
     *  specifies state and shaders that affect rendering.
     * This can be NULL on error or if (technique_count) is zero.
     */
    MOJOSHADER_effectTechnique *techniques;

    /*
     * The number of elements pointed to by (textures).
     */
    int texture_count;

    /*
     * (texture_count) elements of data that specify textures used in
     *  this effect.
     * This can be NULL on error or if (texture_count) is zero.
     */
    MOJOSHADER_effectTexture *textures;

    /*
     * The number of elements pointed to by (shaders).
     */
    int shader_count;

    /*
     * (shader_count) elements of data that specify shaders used in
     *  this effect.
     * This can be NULL on error or if (shader_count) is zero.
     */
    MOJOSHADER_effectShader *shaders;

    /*
     * This is the malloc implementation you passed to MOJOSHADER_parseEffect().
     */
    MOJOSHADER_malloc malloc;

    /*
     * This is the free implementation you passed to MOJOSHADER_parseEffect().
     */
    MOJOSHADER_free free;

    /*
     * This is the pointer you passed as opaque data for your allocator.
     */
    void *malloc_data;
} MOJOSHADER_effect;

/* !!! FIXME: document me. */
const MOJOSHADER_effect *MOJOSHADER_parseEffect(const char *profile,
                                                const unsigned char *buf,
                                                const unsigned int _len,
                                                const MOJOSHADER_swizzle *swiz,
                                                const unsigned int swizcount,
                                                const MOJOSHADER_samplerMap *smap,
                                                const unsigned int smapcount,
                                                MOJOSHADER_malloc m,
                                                MOJOSHADER_free f,
                                                void *d);


/* !!! FIXME: document me. */
void MOJOSHADER_freeEffect(const MOJOSHADER_effect *effect);

#endif /* MOJOSHADER_EFFECT_SUPPORT */

#endif /* MOJOSHADER_EFFECTS_H */
