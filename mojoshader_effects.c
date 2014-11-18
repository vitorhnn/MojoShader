/**
 * MojoShader; generate shader programs from bytecode of compiled
 *  Direct3D shaders.
 *
 * Please see the file LICENSE.txt in the source's root directory.
 *
 *  This file written by Ryan C. Gordon.
 */

#define __MOJOSHADER_INTERNAL__ 1
#include "mojoshader_internal.h"

#ifdef MOJOSHADER_EFFECT_SUPPORT

#include <math.h>

void MOJOSHADER_runPreshader(const MOJOSHADER_preshader *preshader,
                             float *outregs)
{
    const float *inregs = preshader->registers;

    // this is fairly straightforward, as there aren't any branching
    //  opcodes in the preshader instruction set (at the moment, at least).
    const int scalarstart = (int) MOJOSHADER_PRESHADEROP_SCALAR_OPS;

    double *temps = NULL;
    if (preshader->temp_count > 0)
    {
        temps = (double *) alloca(sizeof (double) * preshader->temp_count);
        memset(temps, '\0', sizeof (double) * preshader->temp_count);
    } // if

    double dst[4] = { 0, 0, 0, 0 };
    double src[3][4] = { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };
    const double *src0 = &src[0][0];
    const double *src1 = &src[1][0];
    const double *src2 = &src[2][0];

    MOJOSHADER_preshaderInstruction *inst = preshader->instructions;
    int instit;

    for (instit = 0; instit < preshader->instruction_count; instit++, inst++)
    {
        const MOJOSHADER_preshaderOperand *operand = inst->operands;
        const int elems = inst->element_count;
        const int elemsbytes = sizeof (double) * elems;
        const int isscalarop = (inst->opcode >= scalarstart);

        assert(elems >= 0);
        assert(elems <= 4);

        // load up our operands...
        int opiter, elemiter;
        for (opiter = 0; opiter < inst->operand_count-1; opiter++, operand++)
        {
            const int isscalar = ((isscalarop) && (opiter == 0));
            const unsigned int index = operand->index;
            switch (operand->type)
            {
                case MOJOSHADER_PRESHADEROPERAND_LITERAL:
                {
                    const double *lit = &preshader->literals[index];
                    assert((index + elems) <= preshader->literal_count);
                    if (!isscalar)
                        memcpy(&src[opiter][0], lit, elemsbytes);
                    else
                    {
                        const double val = *lit;
                        for (elemiter = 0; elemiter < elems; elemiter++)
                            src[opiter][elemiter] = val;
                    } // else
                    break;
                } // case

                case MOJOSHADER_PRESHADEROPERAND_INPUT:
                    // TODO: Handle array_register list! -flibit
                    if (isscalar)
                        src[opiter][0] = inregs[index];
                    else
                    {
                        int cpy;
                        for (cpy = 0; cpy < elems; cpy++)
                            src[opiter][cpy] = inregs[index+cpy];
                    } // else
                    break;

                case MOJOSHADER_PRESHADEROPERAND_OUTPUT:
                    if (isscalar)
                        src[opiter][0] = outregs[index];
                    else
                    {
                        int cpy;
                        for (cpy = 0; cpy < elems; cpy++)
                            src[opiter][cpy] = outregs[index+cpy];
                    } // else
                    break;

                case MOJOSHADER_PRESHADEROPERAND_TEMP:
                    if (temps != NULL)
                    {
                        if (isscalar)
                            src[opiter][0] = temps[index];
                        else
                            memcpy(src[opiter], temps + index, elemsbytes);
                    } // if
                    break;

                default:
                    assert(0 && "unexpected preshader operand type.");
                    return;
            } // switch
        } // for

        // run the actual instruction, store result to dst.
        int i;
        switch (inst->opcode)
        {
            #define OPCODE_CASE(op, val) \
                case MOJOSHADER_PRESHADEROP_##op: \
                    for (i = 0; i < elems; i++) { dst[i] = val; } \
                    break;

            //OPCODE_CASE(NOP, 0.0)  // not a real instruction.
            OPCODE_CASE(MOV, src0[i])
            OPCODE_CASE(NEG, -src0[i])
            OPCODE_CASE(RCP, 1.0 / src0[i])
            OPCODE_CASE(FRC, src0[i] - floor(src0[i]))
            OPCODE_CASE(EXP, exp(src0[i]))
            OPCODE_CASE(LOG, log(src0[i]))
            OPCODE_CASE(RSQ, 1.0 / sqrt(src0[i]))
            OPCODE_CASE(SIN, sin(src0[i]))
            OPCODE_CASE(COS, cos(src0[i]))
            OPCODE_CASE(ASIN, asin(src0[i]))
            OPCODE_CASE(ACOS, acos(src0[i]))
            OPCODE_CASE(ATAN, atan(src0[i]))
            OPCODE_CASE(MIN, (src0[i] < src1[i]) ? src0[i] : src1[i])
            OPCODE_CASE(MAX, (src0[i] > src1[i]) ? src0[i] : src1[i])
            OPCODE_CASE(LT, (src0[i] < src1[i]) ? 1.0 : 0.0)
            OPCODE_CASE(GE, (src0[i] >= src1[i]) ? 1.0 : 0.0)
            OPCODE_CASE(ADD, src0[i] + src1[i])
            OPCODE_CASE(MUL,  src0[i] * src1[i])
            OPCODE_CASE(ATAN2, atan2(src0[i], src1[i]))
            OPCODE_CASE(DIV, src0[i] / src1[i])
            OPCODE_CASE(CMP, (src0[i] >= 0.0) ? src1[i] : src2[i])
            //OPCODE_CASE(NOISE, ???)  // !!! FIXME: don't know what this does
            //OPCODE_CASE(MOVC, ???)  // !!! FIXME: don't know what this does
            OPCODE_CASE(MIN_SCALAR, (src0[0] < src1[i]) ? src0[0] : src1[i])
            OPCODE_CASE(MAX_SCALAR, (src0[0] > src1[i]) ? src0[0] : src1[i])
            OPCODE_CASE(LT_SCALAR, (src0[0] < src1[i]) ? 1.0 : 0.0)
            OPCODE_CASE(GE_SCALAR, (src0[0] >= src1[i]) ? 1.0 : 0.0)
            OPCODE_CASE(ADD_SCALAR, src0[0] + src1[i])
            OPCODE_CASE(MUL_SCALAR, src0[0] * src1[i])
            OPCODE_CASE(ATAN2_SCALAR, atan2(src0[0], src1[i]))
            OPCODE_CASE(DIV_SCALAR, src0[0] / src1[i])
            //OPCODE_CASE(DOT_SCALAR)  // !!! FIXME: isn't this just a MUL?
            //OPCODE_CASE(NOISE_SCALAR, ???)  // !!! FIXME: ?
            #undef OPCODE_CASE

            case MOJOSHADER_PRESHADEROP_DOT:
            {
                double final = 0.0;
                for (i = 0; i < elems; i++)
                    final += src0[i] * src1[i];
                for (i = 0; i < elems; i++)
                    dst[i] = final;  // !!! FIXME: is this right?
            } // case

            default:
                assert(0 && "Unhandled preshader opcode!");
                break;
        } // switch

        // Figure out where dst wants to be stored.
        if (operand->type == MOJOSHADER_PRESHADEROPERAND_TEMP)
        {
            assert(preshader->temp_count >=
                    operand->index + (elemsbytes / sizeof (double)));
            memcpy(temps + operand->index, dst, elemsbytes);
        } // if
        else
        {
            assert(operand->type == MOJOSHADER_PRESHADEROPERAND_OUTPUT);
            for (i = 0; i < elems; i++)
                outregs[operand->index + i] = (float) dst[i];
        } // else
    } // for
} // MOJOSHADER_runPreshader

static MOJOSHADER_effect MOJOSHADER_out_of_mem_effect = {
    1, &MOJOSHADER_out_of_mem_error, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static uint32 readui32(const uint8 **_ptr, uint32 *_len)
{
    uint32 retval = 0;
    if (*_len < sizeof (retval))
        *_len = 0;
    else
    {
        const uint32 *ptr = (const uint32 *) *_ptr;
        retval = SWAP32(*ptr);
        *_ptr += sizeof (retval);
        *_len -= sizeof (retval);
    } // else
    return retval;
} // readui32

static char *readstring(const uint8 *base,
                        const uint32 offset,
                        MOJOSHADER_malloc m,
                        void *d)
{
    // !!! FIXME: sanity checks!
    // !!! FIXME: verify this doesn't go past EOF looking for a null.
    const char *str = ((const char *) base) + offset;
    const uint32 len = *((const uint32 *) str);
    char *strptr = NULL;
    if (len == 0) return NULL; /* No length? No string. */
    strptr = (char *) m(len, d);
    memcpy(strptr, str + 4, len);
    return strptr;
} // readstring

static int findparameter(const MOJOSHADER_effectParam *params,
                         const uint32 param_count,
                         const char *name)
{
    int i;
    for (i = 0; i < param_count; i++)
        if (strcmp(name, params[i].value.name) == 0)
            return i;
    assert(0 && "Parameter not found!");
}

static void readvalue(const uint8 *base,
                      const uint32 typeoffset,
                      const uint32 valoffset,
                      MOJOSHADER_effectValue *value,
                      MOJOSHADER_effectObject *objects,
                      MOJOSHADER_malloc m,
                      void *d)
{
    int i;
    const uint8 *typeptr = base + typeoffset;
    const uint8 *valptr = base + valoffset;
    unsigned int typelen = 9999999;  // !!! FIXME
    const uint32 type = readui32(&typeptr, &typelen);
    const uint32 class = readui32(&typeptr, &typelen);
    const uint32 name = readui32(&typeptr, &typelen);
    const uint32 semantic = readui32(&typeptr, &typelen);
    const uint32 numelements = readui32(&typeptr, &typelen);

    value->value_type = (MOJOSHADER_symbolType) type;
    value->value_class = (MOJOSHADER_symbolClass) class;
    value->name = readstring(base, name, m, d);
    value->semantic = readstring(base, semantic, m, d);
    value->element_count = numelements;

    /* Class sanity check */
    assert(class >= MOJOSHADER_SYMCLASS_SCALAR && class <= MOJOSHADER_SYMCLASS_STRUCT);

    if (class == MOJOSHADER_SYMCLASS_SCALAR
     || class == MOJOSHADER_SYMCLASS_VECTOR
     || class == MOJOSHADER_SYMCLASS_MATRIX_ROWS
     || class == MOJOSHADER_SYMCLASS_MATRIX_COLUMNS)
    {
        /* These classes only ever contain scalar values */
        assert(type >= MOJOSHADER_SYMTYPE_BOOL && type <= MOJOSHADER_SYMTYPE_FLOAT);

        const uint32 columncount = readui32(&typeptr, &typelen);
        const uint32 rowcount = readui32(&typeptr, &typelen);

        value->column_count = columncount;
        value->row_count = rowcount;

        uint32 siz = columncount * rowcount;
        if (numelements > 0)
            siz *= numelements;
        value->value_count = siz;
        siz *= 4;
        value->values = m(siz, d);
        memcpy(value->values, valptr, siz);
    } // if
    else if (class == MOJOSHADER_SYMCLASS_OBJECT)
    {
        /* This class contains either samplers or "objects" */
        assert(type >= MOJOSHADER_SYMTYPE_STRING && type <= MOJOSHADER_SYMTYPE_VERTEXSHADER);

        if (type == MOJOSHADER_SYMTYPE_SAMPLER
         || type == MOJOSHADER_SYMTYPE_SAMPLER1D
         || type == MOJOSHADER_SYMTYPE_SAMPLER2D
         || type == MOJOSHADER_SYMTYPE_SAMPLER3D
         || type == MOJOSHADER_SYMTYPE_SAMPLERCUBE)
        {
            unsigned int vallen = 9999999; // !!! FIXME
            const uint32 numstates = readui32(&valptr, &vallen);

            value->value_count = numstates;

            const uint32 siz = sizeof(MOJOSHADER_effectSamplerState) * numstates;
            value->values = m(siz, d);
            memset(value->values, '\0', siz);

            for (i = 0; i < numstates; i++)
            {
                MOJOSHADER_effectSamplerState *state = &value->valuesSS[i];
                const uint32 stype = readui32(&valptr, &vallen) & ~0xA0;
                /*const uint32 FIXME =*/ readui32(&valptr, &vallen);
                const uint32 statetypeoffset = readui32(&valptr, &vallen);
                const uint32 statevaloffset = readui32(&valptr, &vallen);

                state->type = (MOJOSHADER_samplerStateType) stype;
                readvalue(base, statetypeoffset, statevaloffset,
                          &state->value, objects,
                          m, d);
                if (stype == MOJOSHADER_SAMP_TEXTURE)
                    objects[state->value.valuesI[0]].type = type;
            } // for
        } // if
        else
        {
            uint32 numobjects = 1;
            if (numelements > 0)
                numobjects = numelements;

            value->value_count = numobjects;

            const uint32 siz = 4 * numobjects;
            value->values = m(siz, d);
            memcpy(value->values, valptr, siz);

            for (i = 0; i < value->value_count; i++)
                objects[value->valuesI[i]].type = type;
        } // else
    } // else if
    else if (class == MOJOSHADER_SYMCLASS_STRUCT)
    {
        /* TODO: Maybe this is like parse_ctab_typeinfo? -flibit */
    } // else if
} // readvalue

static void readannotations(const uint32 numannos,
                            const uint8 *base,
                            const uint8 **ptr,
                            uint32 *len,
                            MOJOSHADER_effectAnnotation **annotations,
                            MOJOSHADER_effectObject *objects,
                            MOJOSHADER_malloc m,
                            void *d)
{
    int i;
    if (numannos == 0) return;

    const uint32 siz = sizeof(MOJOSHADER_effectAnnotation) * numannos;
    *annotations = (MOJOSHADER_effectAnnotation *) m(siz, d);
    memset(*annotations, '\0', siz);

    for (i = 0; i < numannos; i++)
    {
        MOJOSHADER_effectAnnotation *anno = &(*annotations)[i];

        const uint32 typeoffset = readui32(ptr, len);
        const uint32 valoffset = readui32(ptr, len);

        readvalue(base, typeoffset, valoffset,
                  anno, objects,
                  m, d);
    } // for
} // readannotation

static void readparameters(const uint32 numparams,
                           const uint8 *base,
                           const uint8 **ptr,
                           uint32 *len,
                           MOJOSHADER_effectParam **params,
                           MOJOSHADER_effectObject *objects,
                           MOJOSHADER_malloc m,
                           void *d)
{
    int i;
    if (numparams == 0) return;

    uint32 siz = sizeof(MOJOSHADER_effectParam) * numparams;
    *params = (MOJOSHADER_effectParam *) m(siz, d);
    memset(*params, '\0', siz);

    for (i = 0; i < numparams; i++)
    {
        MOJOSHADER_effectParam *param = &(*params)[i];

        const uint32 typeoffset = readui32(ptr, len);
        const uint32 valoffset = readui32(ptr, len);
        /*const uint32 flags =*/ readui32(ptr, len);
        const uint32 numannos = readui32(ptr, len);

        param->annotation_count = numannos;
        readannotations(numannos, base, ptr, len,
                        &param->annotations, objects,
                        m, d);

        readvalue(base, typeoffset, valoffset,
                  &param->value, objects,
                  m, d);
    } // for
} // readparameters

static void readstates(const uint32 numstates,
                       const uint8 *base,
                       const uint8 **ptr,
                       uint32 *len,
                       MOJOSHADER_effectState **states,
                       MOJOSHADER_effectObject *objects,
                       MOJOSHADER_malloc m,
                       void *d)
{
    int i;
    if (numstates == 0) return;

    const uint32 siz = sizeof (MOJOSHADER_effectState) * numstates;
    *states = (MOJOSHADER_effectState *) m(siz, d);
    memset(*states, '\0', siz);

    for (i = 0; i < numstates; i++)
    {
        MOJOSHADER_effectState *state = &(*states)[i];

        const uint32 type = readui32(ptr, len);
        /*const uint32 FIXME =*/ readui32(ptr, len);
        const uint32 typeoffset = readui32(ptr, len);
        const uint32 valoffset = readui32(ptr, len);

        state->type = (MOJOSHADER_renderStateType) type;
        readvalue(base, typeoffset, valoffset,
                  &state->value, objects,
                  m, d);
    } // for
} // readstates

static void readpasses(const uint32 numpasses,
                       const uint8 *base,
                       const uint8 **ptr,
                       uint32 *len,
                       MOJOSHADER_effectPass **passes,
                       MOJOSHADER_effectObject *objects,
                       MOJOSHADER_malloc m,
                       void *d)
{
    int i;
    if (numpasses == 0) return;

    const uint32 siz = sizeof (MOJOSHADER_effectPass) * numpasses;
    *passes = (MOJOSHADER_effectPass *) m(siz, d);
    memset(*passes, '\0', siz);

    for (i = 0; i < numpasses; i++)
    {
        MOJOSHADER_effectPass *pass = &(*passes)[i];

        const uint32 passnameoffset = readui32(ptr, len);
        const uint32 numannos = readui32(ptr, len);
        const uint32 numstates = readui32(ptr, len);

        pass->name = readstring(base, passnameoffset, m, d);

        pass->annotation_count = numannos;
        readannotations(numannos, base, ptr, len,
                        &pass->annotations, objects,
                        m, d);

        pass->state_count = numstates;
        readstates(numstates, base, ptr, len,
                   &pass->states, objects,
                   m, d);
    } // for
} // readpasses

static void readtechniques(const uint32 numtechniques,
                           const uint8 *base,
                           const uint8 **ptr,
                           uint32 *len,
                           MOJOSHADER_effectTechnique **techniques,
                           MOJOSHADER_effectObject *objects,
                           MOJOSHADER_malloc m,
                           void *d)
{
    int i;
    if (numtechniques == 0) return;

    const uint32 siz = sizeof (MOJOSHADER_effectTechnique) * numtechniques;
    *techniques = (MOJOSHADER_effectTechnique *) m(siz, d);
    memset(*techniques, '\0', siz);

    for (i = 0; i < numtechniques; i++)
    {
        MOJOSHADER_effectTechnique *technique = &(*techniques)[i];

        const uint32 nameoffset = readui32(ptr, len);
        const uint32 numannos = readui32(ptr, len);
        const uint32 numpasses = readui32(ptr, len);

        technique->name = readstring(base, nameoffset, m, d);

        technique->annotation_count = numannos;
        readannotations(numannos, base, ptr, len,
                        &technique->annotations, objects,
                        m, d);

        technique->pass_count = numpasses;
        readpasses(numpasses, base, ptr, len,
                   &technique->passes, objects,
                   m, d);
    } // for
} // readtechniques

static void readsmallobjects(const uint32 numsmallobjects,
                             const uint8 **ptr,
                             uint32 *len,
                             MOJOSHADER_effect *effect,
                             const char *profile,
                             const MOJOSHADER_swizzle *swiz,
                             const unsigned int swizcount,
                             const MOJOSHADER_samplerMap *smap,
                             const unsigned int smapcount,
                             MOJOSHADER_malloc m,
                             MOJOSHADER_free f,
                             void *d)
{
    int i, j;
    if (numsmallobjects == 0) return;

    for (i = 1; i < numsmallobjects + 1; i++)
    {
        const uint32 index = readui32(ptr, len);
        const uint32 length = readui32(ptr, len);

        MOJOSHADER_effectObject *object = &effect->objects[index];
        if (object->type == MOJOSHADER_SYMTYPE_STRING)
        {
            if (length > 0)
            {
                char *str = (char *) m(length, d);
                memcpy(str, *ptr, length);
                object->string.string = str;
            } // if
        } // if
        else if (object->type == MOJOSHADER_SYMTYPE_TEXTURE
              || object->type == MOJOSHADER_SYMTYPE_TEXTURE1D
              || object->type == MOJOSHADER_SYMTYPE_TEXTURE2D
              || object->type == MOJOSHADER_SYMTYPE_TEXTURE3D
              || object->type == MOJOSHADER_SYMTYPE_TEXTURECUBE)
        {
            object->texture.tex_register = *((uint32 *) *ptr);
        } // else if
        else if (object->type == MOJOSHADER_SYMTYPE_SAMPLER
              || object->type == MOJOSHADER_SYMTYPE_SAMPLER1D
              || object->type == MOJOSHADER_SYMTYPE_SAMPLER2D
              || object->type == MOJOSHADER_SYMTYPE_SAMPLER3D
              || object->type == MOJOSHADER_SYMTYPE_SAMPLERCUBE)
        {
            if (length > 0)
            {
                char *str = (char *) m(length, d);
                memcpy(str, *ptr, length);
                object->mapping.name = str;
            } // if
        } // else if
        else if (object->type == MOJOSHADER_SYMTYPE_PIXELSHADER
              || object->type == MOJOSHADER_SYMTYPE_VERTEXSHADER)
        {
            object->shader.technique = -1;
            object->shader.pass = -1;
            object->shader.shader = MOJOSHADER_parse(profile, *ptr, length,
                                                     swiz, swizcount, smap, smapcount,
                                                     m, f, d);
            // !!! FIXME: check for errors.
            for (j = 0; j < object->shader.shader->symbol_count; j++)
                if (object->shader.shader->symbols[j].register_set == MOJOSHADER_SYMREGSET_SAMPLER)
                    object->shader.sampler_count++;
            object->shader.param_count = object->shader.shader->symbol_count;
            object->shader.params = m(object->shader.param_count * sizeof (uint32), d);
            object->shader.samplers = m(object->shader.sampler_count * sizeof (MOJOSHADER_samplerStateRegister), d);
            uint32 curSampler = 0;
            for (j = 0; j < object->shader.shader->symbol_count; j++)
            {
                int par = findparameter(effect->params,
                                        effect->param_count,
                                        object->shader.shader->symbols[j].name);
                object->shader.params[j] = par;
                if (object->shader.shader->symbols[j].register_set == MOJOSHADER_SYMREGSET_SAMPLER)
                {
                    object->shader.samplers[curSampler].sampler_register = object->shader.shader->symbols[j].register_index;
                    object->shader.samplers[curSampler].sampler_state_count = effect->params[par].value.value_count;
                    object->shader.samplers[curSampler].sampler_states = effect->params[par].value.valuesSS;
                    curSampler++;
                } // if
            } // for
            if (object->shader.shader->preshader)
            {
                object->shader.preshader_param_count = object->shader.shader->preshader->symbol_count;
                object->shader.preshader_params = m(object->shader.preshader_param_count * sizeof (uint32), d);
                for (j = 0; j < object->shader.shader->preshader->symbol_count; j++)
                {
                    object->shader.preshader_params[j] = findparameter(effect->params,
                                                                       effect->param_count,
                                                                       object->shader.shader->preshader->symbols[j].name);
                } // for
            } // if
        } // else if
        else
        {
            assert(0 && "Small object type unknown!");
        } // else

        /* Object block is always a multiple of four */
        const uint32 blocklen = (length + 3) - ((length - 1) % 4);
        *ptr += blocklen;
        *len -= blocklen;
    } // for
} // readstrings

static void readlargeobjects(const uint32 numlargeobjects,
                             const uint32 numsmallobjects,
                             const uint8 **ptr,
                             uint32 *len,
                             MOJOSHADER_effect *effect,
                             const char *profile,
                             const MOJOSHADER_swizzle *swiz,
                             const unsigned int swizcount,
                             const MOJOSHADER_samplerMap *smap,
                             const unsigned int smapcount,
                             MOJOSHADER_malloc m,
                             MOJOSHADER_free f,
                             void *d)
{
    int i, j;
    if (numlargeobjects == 0) return;

    int numobjects = numsmallobjects + numlargeobjects + 1;
    for (i = numsmallobjects + 1; i < numobjects; i++)
    {
        const uint32 technique = readui32(ptr, len);
        const uint32 index = readui32(ptr, len);
        /*const uint32 FIXME =*/ readui32(ptr, len);
        const uint32 state = readui32(ptr, len);
        const uint32 type = readui32(ptr, len);
        const uint32 length = readui32(ptr, len);

        uint32 objectIndex;
        if (technique == -1)
            objectIndex = effect->params[index].value.valuesSS[state].value.valuesI[0];
        else
            objectIndex = effect->techniques[technique].passes[index].states[state].value.valuesI[0];

        MOJOSHADER_effectObject *object = &effect->objects[objectIndex];
        if (object->type == MOJOSHADER_SYMTYPE_PIXELSHADER
         || object->type == MOJOSHADER_SYMTYPE_VERTEXSHADER)
        {
            object->shader.technique = technique;
            object->shader.pass = index;

            const char *emitter = profile;
            if (type == 2)
            {
                /* This is a standalone preshader!
                 * It exists solely for effect passes that do not use a single
                 * vertex/fragment shader.
                 */
                object->shader.is_preshader = 1;
                const uint32 start = *((uint32 *) *ptr) + 4;
                const uint32 end = 24; // FIXME: Why? -flibit
                const char *array = readstring(*ptr, 0, m, d);
                object->shader.param_count = 1;
                object->shader.params = (uint32 *) m(sizeof (uint32), d);
                object->shader.params[0] = findparameter(effect->params,
                                                         effect->param_count,
                                                         array);
                f((void *) array, d);
                object->shader.preshader = MOJOSHADER_parsePreshader(*ptr + start, length - end,
                                                                     m, f, d);
                // !!! FIXME: check for errors.
                object->shader.preshader_param_count = object->shader.preshader->symbol_count;
                object->shader.preshader_params = m(object->shader.preshader_param_count * sizeof (uint32), d);
                for (j = 0; j < object->shader.preshader->symbol_count; j++)
                {
                    object->shader.preshader_params[j] = findparameter(effect->params,
                                                                       effect->param_count,
                                                                       object->shader.preshader->symbols[j].name);
                } // for
            } // if
            else
            {
                object->shader.shader = MOJOSHADER_parse(emitter, *ptr, length,
                                                         swiz, swizcount, smap, smapcount,
                                                         m, f, d);
                // !!! FIXME: check for errors.
                for (j = 0; j < object->shader.shader->symbol_count; j++)
                    if (object->shader.shader->symbols[j].register_set == MOJOSHADER_SYMREGSET_SAMPLER)
                        object->shader.sampler_count++;
                object->shader.param_count = object->shader.shader->symbol_count;
                object->shader.params = m(object->shader.param_count * sizeof (uint32), d);
                object->shader.samplers = m(object->shader.sampler_count * sizeof (MOJOSHADER_samplerStateRegister), d);
                uint32 curSampler = 0;
                for (j = 0; j < object->shader.shader->symbol_count; j++)
                {
                    int par = findparameter(effect->params,
                                            effect->param_count,
                                            object->shader.shader->symbols[j].name);
                    object->shader.params[j] = par;
                    if (object->shader.shader->symbols[j].register_set == MOJOSHADER_SYMREGSET_SAMPLER)
                    {
                        object->shader.samplers[curSampler].sampler_register = object->shader.shader->symbols[j].register_index;
                        object->shader.samplers[curSampler].sampler_state_count = effect->params[par].value.value_count;
                        object->shader.samplers[curSampler].sampler_states = effect->params[par].value.valuesSS;
                        curSampler++;
                    } // if
                } // for
                if (object->shader.shader->preshader)
                {
                    object->shader.preshader_param_count = object->shader.shader->preshader->symbol_count;
                    object->shader.preshader_params = m(object->shader.preshader_param_count * sizeof (uint32), d);
                    for (j = 0; j < object->shader.shader->preshader->symbol_count; j++)
                    {
                        object->shader.preshader_params[j] = findparameter(effect->params,
                                                                           effect->param_count,
                                                                           object->shader.shader->preshader->symbols[j].name);
                    } // for
                } // if
            }
        } // if
        else if (object->type == MOJOSHADER_SYMTYPE_SAMPLER
              || object->type == MOJOSHADER_SYMTYPE_SAMPLER1D
              || object->type == MOJOSHADER_SYMTYPE_SAMPLER2D
              || object->type == MOJOSHADER_SYMTYPE_SAMPLER3D
              || object->type == MOJOSHADER_SYMTYPE_SAMPLERCUBE)
        {
            if (length > 0)
            {
                char *str = (char *) m(length, d);
                memcpy(str, *ptr, length);
                object->mapping.name = str;
            } // if
        } // else if
        else
        {
            assert(0 && "Large object type unknown!");
        } // else

        /* Object block is always a multiple of four */
        const uint32 blocklen = (length + 3) - ((length - 1) % 4);
        *ptr += blocklen;
        *len -= blocklen;
    } // for
} // readobjects

MOJOSHADER_effect *MOJOSHADER_parseEffect(const char *profile,
                                          const unsigned char *buf,
                                          const unsigned int _len,
                                          const MOJOSHADER_swizzle *swiz,
                                          const unsigned int swizcount,
                                          const MOJOSHADER_samplerMap *smap,
                                          const unsigned int smapcount,
                                          MOJOSHADER_malloc m,
                                          MOJOSHADER_free f,
                                          void *d)
{
    const uint8 *ptr = (const uint8 *) buf;
    uint32 len = (uint32) _len;

    /* Supply both m and f, or neither */
    if ( ((m == NULL) && (f != NULL)) || ((m != NULL) && (f == NULL)) )
        return &MOJOSHADER_out_of_mem_effect;

    /* Use default malloc/free if m/f were not passed */
    if (m == NULL) m = MOJOSHADER_internal_malloc;
    if (f == NULL) f = MOJOSHADER_internal_free;

    /* malloc base effect structure */
    MOJOSHADER_effect *retval = m(sizeof (MOJOSHADER_effect), d);
    if (retval == NULL)
        return &MOJOSHADER_out_of_mem_effect;
    memset(retval, '\0', sizeof (*retval));

    /* Store m/f/d in effect structure */
    retval->malloc = m;
    retval->free = f;
    retval->malloc_data = d;

    if (len < 8)
        goto parseEffect_unexpectedEOF;

    /* Read in header magic, seek to initial offset */
    const uint8 *base = NULL;
    if (readui32(&ptr, &len) != 0xFEFF0901)
        goto parseEffect_notAnEffectsFile;
    else
    {
        const uint32 offset = readui32(&ptr, &len);
        base = ptr;
        if (offset > len)
            goto parseEffect_unexpectedEOF;
        ptr += offset;
        len -= offset;
    } // else

    if (len < 16)
        goto parseEffect_unexpectedEOF;

    /* Parse structure counts */
    const uint32 numparams = readui32(&ptr, &len);
    const uint32 numtechniques = readui32(&ptr, &len);
    /*const uint32 FIXME =*/ readui32(&ptr, &len);
    const uint32 numobjects = readui32(&ptr, &len);

    /* Alloc structures now, so object types can be stored */
    retval->object_count = numobjects;
    const uint32 siz = sizeof (MOJOSHADER_effectObject) * numobjects;
    retval->objects = (MOJOSHADER_effectObject *) m(siz, d);
    if (retval->objects == NULL)
        goto parseEffect_outOfMemory;
    memset(retval->objects, '\0', siz);

    /* Parse effect parameters */
    retval->param_count = numparams;
    readparameters(numparams, base, &ptr, &len,
                   &retval->params, retval->objects,
                   m, d);

    /* Parse effect techniques */
    retval->technique_count = numtechniques;
    readtechniques(numtechniques, base, &ptr, &len,
                   &retval->techniques, retval->objects,
                   m, d);

    /* Initial effect technique/pass */
    retval->current_technique = &retval->techniques[0];
    retval->current_pass = -1;

    if (len < 8)
        goto parseEffect_unexpectedEOF;

    /* Parse object counts */
    const int numsmallobjects = readui32(&ptr, &len);
    const int numlargeobjects = readui32(&ptr, &len);

    /* Parse "small" object table */
    readsmallobjects(numsmallobjects, &ptr, &len,
                     retval,
                     profile, swiz, swizcount, smap, smapcount,
                     m, f, d);

    /* Parse "large" object table. */
    readlargeobjects(numlargeobjects, numsmallobjects, &ptr, &len,
                     retval,
                     profile, swiz, swizcount, smap, smapcount,
                     m, f, d);

    /* Store MojoShader profile in effect structure */
    retval->profile = (char *) m(strlen(profile) + 1, d);
    if (retval->profile == NULL)
        goto parseEffect_outOfMemory;
    strcpy((char *) retval->profile, profile);

    return retval;

// !!! FIXME: do something with this.
parseEffect_notAnEffectsFile:
parseEffect_unexpectedEOF:
parseEffect_outOfMemory:
    MOJOSHADER_freeEffect(retval);
    return &MOJOSHADER_out_of_mem_effect;
} // MOJOSHADER_parseEffect


void freevalue(MOJOSHADER_effectValue *value, MOJOSHADER_free f, void *d)
{
    int i;
    f((void *) value->name, d);
    f((void *) value->semantic, d);
    if (value->value_type == MOJOSHADER_SYMTYPE_SAMPLER
     || value->value_type == MOJOSHADER_SYMTYPE_SAMPLER1D
     || value->value_type == MOJOSHADER_SYMTYPE_SAMPLER2D
     || value->value_type == MOJOSHADER_SYMTYPE_SAMPLER3D
     || value->value_type == MOJOSHADER_SYMTYPE_SAMPLERCUBE)
        for (i = 0; i < value->value_count; i++)
            freevalue(&value->valuesSS[i].value, f, d);
    f(value->values, d);
} // freevalue


void MOJOSHADER_freeEffect(const MOJOSHADER_effect *_effect)
{
    MOJOSHADER_effect *effect = (MOJOSHADER_effect *) _effect;
    if ((effect == NULL) || (effect == &MOJOSHADER_out_of_mem_effect))
        return;  // no-op.

    MOJOSHADER_free f = effect->free;
    void *d = effect->malloc_data;
    int i, j, k;

    /* Free errors */
    for (i = 0; i < effect->error_count; i++)
    {
        f((void *) effect->errors[i].error, d);
        f((void *) effect->errors[i].filename, d);
    } // for
    f((void *) effect->errors, d);

    /* Free profile string */
    f((void *) effect->profile, d);

    /* Free parameters, including annotations */
    for (i = 0; i < effect->param_count; i++)
    {
        MOJOSHADER_effectParam *param = &effect->params[i];
        freevalue(&param->value, f, d);
        for (j = 0; j < param->annotation_count; j++)
        {
            freevalue(&param->annotations[j], f, d);
        } // for
        f((void *) param->annotations, d);
    } // for
    f((void *) effect->params, d);

    /* Free techniques, including passes and all annotations */
    for (i = 0; i < effect->technique_count; i++)
    {
        MOJOSHADER_effectTechnique *technique = &effect->techniques[i];
        f((void *) technique->name, d);
        for (j = 0; j < technique->pass_count; j++)
        {
            MOJOSHADER_effectPass *pass = &technique->passes[j];
            f((void *) pass->name, d);
            for (k = 0; k < pass->state_count; k++)
            {
                freevalue(&pass->states[k].value, f, d);
            } // for
            f((void *) pass->states, d);
            for (k = 0; k < pass->annotation_count; k++)
            {
                freevalue(&pass->annotations[k], f, d);
            } // for
            f((void *) pass->annotations, d);
        } // for
        f((void *) technique->passes, d);
        for (j = 0; j < technique->annotation_count; j++)
        {
            freevalue(&technique->annotations[j], f, d);
        } // for
        f((void *) technique->annotations, d);
    } // for
    f((void *) effect->techniques, d);

    /* Free object table */
    for (i = 0; i < effect->object_count; i++)
    {
        MOJOSHADER_effectObject *object = &effect->objects[i];
        if (object->type == MOJOSHADER_SYMTYPE_PIXELSHADER
         || object->type == MOJOSHADER_SYMTYPE_VERTEXSHADER)
        {
            if (object->shader.is_preshader)
                MOJOSHADER_freePreshader(object->shader.preshader, f, d);
            else
                MOJOSHADER_freeParseData(object->shader.shader);
            f((void *) object->shader.params, d);
            f((void *) object->shader.samplers, d);
            f((void *) object->shader.preshader_params, d);
        } // if
        else if (object->type == MOJOSHADER_SYMTYPE_SAMPLER
              || object->type == MOJOSHADER_SYMTYPE_SAMPLER1D
              || object->type == MOJOSHADER_SYMTYPE_SAMPLER2D
              || object->type == MOJOSHADER_SYMTYPE_SAMPLER3D
              || object->type == MOJOSHADER_SYMTYPE_SAMPLERCUBE)
            f((void *) object->mapping.name, d);
        else if (object->type == MOJOSHADER_SYMTYPE_STRING)
            f((void *) object->string.string, d);
    } // for
    f((void *) effect->objects, d);

    /* Free base effect structure */
    f((void *) effect, d);
} // MOJOSHADER_freeEffect


void MOJOSHADER_effectSetRawValueHandle(const MOJOSHADER_effectParam *parameter,
                                        const void *data,
                                        const unsigned int offset,
                                        const unsigned int len)
{
    memcpy(parameter->value.values + offset, data, len);
} // MOJOSHADER_effectSetRawValueHandle


void MOJOSHADER_effectSetRawValueName(const MOJOSHADER_effect *effect,
                                      const char *name,
                                      const void *data,
                                      const unsigned int offset,
                                      const unsigned int len)
{
    int i;
    for (i = 0; i < effect->param_count; i++)
    {
        if (strcmp(name, effect->params[i].value.name) == 0)
        {
            memcpy(effect->params[i].value.values + offset, data, len);
            return;
        } // if
    } // for
    assert(0 && "Effect parameter not found!");
} // MOJOSHADER_effectSetRawValueName


const MOJOSHADER_effectTechnique *MOJOSHADER_effectGetCurrentTechnique(const MOJOSHADER_effect *effect)
{
    return effect->current_technique;
} // MOJOSHADER_effectGetCurrentTechnique


void MOJOSHADER_effectSetTechnique(MOJOSHADER_effect *effect,
                                   const MOJOSHADER_effectTechnique *technique)
{
    int i;
    for (i = 0; i < effect->technique_count; i++)
    {
        if (technique == &effect->techniques[i])
        {
            effect->current_technique = technique;
            return;
        } // if
    } // for
    assert(0 && "Technique is not part of this effect!");
} // MOJOSHADER_effectSetTechnique


const MOJOSHADER_effectTechnique *MOJOSHADER_effectFindNextTechnique(const MOJOSHADER_effect *effect,
                                                                     const MOJOSHADER_effectTechnique *technique
)
{
    int i;
    if (technique == NULL)
        return &effect->techniques[0];
    for (i = 0; i < effect->technique_count; i++)
    {
        if (technique == &effect->techniques[i])
        {
            if (i == effect->technique_count - 1)
                return NULL; /* We were passed the last technique! */
            return &effect->techniques[i + 1];
        } // if
    } // for
    assert(0 && "Technique is not part of this effect!");
} // MOJOSHADER_effectFindNextValidTechnique

#endif // MOJOSHADER_EFFECT_SUPPORT

// end of mojoshader_effects.c ...

