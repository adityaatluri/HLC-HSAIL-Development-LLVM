// University of Illinois/NCSA
// Open Source License
//
// Copyright (c) 2013, Advanced Micro Devices, Inc.
// All rights reserved.
//
// Developed by:
//
//     HSA Team
//
//     Advanced Micro Devices, Inc
//
//     www.amd.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal with
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so, subject to the following conditions:
//
//     * Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimers.
//
//     * Redistributions in binary form must reproduce the above copyright notice,
//       this list of conditions and the following disclaimers in the
//       documentation and/or other materials provided with the distribution.
//
//     * Neither the names of the LLVM Team, University of Illinois at
//       Urbana-Champaign, nor the names of its contributors may be used to
//       endorse or promote products derived from this Software without specific
//       prior written permission.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE
// SOFTWARE.
#include "HSAILUtilities.h"
#include "HSAILItems.h"
#include "HSAILValidator.h"
#include "HSAILInstProps.h"

#include <cassert>
#include <sstream>

using std::ostringstream;
using HSAIL_PROPS::isTypeProp;

namespace HSAIL_ASM {

//============================================================================
// Autogenerated utilities

#include "HSAILBrigInstUtils_gen.hpp"

//==============================================================================
// Autogenerated code for getting and setting BRIG properties

#include "HSAILBrigPropsAcc_gen.hpp"

//============================================================================
// Operations with Brig properties

const char* validateProp(unsigned propId, unsigned val, unsigned* vals, unsigned length, unsigned model, unsigned profile)
{
    using namespace Brig;

    if (profile == BRIG_PROFILE_FULL) return 0;

    if (propId == HSAIL_PROPS::PROP_FTZ)
    {
        // ftz=0 is allowed only if instruction does not support ftz, i.e. list of possible values only include 0
        // NB: if length=1, there is only one valid value x. If x!=val, we will fail elsewhere.
        if (val == 0 && length > 1) return "Base profile requires ftz modifier to be specified";
    }
    else if (propId == HSAIL_PROPS::PROP_ROUND)
    {
        if (val == BRIG_ROUND_NONE && length == 1) return 0;
        if (val == BRIG_ROUND_FLOAT_NEAR_EVEN || val == BRIG_ROUND_INTEGER_ZERO || val == BRIG_ROUND_INTEGER_ZERO_SAT) return 0;
        return "Base profile only supports 'near', 'zeroi' and 'zeroi_sat' rounding modes";
    }
    return 0;
}

const char* validateProp(unsigned propId, unsigned val, unsigned model, unsigned profile, bool imageExt)
{
    using namespace Brig;

    assert(model   == BRIG_MACHINE_SMALL || model == BRIG_MACHINE_LARGE);
    assert(profile == BRIG_PROFILE_BASE  || profile == BRIG_PROFILE_FULL);

    if (isTypeProp(propId))
    {
        if (isImageExtType(val) && !imageExt)                           return "Image and sampler types are only supported if the IMAGE extension has been specified";
        if (isFullProfileOnlyType(val) && profile != BRIG_PROFILE_FULL) return "f64 and f64x2 types are not supported by the Base pofile";
        if (val == BRIG_TYPE_SIG64 && model != BRIG_MACHINE_LARGE)      return "sig64 type is not supported by the small machine model";
        if (val == BRIG_TYPE_SIG32 && model != BRIG_MACHINE_SMALL)      return "sig32 type is not supported by the large machine model";
    }
    else if (propId == HSAIL_PROPS::PROP_IMAGESEGMENTMEMORYSCOPE)
    {
        if (val != BRIG_MEMORY_SCOPE_NONE && !imageExt) return "Image segment is only allowed if the IMAGE extension has been specified";
    }

    return 0;
}

// Used by TestGen
const char* validateProp(Inst inst, unsigned propId, unsigned model, unsigned profile, bool imageExt)
{
    if (propId == HSAIL_PROPS::PROP_IMAGESEGMENTMEMORYSCOPE || isTypeProp(propId))
    {
        return validateProp(propId, getBrigProp(inst, propId), model, profile, imageExt);
    }
    return 0;
}

bool hasImageExtProps(Inst inst)
{
    using namespace Brig;

    if (isImageInst(inst.opcode())       ||
        isImageExtType(getType(inst))    ||
        isImageExtType(getSrcType(inst)) ||
        isImageExtType(getImgType(inst))) return true;

    if (InstMemFence i = inst)
    {
        if (i.imageSegmentMemoryScope() != BRIG_MEMORY_SCOPE_NONE) return true;
    }

    for (unsigned i = 0; i < 5; ++i)
    {
        if (OperandAddress addr = inst.operand(i))
        {
            if (DirectiveVariable var = addr.symbol())
            {
                if (isImageExtType(var.type())) return true;
            }
        }
    }

    return false;
}

//============================================================================
// Operations with directives

bool isDirective(unsigned id)
{
    return Brig::BRIG_KIND_DIRECTIVE_BEGIN <= id && id < Brig::BRIG_KIND_DIRECTIVE_END;
}

SRef getName(Directive d)
{
    if      (DirectiveExecutable   dn = d) return dn.name();
    else if (DirectiveVariable     dn = d) return dn.name();
    else if (DirectiveLabel        dn = d) return dn.name();
    else if (DirectiveSignature    dn = d) return dn.name();
    else if (DirectiveFbarrier     dn = d) return dn.name();

    assert(false);
    return SRef();
}

unsigned getDataType(Directive d)
{
    assert(d);
    if      (DirectiveVariable     sym  = d) return sym.type();

    assert(false);
    return Brig::BRIG_TYPE_NONE;
}

unsigned getSegment(Directive d)
{
    if (DirectiveVariable sym = d)     return sym.segment();
    else if (DirectiveFbarrier(d))     return Brig::BRIG_SEGMENT_GROUP;

    assert(false);
    return Brig::BRIG_SEGMENT_NONE;
}

unsigned getSymLinkage(Directive d)
{
    if      (DirectiveVariable   ds = d) return ds.linkage();
    else if (DirectiveExecutable ds = d) return ds.linkage();
    else if (DirectiveFbarrier   ds = d) return ds.linkage();

    assert(false);
    return Brig::BRIG_LINKAGE_NONE;
}

bool isDecl(Directive d)
{
    return !isDef(d);
}

bool isDef(Directive d)
{
    if      (DirectiveVariable   ds = d) return ds.modifier().isDefinition();
    else if (DirectiveFbarrier   ds = d) return ds.modifier().isDefinition();
    else if (DirectiveExecutable ds = d) return ds.modifier().isDefinition();

    assert(false);
    return false;
}

DirectiveVariable getInputArg(DirectiveExecutable sbr, unsigned idx)
{
    assert(idx < sbr.inArgCount());

    Directive arg;
    for (arg = sbr.firstInArg(); idx-- > 0; arg = arg.next());

    assert(arg);
    assert(DirectiveVariable(arg));

    return arg;
}

unsigned getCtlDirOperandType(unsigned kind, unsigned idx)
{
    using namespace Brig;

    switch(kind)
    {
    case BRIG_CONTROL_ENABLEBREAKEXCEPTIONS:
    case BRIG_CONTROL_ENABLEDETECTEXCEPTIONS:
    case BRIG_CONTROL_MAXDYNAMICGROUPSIZE:
    case BRIG_CONTROL_MAXFLATWORKGROUPSIZE:
    case BRIG_CONTROL_REQUESTEDWORKGROUPSPERCU:
    case BRIG_CONTROL_REQUIREDDIM:
                                                    if (idx == 0) return BRIG_TYPE_U32;
                                                    break;
    case BRIG_CONTROL_MAXFLATGRIDSIZE:
                                                    if (idx == 0) return BRIG_TYPE_U64;
                                                    break;
    case BRIG_CONTROL_REQUIREDGRIDSIZE:
                                                    if (0 <= idx && idx <= 2) return BRIG_TYPE_U64;
                                                    break;
    case BRIG_CONTROL_REQUIREDWORKGROUPSIZE:
                                                    if (0 <= idx && idx <= 2) return BRIG_TYPE_U32;
                                                    break;

    case BRIG_CONTROL_REQUIRENOPARTIALWORKGROUPS:   break;

    default:
        assert(false);
        break;
    }
    return BRIG_TYPE_NONE;
}

const char* validateCtlDirOperandBounds(unsigned kind, unsigned idx, uint64_t val)
{
    using namespace Brig;

    switch(kind)
    {
    case BRIG_CONTROL_REQUIREDDIM:
        if (val < 1 || val > 3) return "Operand value must be in the range [1..3]";
        break;

    case BRIG_CONTROL_MAXFLATGRIDSIZE:
    case BRIG_CONTROL_MAXFLATWORKGROUPSIZE:
    case BRIG_CONTROL_REQUESTEDWORKGROUPSPERCU:
    case BRIG_CONTROL_REQUIREDGRIDSIZE:
    case BRIG_CONTROL_REQUIREDWORKGROUPSIZE:
        if (val == 0) return "Operand must be greater than 0";
        break;

    case BRIG_CONTROL_ENABLEBREAKEXCEPTIONS:
    case BRIG_CONTROL_ENABLEDETECTEXCEPTIONS:
    case BRIG_CONTROL_MAXDYNAMICGROUPSIZE:
    case BRIG_CONTROL_REQUIRENOPARTIALWORKGROUPS:
        break;

    default:
        assert(false);
        break;
    }

    return 0;
}

bool allowCtlDirOperandWs(unsigned kind)
{
    using namespace Brig;

    switch(kind)
    {
    case BRIG_CONTROL_REQUIREDGRIDSIZE:
    case BRIG_CONTROL_REQUIREDWORKGROUPSIZE:
    case BRIG_CONTROL_MAXFLATWORKGROUPSIZE:
    case BRIG_CONTROL_MAXFLATGRIDSIZE:
        return true;

    case BRIG_CONTROL_ENABLEBREAKEXCEPTIONS:
    case BRIG_CONTROL_ENABLEDETECTEXCEPTIONS:
    case BRIG_CONTROL_MAXDYNAMICGROUPSIZE:
    case BRIG_CONTROL_REQUESTEDWORKGROUPSPERCU:
    case BRIG_CONTROL_REQUIREDDIM:
    case BRIG_CONTROL_REQUIRENOPARTIALWORKGROUPS:
        return false;

    default:
        assert(false);
        return false;
    }
}

//============================================================================
// Operations with instructions

bool isInstruction(unsigned id){ return Brig::BRIG_KIND_INST_BEGIN <= id && id < Brig::BRIG_KIND_INST_END; }

unsigned getType(Inst inst)    { return getBrigProp(inst, HSAIL_PROPS::PROP_TYPE,       true, Brig::BRIG_TYPE_NONE);    }
unsigned getSrcType(Inst inst) { return getBrigProp(inst, HSAIL_PROPS::PROP_SOURCETYPE, true, Brig::BRIG_TYPE_NONE);    }
unsigned getCrdType(Inst inst) { return getBrigProp(inst, HSAIL_PROPS::PROP_COORDTYPE,  true, Brig::BRIG_TYPE_NONE);    }
unsigned getSigType(Inst inst) { return getBrigProp(inst, HSAIL_PROPS::PROP_SIGNALTYPE, true, Brig::BRIG_TYPE_NONE);    }
unsigned getImgType(Inst inst) { return getBrigProp(inst, HSAIL_PROPS::PROP_IMAGETYPE,  true, Brig::BRIG_TYPE_NONE);    }
unsigned getSegment(Inst inst) { return getBrigProp(inst, HSAIL_PROPS::PROP_SEGMENT,    true, Brig::BRIG_SEGMENT_NONE); }
unsigned getPacking(Inst inst) { return getBrigProp(inst, HSAIL_PROPS::PROP_PACK,       true, Brig::BRIG_PACK_NONE);    }
unsigned getEqClass(Inst inst) { return getBrigProp(inst, HSAIL_PROPS::PROP_EQUIVCLASS, true, 0);                       }

bool isImageInst(unsigned opcode)
{
    using namespace Brig;

    switch(opcode)
    {
    case BRIG_OPCODE_RDIMAGE:
    case BRIG_OPCODE_LDIMAGE:
    case BRIG_OPCODE_STIMAGE:
    case BRIG_OPCODE_QUERYIMAGE:
    case BRIG_OPCODE_QUERYSAMPLER:
        return true;
    default:
        return false;
    }
}

bool isCallInst(unsigned opcode)
{
    using namespace Brig;
    return opcode == BRIG_OPCODE_CALL  ||
           opcode == BRIG_OPCODE_SCALL ||
           opcode == BRIG_OPCODE_ICALL;
}

bool isBranchInst(unsigned opcode)
{
    using namespace Brig;
    return opcode == BRIG_OPCODE_BR  ||
           opcode == BRIG_OPCODE_CBR ||
           opcode == BRIG_OPCODE_SBR;
}

bool isTermInst(unsigned opcode)
{
    using namespace Brig;
    return opcode == BRIG_OPCODE_BR  ||
           opcode == BRIG_OPCODE_SBR ||
           opcode == BRIG_OPCODE_RET;
}

//============================================================================
// Operations with operands

bool isOperand(unsigned id)
{
    return Brig::BRIG_KIND_OPERAND_BEGIN <= id && id < Brig::BRIG_KIND_OPERAND_END;
}

bool isCodeRef(OperandCodeRef cr, unsigned targetKind)
{
    return cr && cr.ref() && cr.ref().kind() == targetKind;
}

unsigned getRegKind(SRef regName)
{
    string name = regName;

    if (name.empty()) return (unsigned)-1;

    switch(name[1])
    {
    case 'c': return 1;
    case 's': return 32;
    case 'd': return 64;
    case 'q': return 128;
    default:  return 0;
    }
}

unsigned getRegSize(OperandReg reg)
{
  using namespace Brig;
  switch(reg.regKind()) {
    case BRIG_REGISTER_CONTROL          : return 1;
    case BRIG_REGISTER_SINGLE           : return 32;
    case BRIG_REGISTER_DOUBLE           : return 64;
    case BRIG_REGISTER_QUAD             : return 128;
    default : return (unsigned)-1;
    }
}

string getRegName(OperandReg reg)
{
    ostringstream s;

    const char* regKind = registerKind2str(reg.regKind());
    s << regKind << reg.regNum();

    return s.str();
}

unsigned getImmSize(OperandData imm)
{
    assert(imm);
    SRef bytes = imm.data();
    return (unsigned)(bytes.length() * 8);
}

bool isImmB1(OperandData imm)
{
    assert(imm);
    SRef bytes = imm.data();
    return bytes.length() == 1 && (bytes[0] == 0 || bytes[0] == 1);
}

unsigned getAddrSize(OperandAddress addr, bool isLargeModel)
{
    assert(addr);

    if (addr.reg())    return getRegBits(addr.reg().regKind());
    if (addr.symbol()) return getSegAddrSize(addr.symbol().segment(), isLargeModel);

    // currently there are no limitations on offset size, even for small model and 32-bit addresses
    // if (addr.offset().hi() != 0) return 64;

    return 0; // unknown, both 32 and 64 are possible
}

unsigned getSegAddrSize(unsigned segment, bool isLargeModel)
{
    using namespace Brig;

    switch (segment)
    {
    case BRIG_SEGMENT_FLAT:
    case BRIG_SEGMENT_GLOBAL:
    case BRIG_SEGMENT_READONLY:
    case BRIG_SEGMENT_KERNARG:
        return isLargeModel? 64 : 32;

    default:
        return 32;
    }
}

//============================================================================
// Operations with types

bool isIntType(unsigned type)
{
    using namespace Brig;
    switch(type)
    {
    case BRIG_TYPE_B1:
    case BRIG_TYPE_B8:
    case BRIG_TYPE_B16:
    case BRIG_TYPE_B32:
    case BRIG_TYPE_B64:
    case BRIG_TYPE_B128:

    case BRIG_TYPE_S8:
    case BRIG_TYPE_S16:
    case BRIG_TYPE_S32:
    case BRIG_TYPE_S64:

    case BRIG_TYPE_U8:
    case BRIG_TYPE_U16:
    case BRIG_TYPE_U32:
    case BRIG_TYPE_U64:
        return true;

    default:
        return false;
    }
}

bool isSignedType(unsigned type)
{
    using namespace Brig;

    switch(type)
    {
    case BRIG_TYPE_S8:
    case BRIG_TYPE_S16:
    case BRIG_TYPE_S32:
    case BRIG_TYPE_S64:
        return true;

    default:
        return false;
    }
}

bool isUnsignedType(unsigned type)
{
    using namespace Brig;

    switch(type)
    {
    case BRIG_TYPE_U8:
    case BRIG_TYPE_U16:
    case BRIG_TYPE_U32:
    case BRIG_TYPE_U64:
        return true;

    default:
        return false;
    }
}

bool isBitType(unsigned type)
{
    using namespace Brig;
    switch(type)
    {
    case BRIG_TYPE_B1:
    case BRIG_TYPE_B8:
    case BRIG_TYPE_B16:
    case BRIG_TYPE_B32:
    case BRIG_TYPE_B64:
    case BRIG_TYPE_B128:
        return true;

    default:
        return false;
    }
}

bool isOpaqueType(unsigned type)
{
    using namespace Brig;
    switch(type)
    {
    case BRIG_TYPE_SAMP:
    case BRIG_TYPE_ROIMG:
    case BRIG_TYPE_WOIMG:
    case BRIG_TYPE_RWIMG:
    case BRIG_TYPE_SIG32:
    case BRIG_TYPE_SIG64:
        return true;

    default:
        return false;
    }
}

bool isImageExtType(unsigned type)
{
    using namespace Brig;
    switch(type)
    {
    case BRIG_TYPE_SAMP:
    case BRIG_TYPE_ROIMG:
    case BRIG_TYPE_WOIMG:
    case BRIG_TYPE_RWIMG:
        return true;

    default:
        return false;
    }
}

bool isImageType(unsigned type)
{
    using namespace Brig;
    switch(type)
    {
    case BRIG_TYPE_ROIMG:
    case BRIG_TYPE_WOIMG:
    case BRIG_TYPE_RWIMG:
        return true;

    default:
        return false;
    }
}


bool isFullProfileOnlyType(unsigned type)
{
    using namespace Brig;
    switch(type)
    {
    case BRIG_TYPE_F64:
    case BRIG_TYPE_F64X2:
        return true;

    default:
        return false;
    }
}

bool isFloatType(unsigned type)
{
    using namespace Brig;
    return type == BRIG_TYPE_F16 || type == BRIG_TYPE_F32 || type == BRIG_TYPE_F64;
}

bool isPackedType(unsigned type)
{
    return isIntPackedType(type) || isFloatPackedType(type);
}

bool isIntPackedType(unsigned type)
{
    using namespace Brig;
    switch(type)
    {
    case BRIG_TYPE_U8X4:
    case BRIG_TYPE_S8X4:
    case BRIG_TYPE_U16X2:
    case BRIG_TYPE_S16X2:
    case BRIG_TYPE_U8X8:
    case BRIG_TYPE_S8X8:
    case BRIG_TYPE_U16X4:
    case BRIG_TYPE_S16X4:
    case BRIG_TYPE_U32X2:
    case BRIG_TYPE_S32X2:
    case BRIG_TYPE_U8X16:
    case BRIG_TYPE_S8X16:
    case BRIG_TYPE_U16X8:
    case BRIG_TYPE_S16X8:
    case BRIG_TYPE_U32X4:
    case BRIG_TYPE_S32X4:
    case BRIG_TYPE_U64X2:
    case BRIG_TYPE_S64X2:
        return true;

    default:
        return false;
    }
}

bool isFloatPackedType(unsigned type)
{
    using namespace Brig;
    switch(type)
    {
    case BRIG_TYPE_F16X2:
    case BRIG_TYPE_F16X4:
    case BRIG_TYPE_F32X2:
    case BRIG_TYPE_F16X8:
    case BRIG_TYPE_F32X4:
    case BRIG_TYPE_F64X2:
        return true;
    default:
        return false;
    }
}

unsigned convType2BitType(unsigned type)
{
    using namespace Brig;
    switch(type)
    {
    case BRIG_TYPE_B1:     return BRIG_TYPE_B1;
    case BRIG_TYPE_B8:     return BRIG_TYPE_B8;
    case BRIG_TYPE_B16:    return BRIG_TYPE_B16;
    case BRIG_TYPE_B32:    return BRIG_TYPE_B32;
    case BRIG_TYPE_B64:    return BRIG_TYPE_B64;
    case BRIG_TYPE_B128:   return BRIG_TYPE_B128;

    case BRIG_TYPE_S8:     return BRIG_TYPE_B8;
    case BRIG_TYPE_S16:    return BRIG_TYPE_B16;
    case BRIG_TYPE_S32:    return BRIG_TYPE_B32;
    case BRIG_TYPE_S64:    return BRIG_TYPE_B64;

    case BRIG_TYPE_U8:     return BRIG_TYPE_B8;
    case BRIG_TYPE_U16:    return BRIG_TYPE_B16;
    case BRIG_TYPE_U32:    return BRIG_TYPE_B32;
    case BRIG_TYPE_U64:    return BRIG_TYPE_B64;

    case BRIG_TYPE_F16:    return BRIG_TYPE_B16;
    case BRIG_TYPE_F32:    return BRIG_TYPE_B32;
    case BRIG_TYPE_F64:    return BRIG_TYPE_B64;

    case BRIG_TYPE_U8X4:   return BRIG_TYPE_B32;
    case BRIG_TYPE_S8X4:   return BRIG_TYPE_B32;
    case BRIG_TYPE_U16X2:  return BRIG_TYPE_B32;
    case BRIG_TYPE_S16X2:  return BRIG_TYPE_B32;
    case BRIG_TYPE_F16X2:  return BRIG_TYPE_B32;
    case BRIG_TYPE_U8X8:   return BRIG_TYPE_B64;
    case BRIG_TYPE_S8X8:   return BRIG_TYPE_B64;
    case BRIG_TYPE_U16X4:  return BRIG_TYPE_B64;
    case BRIG_TYPE_S16X4:  return BRIG_TYPE_B64;
    case BRIG_TYPE_F16X4:  return BRIG_TYPE_B64;
    case BRIG_TYPE_U32X2:  return BRIG_TYPE_B64;
    case BRIG_TYPE_S32X2:  return BRIG_TYPE_B64;
    case BRIG_TYPE_F32X2:  return BRIG_TYPE_B64;
    case BRIG_TYPE_U8X16:  return BRIG_TYPE_B128;
    case BRIG_TYPE_S8X16:  return BRIG_TYPE_B128;
    case BRIG_TYPE_U16X8:  return BRIG_TYPE_B128;
    case BRIG_TYPE_S16X8:  return BRIG_TYPE_B128;
    case BRIG_TYPE_F16X8:  return BRIG_TYPE_B128;
    case BRIG_TYPE_U32X4:  return BRIG_TYPE_B128;
    case BRIG_TYPE_S32X4:  return BRIG_TYPE_B128;
    case BRIG_TYPE_F32X4:  return BRIG_TYPE_B128;
    case BRIG_TYPE_U64X2:  return BRIG_TYPE_B128;
    case BRIG_TYPE_S64X2:  return BRIG_TYPE_B128;
    case BRIG_TYPE_F64X2:  return BRIG_TYPE_B128;

    case BRIG_TYPE_SAMP:
    case BRIG_TYPE_ROIMG:
    case BRIG_TYPE_WOIMG:
    case BRIG_TYPE_RWIMG:
    case BRIG_TYPE_SIG32:
    case BRIG_TYPE_SIG64:  return BRIG_TYPE_B64;


    default:
        return BRIG_TYPE_NONE;
    }
}

unsigned convPackedType2U(unsigned type)
{
    using namespace Brig;
    switch(type)
    {
    case BRIG_TYPE_U8X4:   return BRIG_TYPE_U8X4;
    case BRIG_TYPE_S8X4:   return BRIG_TYPE_U8X4;
    case BRIG_TYPE_U16X2:  return BRIG_TYPE_U16X2;
    case BRIG_TYPE_S16X2:  return BRIG_TYPE_U16X2;
    case BRIG_TYPE_F16X2:  return BRIG_TYPE_U16X2;
    case BRIG_TYPE_U8X8:   return BRIG_TYPE_U8X8;
    case BRIG_TYPE_S8X8:   return BRIG_TYPE_U8X8;
    case BRIG_TYPE_U16X4:  return BRIG_TYPE_U16X4;
    case BRIG_TYPE_S16X4:  return BRIG_TYPE_U16X4;
    case BRIG_TYPE_F16X4:  return BRIG_TYPE_U16X4;
    case BRIG_TYPE_U32X2:  return BRIG_TYPE_U32X2;
    case BRIG_TYPE_S32X2:  return BRIG_TYPE_U32X2;
    case BRIG_TYPE_F32X2:  return BRIG_TYPE_U32X2;
    case BRIG_TYPE_U8X16:  return BRIG_TYPE_U8X16;
    case BRIG_TYPE_S8X16:  return BRIG_TYPE_U8X16;
    case BRIG_TYPE_U16X8:  return BRIG_TYPE_U16X8;
    case BRIG_TYPE_S16X8:  return BRIG_TYPE_U16X8;
    case BRIG_TYPE_F16X8:  return BRIG_TYPE_U16X8;
    case BRIG_TYPE_U32X4:  return BRIG_TYPE_U32X4;
    case BRIG_TYPE_S32X4:  return BRIG_TYPE_U32X4;
    case BRIG_TYPE_F32X4:  return BRIG_TYPE_U32X4;
    case BRIG_TYPE_U64X2:  return BRIG_TYPE_U64X2;
    case BRIG_TYPE_S64X2:  return BRIG_TYPE_U64X2;
    case BRIG_TYPE_F64X2:  return BRIG_TYPE_U64X2;

    default:
        return BRIG_TYPE_NONE;
    }
}

bool isValidImmType(unsigned type)
{
    return type != Brig::BRIG_TYPE_NONE && !isOpaqueType(type);
}

bool isValidVarType(unsigned type)
{
    return type != Brig::BRIG_TYPE_NONE && type != Brig::BRIG_TYPE_B1;
}

unsigned getBitType(unsigned size)
{
    using namespace Brig;

    switch(size) {
    case 1:   return BRIG_TYPE_B1;
    case 8:   return BRIG_TYPE_B8;
    case 16:  return BRIG_TYPE_B16;
    case 32:  return BRIG_TYPE_B32;
    case 64:  return BRIG_TYPE_B64;
    case 128: return BRIG_TYPE_B128;
    default:
        assert(false);
        return BRIG_TYPE_NONE;
    }
}

unsigned getSignedType(unsigned size)
{
    using namespace Brig;

    switch(size) {
    case 8:   return BRIG_TYPE_S8;
    case 16:  return BRIG_TYPE_S16;
    case 32:  return BRIG_TYPE_S32;
    case 64:  return BRIG_TYPE_S64;
    default:
        assert(false);
        return BRIG_TYPE_NONE;
    }
}

unsigned getUnsignedType(unsigned size)
{
    using namespace Brig;

    switch(size) {
    case 8:   return BRIG_TYPE_U8;
    case 16:  return BRIG_TYPE_U16;
    case 32:  return BRIG_TYPE_U32;
    case 64:  return BRIG_TYPE_U64;
    default:
        assert(false);
        return BRIG_TYPE_NONE;
    }
}

unsigned expandSubwordType(unsigned type)
{
    using namespace Brig;

    switch(type)
    {
    case BRIG_TYPE_B1:  assert(false);

    case BRIG_TYPE_B8:
    case BRIG_TYPE_B16: return BRIG_TYPE_B32;

    case BRIG_TYPE_U8:
    case BRIG_TYPE_U16: return BRIG_TYPE_U32;

    case BRIG_TYPE_S8:
    case BRIG_TYPE_S16: return BRIG_TYPE_S32;

    default:            return type;
    }
}

unsigned packedType2elementType(unsigned type)
{
    using namespace Brig;

    switch(type)
    {
    case BRIG_TYPE_U8X4:
    case BRIG_TYPE_U8X8:
    case BRIG_TYPE_U8X16:   return BRIG_TYPE_U8;

    case BRIG_TYPE_U16X2:
    case BRIG_TYPE_U16X4:
    case BRIG_TYPE_U16X8:   return BRIG_TYPE_U16;

    case BRIG_TYPE_U32X2:
    case BRIG_TYPE_U32X4:   return BRIG_TYPE_U32;

    case BRIG_TYPE_U64X2:   return BRIG_TYPE_U64;

    case BRIG_TYPE_S8X4:
    case BRIG_TYPE_S8X8:
    case BRIG_TYPE_S8X16:   return BRIG_TYPE_S8;

    case BRIG_TYPE_S16X2:
    case BRIG_TYPE_S16X4:
    case BRIG_TYPE_S16X8:   return BRIG_TYPE_S16;

    case BRIG_TYPE_S32X2:
    case BRIG_TYPE_S32X4:   return BRIG_TYPE_S32;

    case BRIG_TYPE_S64X2:   return BRIG_TYPE_S64;

    case BRIG_TYPE_F16X2:
    case BRIG_TYPE_F16X4:
    case BRIG_TYPE_F16X8:   return BRIG_TYPE_F16;

    case BRIG_TYPE_F32X2:
    case BRIG_TYPE_F32X4:   return BRIG_TYPE_F32;

    case BRIG_TYPE_F64X2:   return BRIG_TYPE_F64;

    default:
        assert(false);
        return BRIG_TYPE_NONE;
    }
}

unsigned packedType2baseType(unsigned type)
{
    return expandSubwordType(packedType2elementType(type));
}

//============================================================================
// Operations with packing

bool isSatPacking(unsigned packing)
{
    using namespace Brig;

    switch(packing)
    {
    case BRIG_PACK_PP:
    case BRIG_PACK_PS:
    case BRIG_PACK_SP:
    case BRIG_PACK_SS:
    case BRIG_PACK_S:
    case BRIG_PACK_P:
        return false;

    case BRIG_PACK_PPSAT:
    case BRIG_PACK_PSSAT:
    case BRIG_PACK_SPSAT:
    case BRIG_PACK_SSSAT:
    case BRIG_PACK_SSAT:
    case BRIG_PACK_PSAT:
        return true;

    default:
        assert(false);
        return false;
    }
}

bool isUnrPacking(unsigned packing)
{
    using namespace Brig;

    switch(packing)
    {
    case BRIG_PACK_S:
    case BRIG_PACK_P:
    case BRIG_PACK_SSAT:
    case BRIG_PACK_PSAT:
        return true;

    case BRIG_PACK_PP:
    case BRIG_PACK_PS:
    case BRIG_PACK_SP:
    case BRIG_PACK_SS:
    case BRIG_PACK_PPSAT:
    case BRIG_PACK_PSSAT:
    case BRIG_PACK_SPSAT:
    case BRIG_PACK_SSSAT:
        return false;

    default:
        assert(false);
        return false;
    }
}

bool isBinPacking(unsigned packing)
{
    return !isUnrPacking(packing);
}

unsigned getPackedTypeDim(unsigned type)
{
    using namespace Brig;

    switch(type)
    {
    case BRIG_TYPE_U16X2:
    case BRIG_TYPE_S16X2:
    case BRIG_TYPE_F16X2:
    case BRIG_TYPE_U32X2:
    case BRIG_TYPE_S32X2:
    case BRIG_TYPE_F32X2:
    case BRIG_TYPE_U64X2:
    case BRIG_TYPE_S64X2:
    case BRIG_TYPE_F64X2:  return 2;

    case BRIG_TYPE_U8X4:
    case BRIG_TYPE_S8X4:
    case BRIG_TYPE_U16X4:
    case BRIG_TYPE_S16X4:
    case BRIG_TYPE_F16X4:
    case BRIG_TYPE_U32X4:
    case BRIG_TYPE_S32X4:
    case BRIG_TYPE_F32X4:  return 4;

    case BRIG_TYPE_U8X8:
    case BRIG_TYPE_S8X8:
    case BRIG_TYPE_U16X8:
    case BRIG_TYPE_S16X8:
    case BRIG_TYPE_F16X8:  return 8;

    case BRIG_TYPE_U8X16:
    case BRIG_TYPE_S8X16:  return 16;

    default:               return 0;
    }
}

char getPackingControl(unsigned srcOperandIdx, unsigned packing)
{
    assert(srcOperandIdx == 0 || srcOperandIdx == 1);

    using namespace Brig;

    const char* ctl;
    switch(packing) {
    case BRIG_PACK_NONE:    ctl = "  "; break;
    case BRIG_PACK_P:       ctl = "p "; break;
    case BRIG_PACK_PP:      ctl = "pp"; break;
    case BRIG_PACK_PPSAT:   ctl = "pp"; break;
    case BRIG_PACK_PS:      ctl = "ps"; break;
    case BRIG_PACK_PSAT:    ctl = "p "; break;
    case BRIG_PACK_PSSAT:   ctl = "ps"; break;
    case BRIG_PACK_S:       ctl = "s "; break;
    case BRIG_PACK_SP:      ctl = "sp"; break;
    case BRIG_PACK_SPSAT:   ctl = "sp"; break;
    case BRIG_PACK_SS:      ctl = "ss"; break;
    case BRIG_PACK_SSAT:    ctl = "s "; break;
    case BRIG_PACK_SSSAT:   ctl = "ss"; break;
    default:                ctl = "  "; break;
    }

    return ctl[srcOperandIdx];
}

unsigned getPackedDstDim(unsigned type, unsigned packing)
{
    assert(isPackedType(type));

    return (getPackingControl(0, packing) == 'p' || getPackingControl(1, packing) == 'p')? getPackedTypeDim(type) : 1;
}

//============================================================================
// Operations with alignment

Brig::BrigAlignment getNaturalAlignment(unsigned type)
{
    return num2align(getBrigTypeNumBytes(type));
}

Brig::BrigAlignment getMaxAlignment()
{
    return Brig::BRIG_ALIGNMENT_MAX;
}

bool isValidAlignment(unsigned align, unsigned type)
{
    return align2num(getNaturalAlignment(type)) <= align2num(align);
}

//============================================================================
// Misc operations

const char* width2str(unsigned val)
{
    using namespace Brig;

    switch(val)
    {
    case BRIG_WIDTH_1                   : return "width(1)";
    case BRIG_WIDTH_1024                : return "width(1024)";
    case BRIG_WIDTH_1048576             : return "width(1048576)";
    case BRIG_WIDTH_1073741824          : return "width(1073741824)";
    case BRIG_WIDTH_128                 : return "width(128)";
    case BRIG_WIDTH_131072              : return "width(131072)";
    case BRIG_WIDTH_134217728           : return "width(134217728)";
    case BRIG_WIDTH_16                  : return "width(16)";
    case BRIG_WIDTH_16384               : return "width(16384)";
    case BRIG_WIDTH_16777216            : return "width(16777216)";
    case BRIG_WIDTH_2                   : return "width(2)";
    case BRIG_WIDTH_2048                : return "width(2048)";
    case BRIG_WIDTH_2097152             : return "width(2097152)";
    case BRIG_WIDTH_2147483648          : return "width(2147483648)";
    case BRIG_WIDTH_256                 : return "width(256)";
    case BRIG_WIDTH_262144              : return "width(262144)";
    case BRIG_WIDTH_268435456           : return "width(268435456)";
    case BRIG_WIDTH_32                  : return "width(32)";
    case BRIG_WIDTH_32768               : return "width(32768)";
    case BRIG_WIDTH_33554432            : return "width(33554432)";
    case BRIG_WIDTH_4                   : return "width(4)";
    case BRIG_WIDTH_4096                : return "width(4096)";
    case BRIG_WIDTH_4194304             : return "width(4194304)";
    case BRIG_WIDTH_512                 : return "width(512)";
    case BRIG_WIDTH_524288              : return "width(524288)";
    case BRIG_WIDTH_536870912           : return "width(536870912)";
    case BRIG_WIDTH_64                  : return "width(64)";
    case BRIG_WIDTH_65536               : return "width(65536)";
    case BRIG_WIDTH_67108864            : return "width(67108864)";
    case BRIG_WIDTH_8                   : return "width(8)";
    case BRIG_WIDTH_8192                : return "width(8192)";
    case BRIG_WIDTH_8388608             : return "width(8388608)";
    case BRIG_WIDTH_ALL                 : return "width(all)";
    case BRIG_WIDTH_WAVESIZE            : return "width(WAVESIZE)";
    case BRIG_WIDTH_NONE                : return "";
    default:
        return NULL;
    }
}

//============================================================================
// Misc operations

std::ostream& operator<<(std::ostream& os, const SRef& s) {
    os.write(s.begin,s.length());
    return os;
}

size_t align(size_t s, size_t pow2)
{
    assert( (pow2 & (pow2-1))==0 );
    size_t const m = pow2-1;
    return (s + m) & ~m;
}

/// zero rightpadded copy. Copies min(len,room) elements from src to dst,
/// if len < room fills the gap with zeroes. returns min(len,room).
size_t zeroPaddedCopy(void *dst, const void* src, size_t len, size_t room)
{
    size_t const toCopy = std::min(len,room);
    memcpy(dst,src,toCopy);
    size_t const pad = room-toCopy;
    if (pad > 0) memset(reinterpret_cast<char*>(dst)+toCopy,0,pad);
    return toCopy;
}

//============================================================================

} // end namespace

