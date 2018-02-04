// ================================================================================================
// -*- C++ -*-
// File: ls_vm.cpp
// Author: LuisCM
// Created on: 24/08/2001
// Brief: The Virtual Machine and execution model.
// ================================================================================================

#include "ls_vm.hpp"
#include "ls_semantic_check.hpp" // For some type conversion helpers
#include <iomanip>            // For std::setw & friends

namespace lavascript
{

// ========================================================
// Opcode handlers:
// ========================================================

// Function signature of an opcode handler. See the array of handlers below.
using OpCodeHandlerCB = void (*)(LSVM &, UInt32);

static void opNOOP(LSVM &, UInt32)
{
    // Unsurprisingly, a no-op does nothing.
}

static void opProgEnd(LSVM & vm, UInt32)
{
    // Set the program count to the end of the code vector to terminate
    // execution, even if there was more code after this instruction.
    vm.setProgramCounter(static_cast<int>(vm.code.size()));
}

static void opJump(LSVM & vm, const UInt32 operandIndex)
{
    vm.setProgramCounter(operandIndex);
}

static void opJumpIfTrue(LSVM & vm, const UInt32 operandIndex)
{
    if (!vm.stack.pop().isZero())
    {
        vm.setProgramCounter(operandIndex);
    }
}

static void opJumpIfFalse(LSVM & vm, const UInt32 operandIndex)
{
    if (vm.stack.pop().isZero())
    {
        vm.setProgramCounter(operandIndex);
    }
}

static void opLoadRVR(LSVM & vm, UInt32)
{
    vm.stack.push(vm.getReturnValue());
}

static void opStoreRVR(LSVM & vm, UInt32)
{
    vm.setReturnValue(vm.stack.pop());
}

static void opLoadGlobal(LSVM & vm, const UInt32 operandIndex)
{
    // Push integer operand to the VM stack.
    vm.stack.push(vm.data[operandIndex]);
}

static void opStoreGlobal(LSVM & vm, const UInt32 operandIndex)
{
    // Stores the current stack top to the data index of
    // this instruction, then pop the VM stack.
    performAssignmentWithConversion(vm.data[operandIndex], vm.stack.pop());
}

static void opLoadLocal(LSVM & vm, const UInt32 operandIndex)
{
    const auto argc = vm.locals.getTopVar().getAsInteger();
    const auto args = vm.locals.slice(vm.locals.getCurrSize() - argc - 1, argc);

    vm.stack.push(args[operandIndex]);
}

static void opStoreLocal(LSVM & vm, const UInt32 operandIndex)
{
    const auto argc = vm.locals.getTopVar().getAsInteger();

    // Expand the local stack if needed:
    if (operandIndex >= argc)
    {
        Variant argcVar = vm.locals.pop();
        argcVar.value.asInteger += 1;

        vm.locals.push(vm.stack.pop());
        vm.locals.push(argcVar);
    }
    else // Assign to already allocated stack variable (function local or parameter):
    {
        auto args = vm.locals.slice(vm.locals.getCurrSize() - argc - 1, argc);
        performAssignmentWithConversion(args[operandIndex], vm.stack.pop());
    }
}

static void opMemberStoreCommon(LSVM & vm, Variant objRef)
{
    const auto memberCount = vm.stack.pop().getAsInteger();
    const auto offsets = vm.stack.slice(vm.stack.getCurrSize() - memberCount, memberCount);
    vm.stack.popN(memberCount);

    const Variant storeValue = vm.stack.pop();
    LSObject::Member * memberRef = nullptr;

    // We have a list of member offsets in the stack, so
    // we start from the head object and keep going down
    // until the last member reference that is the target
    // of this store instruction.
    for (int o = 0; o < offsets.getSize(); ++o)
    {
        LSObject * thisObj = objRef.getAsObject();
        if (thisObj == nullptr)
        {
            LAVASCRIPT_RUNTIME_EXCEPTION("opMemberStore: trying to access member of a null object!");
        }

        memberRef = &thisObj->getMemberAt(offsets[o].getAsInteger());
        objRef = memberRef->data;
    }

    LAVASCRIPT_ASSERT(memberRef != nullptr);
    performAssignmentWithConversion(memberRef->data, storeValue);
}

static void opMemberStoreGlobal(LSVM & vm, const UInt32 operandIndex)
{
    // Source the target object from the global program data.
    opMemberStoreCommon(vm, vm.data[operandIndex]);
}

static void opMemberStoreLocal(LSVM & vm, const UInt32 operandIndex)
{
    // Source the target object from the function-local stack.
    const auto argc = vm.locals.getTopVar().getAsInteger();
    auto args = vm.locals.slice(vm.locals.getCurrSize() - argc - 1, argc);
    opMemberStoreCommon(vm, args[operandIndex]);
}

static void opStoreArraySubLocal(LSVM & vm, const UInt32 operandIndex)
{
    const Variant subscriptVar   = vm.stack.pop();
    const Variant assignedValVar = vm.stack.pop();

    const auto argc = vm.locals.getTopVar().getAsInteger();
    auto args = vm.locals.slice(vm.locals.getCurrSize() - argc - 1, argc);

    LSArray * destArray = args[operandIndex].getAsArray();

    const int index = static_cast<int>(subscriptVar.getAsInteger());
    destArray->setIndex(index, assignedValVar);
}

static void opStoreArraySubGlobal(LSVM & vm, const UInt32 operandIndex)
{
    const Variant subscriptVar   = vm.stack.pop();
    const Variant assignedValVar = vm.stack.pop();

    LSArray * destArray = vm.data[operandIndex].getAsArray();

    const int index = static_cast<int>(subscriptVar.getAsInteger());
    destArray->setIndex(index, assignedValVar);
}

static void opStoreArraySubStk(LSVM & vm, UInt32)
{
    const Variant subscriptVar   = vm.stack.pop();
    const Variant assignedValVar = vm.stack.pop();
    const Variant arrayObjVar    = vm.stack.pop();

    if (arrayObjVar.type != Variant::Type::LSArray)
    {
    	LAVASCRIPT_RUNTIME_EXCEPTION("opStoreArraySubStk: expected an array object!");
    }

    LSArray * destArray = arrayObjVar.getAsArray();

    const int index = static_cast<int>(subscriptVar.getAsInteger());
    destArray->setIndex(index, assignedValVar);
}

static void opStoreSetTypeLocal(LSVM & vm, const UInt32 operandIndex)
{
    const auto argc = vm.locals.getTopVar().getAsInteger();

    // Expand the local stack if needed:
    if (operandIndex >= argc)
    {
        Variant argcVar = vm.locals.pop();

        // Padding is needed mostly for the for-loops that can reuse
        // local stack indexes, so we might be reusing a higher index
        // that is not yet allocated, so pad with nulls.
        if ((operandIndex - argc) > 1)
        {
            auto diff = operandIndex - argc;
            argcVar.value.asInteger += diff;

            while (diff--)
            {
                vm.locals.push(Variant{});
            }
        }

        argcVar.value.asInteger += 1;
        vm.locals.push(vm.stack.pop());
        vm.locals.push(argcVar);
    }
    else // Assign to already allocated stack variable (function local or parameter):
    {
        auto args = vm.locals.slice(vm.locals.getCurrSize() - argc - 1, argc);
        args[operandIndex] = vm.stack.pop(); // Override the type instead of assigning with conversion.
    }
}

static void opStoreSetTypeGlobal(LSVM & vm, const UInt32 operandIndex)
{
    // Override the type instead of assigning with conversion.
    vm.data[operandIndex] = vm.stack.pop();
}

static void opCallCommon(LSVM & vm, const Variant funcVar)
{
    if (funcVar.type != Variant::Type::Function)
    {
    	LAVASCRIPT_RUNTIME_EXCEPTION("opCall: expected a function object!");
    }
    if (funcVar.value.asFunction == nullptr)
    {
    	LAVASCRIPT_RUNTIME_EXCEPTION("opCall: attempting to call a null function object!");
    }

    const Variant argcVar = vm.stack.getTopVar();
    const auto argc = argcVar.value.asInteger;

    if (argcVar.type != Variant::Type::Integer)
    {
    	LAVASCRIPT_RUNTIME_EXCEPTION("opCall: function arg count sentry should be an integer!");
    }

    vm.funcArgs = vm.stack.slice(vm.stack.getCurrSize() - argc - 1, argc + 1);
    vm.stack.pop(); // the argCount

    const auto callArgs = vm.stack.slice(vm.stack.getCurrSize() - argc, argc);
    vm.stack.popN(argc);

    funcVar.value.asFunction->invoke(vm, callArgs);
}

static void opCall(LSVM & vm, const UInt32 operandIndex)
{
    // Source the function pointer from the global program data.
    opCallCommon(vm, vm.data[operandIndex]);
}

static void opCallLocal(LSVM & vm, const UInt32 operandIndex)
{
    // Source the function pointer from the local function stack.
    const auto argc = vm.locals.getTopVar().getAsInteger();
    const auto args = vm.locals.slice(vm.locals.getCurrSize() - argc - 1, argc);
    opCallCommon(vm, args[operandIndex]);
}

static void opFuncStart(LSVM & vm, const UInt32 operandIndex)
{
    const Variant funcVar = vm.data[operandIndex];

    // Turn the stack slice into an array for the varargs:
    if (funcVar.getAsFunction()->isVarArgs())
    {
        LSStack::LSSlice arrayInitializers = vm.funcArgs;
        arrayInitializers.pop();

        // We don't known the types of each argument, so the array tag is 'Any'.
        Variant result{ Variant::Type::LSArray };
        result.value.asArray = LSArray::newFromArgs(vm, vm.types.anyTypeId, arrayInitializers);
        vm.locals.push(result);

        // Also push the argCount (1) so we can rollback at FuncEnd:
        Variant argc{ Variant::Type::Integer };
        argc.value.asInteger = 1;
        vm.locals.push(argc);
    }
    else // Copy from the main work stack to the function-local stack (including the argCount):
    {
        for (int i = 0; i < vm.funcArgs.getSize(); ++i)
        {
            vm.locals.push(vm.funcArgs[i]);
        }
    }

    // Save so we can return to the caller.
    Variant retAddr{ Variant::Type::Integer };
    retAddr.value.asInteger = vm.getReturnAddress();
    vm.stack.push(retAddr);

    #if LAVASCRIPT_SAVE_SCRIPT_CALLSTACK
    vm.callstack.push(funcVar);
    #endif // LAVASCRIPT_SAVE_SCRIPT_CALLSTACK
}

static void opFuncEnd(LSVM & vm, const UInt32 operandIndex)
{
    const auto argc = vm.locals.getTopVar().getAsInteger();
    vm.locals.popN(argc + 1); // +1 to also pop the argCount

    // Pop the return address pushed by FuncStart and jump to it on the next instruction.
    const Variant retAddr = vm.stack.pop();
    vm.setProgramCounter(retAddr.getAsInteger());

    // We can double check this here, but the compiler should have
    // already validated the return type against the return statements.
    const Variant funcVar = vm.data[operandIndex];
    funcVar.getAsFunction()->validateReturnValue(vm.getReturnValue());

    #if LAVASCRIPT_SAVE_SCRIPT_CALLSTACK
    vm.callstack.pop();
    #endif // LAVASCRIPT_SAVE_SCRIPT_CALLSTACK

    // Proactive GC tries to reclaim memory at the end of every function.
    // This approach keeps the allocation cost more balanced, since there's
    // not risk a GC run will be triggered when allocating a new object.
    if (vm.gc.needToCollectGarbage())
    {
        vm.gc.collectGarbage(vm);
    }
}

static void opNewVar(LSVM & vm, const UInt32 operandIndex)
{
    const Variant tidVar = vm.data[operandIndex];
    Variant initVal;

    if (tidVar.type == Variant::Type::Null)
    {
        initVal.type = Variant::Type::Null;
    }
    else
    {
        initVal.type = typeId2VarType(tidVar.getAsTypeId());
    }

    const Variant hasInitializer = vm.stack.pop();
    if (hasInitializer.toBool())
    {
        const Variant rhs = vm.stack.pop();
        performAssignmentWithConversion(initVal, rhs);
    }
    vm.stack.push(initVal);
}

static void opNewObj(LSVM & vm, const UInt32 operandIndex)
{
    const Variant tidVar = vm.data[operandIndex];
    if (tidVar.type != Variant::Type::Tid || tidVar.value.asTypeId == nullptr)
    {
        LAVASCRIPT_RUNTIME_EXCEPTION("opNewObj: expected a type id!");
    }

    const Variant constructorArgCount = vm.stack.pop();
    const auto argc = constructorArgCount.getAsInteger();
    const auto args = vm.stack.slice(vm.stack.getCurrSize() - argc, argc);

    Variant newObj{ Variant::Type::LSObject };
    newObj.value.asObject = newRuntimeObject(vm, tidVar.value.asTypeId, args);

    vm.stack.popN(argc);
    vm.stack.push(newObj);
}

static void opEnumDynInit(LSVM & vm, const UInt32 operandIndex)
{
    const Variant tidVar = vm.stack.pop();
    if (tidVar.type != Variant::Type::Tid || tidVar.value.asTypeId == nullptr)
    {
        LAVASCRIPT_RUNTIME_EXCEPTION("opEnumDynInit: expected a type id!");
    }

    const TypeId * tid = tidVar.getAsTypeId();
    if (tid->templateObject == nullptr || !tid->templateObject->isEnumType)
    {
        LAVASCRIPT_RUNTIME_EXCEPTION("opEnumDynInit: invalid enum TypeId!");
    }

    Variant enumInstance{ Variant::Type::LSObject };
    enumInstance.value.asObject = const_cast<LSObject *>(tid->templateObject);

    vm.data[operandIndex] = enumInstance;
}

static void opMemberRef(LSVM & vm, UInt32)
{
    // Pop the object and its member index from
    // the stack and push the member var back:
    const Variant memberIdx = vm.stack.pop();
    const Variant objRef    = vm.stack.pop();

    const LSObject * thisObj = objRef.getAsObject();
    if (thisObj == nullptr)
    {
        LAVASCRIPT_RUNTIME_EXCEPTION("opMemberRef: trying to access member of a null object!");
    }

    const LSObject::Member member = thisObj->getMemberAt(memberIdx.getAsInteger());
    vm.stack.push(member.data);
}

static void opNewArray(LSVM & vm, UInt32)
{
    const auto elementCount = vm.stack.pop().getAsInteger();
    const auto arrayInitializers = vm.stack.slice(vm.stack.getCurrSize() - elementCount, elementCount);
    vm.stack.popN(elementCount);

    const Variant::Type underlayingType = arrayInitializers[0].type; // Type of first item dictates the storage type
    const TypeId * dataTypeId = varType2TypeId(vm.types, underlayingType);

    auto newArrayObj = LSArray::newFromArgs(vm, dataTypeId, arrayInitializers);

    Variant result{ Variant::Type::LSArray };
    result.value.asArray = newArrayObj;
    vm.stack.push(result);
}

static void opArraySubscript(LSVM & vm, UInt32)
{
    const int index = vm.stack.pop().getAsInteger();
    const LSArray * array = vm.stack.pop().getAsArray();
    vm.stack.push(array->getIndex(index));
}

static void opNewRange(LSVM & vm, UInt32)
{
    Range newRange;
    newRange.end   = vm.stack.pop().getAsInteger();
    newRange.begin = vm.stack.pop().getAsInteger();

    Variant result{ Variant::Type::Range };
    result.value.asRange = newRange;
    vm.stack.push(result);
}

template<OpCode OP>
static void opUnary(LSVM & vm, UInt32)
{
    vm.stack.push(performUnaryOp(OP, vm.stack.pop()));
}

template<OpCode OP>
static void opBinary(LSVM & vm, UInt32)
{
    const Variant operandB = vm.stack.pop();
    const Variant operandA = vm.stack.pop();
    vm.stack.push(performBinaryOp(OP, operandA, operandB, &vm));
}

static void opTypeof(LSVM & vm, UInt32)
{
    // type_of() simply converts its operand Variant to a TypeId.
    // Note that for the 'Any' type we actually want to get the underlaying type, not "any".
    const Variant operand = vm.stack.pop();
    const Variant::Type varType = (operand.isAny() ? operand.anyType : operand.type);

    Variant tidVar{ Variant::Type::Tid };
    if (varType == Variant::Type::LSObject && operand.value.asObject != nullptr)
    {
        // For objects we can get the precise type
        tidVar.value.asTypeId = operand.value.asObject->getTypeId();
    }
    else // Other types might get a generic tag
    {
        tidVar.value.asTypeId = varType2TypeId(vm.types, varType);
    }

    vm.stack.push(tidVar);
}

static void opTypecast(LSVM & vm, UInt32)
{
    const Variant targetTid = vm.stack.pop();
    const Variant operand   = vm.stack.pop();
    const Variant::Type targetVarType = typeId2VarType(targetTid.getAsTypeId());

    Variant destVar{ targetVarType };
    performAssignmentWithConversion(destVar, operand);
    vm.stack.push(destVar);
}

static void opForLoopPrep(LSVM & vm, UInt32)
{
    Variant forParam = vm.stack.pop();
    Variant forIter  = vm.stack.pop();
    Variant forIdx   = vm.stack.pop();

    if (forParam.type == Variant::Type::Range)
    {
        forIdx.value.asInteger  = forParam.value.asRange.begin;
        forIter.value.asInteger = forParam.value.asRange.begin;
    }
    else if (forParam.type == Variant::Type::LSArray)
    {
        forIdx.value.asInteger = 0;
        performAssignmentWithConversion(forIter, forParam.value.asArray->getIndex(0));
    }
    else
    {
        LAVASCRIPT_RUNTIME_EXCEPTION("opForLoopPrep: for loop param is not a range or array!");
    }

    vm.stack.push(forIter);
    vm.stack.push(forIdx);
    // Gets written back to the 'i' and '__for_idx__' vars
}

static void opForLoopTest(LSVM & vm, UInt32)
{
    Variant forParam = vm.stack.pop();
    Variant forIdx   = vm.stack.pop();
    Variant continueLooping{ Variant::Type::Integer };

    if (forParam.type == Variant::Type::Range)
    {
        continueLooping.value.asInteger = (forIdx.value.asInteger != forParam.value.asRange.end);
    }
    else if (forParam.type == Variant::Type::LSArray)
    {
        continueLooping.value.asInteger = (forIdx.value.asInteger != forParam.value.asArray->getArrayLength());
    }
    else
    {
        LAVASCRIPT_RUNTIME_EXCEPTION("opForLoopTest: for loop param is not a range or array!");
    }

    vm.stack.push(continueLooping);
}

static void opForLoopStep(LSVM & vm, UInt32)
{
    Variant forParam = vm.stack.pop();
    Variant forIter  = vm.stack.pop();
    Variant forIdx   = vm.stack.pop();

    forIdx.value.asInteger += 1;
    const int idx = forIdx.value.asInteger;

    if (forParam.type == Variant::Type::Range)
    {
        forIter.value.asInteger = idx;
    }
    else if (forParam.type == Variant::Type::LSArray)
    {
        if (idx < forParam.value.asArray->getArrayLength())
        {
            performAssignmentWithConversion(forIter, forParam.value.asArray->getIndex(idx));
        }
    }
    else
    {
        LAVASCRIPT_RUNTIME_EXCEPTION("opForLoopStep: for loop param is not a range or array!");
    }

    vm.stack.push(forIter);
    vm.stack.push(forIdx);
    // Gets written back to the 'i' and '__for_idx__' vars
}

static void opMatchTest(LSVM & vm, UInt32)
{
    const Variant caseVar = vm.stack.pop();
    const Variant testVar = vm.stack.pop();
    Variant result;

    if (caseVar.type == Variant::Type::Range && testVar.type != Variant::Type::Range)
    {
        Int32 test;
        const Range range = caseVar.getAsRange();

        if (testVar.type == Variant::Type::Integer)
        {
            test = static_cast<Int32>(testVar.getAsInteger());
        }
        else if (testVar.type == Variant::Type::LSStr)
        {
            if (testVar.value.asString->getStringLength() != 1)
            {
                LAVASCRIPT_RUNTIME_EXCEPTION("expected single character literal in match statement");
            }

            // Get the ASCII value of the character:
            test = *(testVar.value.asString->c_str());
        }
        else
        {
            LAVASCRIPT_RUNTIME_EXCEPTION("cannot compare range with " + toString(testVar.type));
        }

        result.type = Variant::Type::Integer;
        result.value.asInteger = (test >= range.begin && test < range.end);
    }
    else if (caseVar.type == Variant::Type::LSArray && testVar.type != Variant::Type::LSArray)
    {
        // Test each array element against the testVar.
        const LSArray * array = caseVar.getAsArray();
        const int count = array->getArrayLength();

        for (int i = 0; i < count; ++i)
        {
            const Variant arrayElement = array->getIndex(i);
            result = performBinaryOp(OpCode::CmpEqual, testVar, arrayElement, &vm);
            if (!result.isZero())
            {
                break;
            }
        }
    }
    else
    {
        result = performBinaryOp(OpCode::CmpEqual, testVar, caseVar, &vm);
    }

    // The test value is put back into the stack for the next case.
    // MatchEnd removes it when reached.
    vm.stack.push(testVar);
    vm.stack.push(result);
}

static void opMatchEnd(LSVM & vm, UInt32)
{
    // Pop the match test value.
    vm.stack.pop();
}

// ----------------------------------------------------------------------------
// opHandlerCallbacks[]:
//
// The handlers for each instruction opcode.
// These are called by the VM for each instruction
// in the program by VM::executeSingleInstruction().
// ----------------------------------------------------------------------------
static const OpCodeHandlerCB opHandlerCallbacks[]
{
    &opNOOP,                            // NoOp
    &opNOOP,                            // ProgStart
    &opProgEnd,                         // ProgEnd
    &opJump,                            // Jmp
    &opJumpIfTrue,                      // JmpIfTrue
    &opJumpIfFalse,                     // JmpIfFalse
    &opJump,                            // JmpReturn
    &opTypeof,                          // Typeof
    &opTypecast,                        // Typecast
    &opCall,                            // Call
    &opCallLocal,                       // CallLocal
    &opNewVar,                          // NewVar
    &opNewRange,                        // NewRange
    &opNewArray,                        // NewArray
    &opNewObj,                          // NewObj
    &opEnumDynInit,                     // EnumDynInit
    &opFuncStart,                       // FuncStart
    &opFuncEnd,                         // FuncEnd
    &opForLoopPrep,                     // ForLoopPrep
    &opForLoopTest,                     // ForLoopTest
    &opForLoopStep,                     // ForLoopStep
    &opMatchTest,                       // MatchTest
    &opMatchEnd,                        // MatchEnd
    &opArraySubscript,                  // ArraySubscript
    &opMemberRef,                       // MemberRef
    &opLoadRVR,                         // LoadRVR
    &opStoreRVR,                        // StoreRVR
    &opLoadGlobal,                      // LoadGlobal
    &opStoreGlobal,                     // StoreGlobal
    &opMemberStoreGlobal,               // MemberStoreGlobal
    &opLoadLocal,                       // LoadLocal
    &opStoreLocal,                      // StoreLocal
    &opMemberStoreLocal,                // MemberStoreLocal
    &opStoreArraySubLocal,              // StoreArraySubLocal
    &opStoreArraySubGlobal,             // StoreArraySubGlobal
    &opStoreArraySubStk,                // StoreArraySubStk
    &opStoreSetTypeLocal,               // StoreSetTypeLocal
    &opStoreSetTypeGlobal,              // StoreSetTypeGlobal
    &opBinary<OpCode::CmpNotEqual>,     // CmpNotEqual
    &opBinary<OpCode::CmpEqual>,        // CmpEqual
    &opBinary<OpCode::CmpGreaterEqual>, // CmpGreaterEqual
    &opBinary<OpCode::CmpGreater>,      // CmpGreater
    &opBinary<OpCode::CmpLessEqual>,    // CmpLessEqual
    &opBinary<OpCode::CmpLess>,         // CmpLess
    &opBinary<OpCode::LogicOr>,         // LogicOr
    &opBinary<OpCode::LogicAnd>,        // LogicAnd
    &opBinary<OpCode::Sub>,             // Sub
    &opBinary<OpCode::Add>,             // Add
    &opBinary<OpCode::Mod>,             // Mod
    &opBinary<OpCode::Div>,             // Div
    &opBinary<OpCode::Mul>,             // Mul
    &opUnary<OpCode::LogicNot>,         // LogicNot
    &opUnary<OpCode::Negate>,           // Negate
    &opUnary<OpCode::Plus>              // Plus
};
static_assert(arrayLength(opHandlerCallbacks) == UInt32(OpCode::Count),
              "Keep this array in sync with the enum declaration!");

// ========================================================
// VM class methods:
// ========================================================

LSVM::LSVM(const bool loadBuiltIns, const int stackSize)
    : stack{ stackSize }
    , locals{ stackSize }
    , types{ *this }
    #if LAVASCRIPT_GLOBALS_TABLE
    , globals{ data }
    #endif // LAVASCRIPT_GLOBALS_TABLE
    #if LAVASCRIPT_SAVE_SCRIPT_CALLSTACK
    , callstack{ stackSize }
    #endif // LAVASCRIPT_SAVE_SCRIPT_CALLSTACK
{
    if (loadBuiltIns)
    {
        registerNativeBuiltInFunctions(functions);
    }
}

void LSVM::setProgramCounter(const int target)
{
    if (target < 0 || target > static_cast<int>(code.size()))
    {
        LAVASCRIPT_RUNTIME_EXCEPTION("invalid instruction index: " + toString(target));
    }

    // The -1 is necessary because the 'for' loop
    // in execute() will still increment the pc after
    // executeSingleInstruction() returns.
    // This is arguably a hack. Perhaps it should
    // be implemented in a more clear way...
    pc = target - 1;
}

void LSVM::setReturnAddress(const int target)
{
    if (target < 0 || target > static_cast<int>(code.size()))
    {
        LAVASCRIPT_RUNTIME_EXCEPTION("invalid instruction index: " + toString(target));
    }

    retAddr = target;
}

void LSVM::executeSingleInstruction(const OpCode op, const UInt32 operandIndex)
{
    const UInt32 handlerIndex = static_cast<UInt32>(op);
    LAVASCRIPT_ASSERT(handlerIndex < static_cast<UInt32>(OpCode::Count));
    return opHandlerCallbacks[handlerIndex](*this, operandIndex);
}

void LSVM::execute()
{
    const int instructionCount = static_cast<int>(code.size());
    for (pc = 0; pc < instructionCount; ++pc)
    {
        OpCode op;
        UInt32 operandIndex;
        unpackInstruction(code[pc], op, operandIndex);
        executeSingleInstruction(op, operandIndex);
    }

    // If a stack is left dirty at the end of a program, there's probably a bug somewhere.
    LAVASCRIPT_ASSERT(stack.isEmpty()  && "LSVM work stack should be empty at the end of a program!");
    LAVASCRIPT_ASSERT(locals.isEmpty() && "Function stack should be empty at the end of a program!");
}

void LSVM::execute(const int firstInstruction, const int maxInstructionsToExec)
{
    int n = 0;
    const int instructionCount = static_cast<int>(code.size());

    for (pc = firstInstruction; pc < instructionCount && n < maxInstructionsToExec; ++pc, ++n)
    {
        OpCode op;
        UInt32 operandIndex;
        unpackInstruction(code[pc], op, operandIndex);
        executeSingleInstruction(op, operandIndex);
    }
}

void LSVM::executeWithCallback(const std::function<bool(LSVM & vm, OpCode, UInt32)> & callback, const int firstInstruction)
{
    const int instructionCount = static_cast<int>(code.size());
    for (pc = firstInstruction; pc < instructionCount; ++pc)
    {
        OpCode op;
        UInt32 operandIndex;
        unpackInstruction(code[pc], op, operandIndex);

        if (!callback(*this, op, operandIndex))
        {
            break;
        }

        executeSingleInstruction(op, operandIndex);
    }
}

Variant LSVM::call(const char * funcName, const std::initializer_list<Variant> & arguments)
{
    LAVASCRIPT_ASSERT(funcName != nullptr && *funcName != '\0');

    const Function * const funcPtr = functions.findFunction(funcName);
    if (funcPtr == nullptr)
    {
        LAVASCRIPT_RUNTIME_EXCEPTION("attempting to call undefined function \'" + toString(funcName) + "\'!");
    }

    Variant funcVar{ Variant::Type::Function };
    Variant argcVar{ Variant::Type::Integer  };

    funcVar.value.asFunction = funcPtr;
    argcVar.value.asInteger  = arguments.size();

    for (const Variant & arg : arguments)
    {
        stack.push(arg);
    }

    const int savedPC = pc;
    pc = 0;

    stack.push(argcVar);
    opCallCommon(*this, funcVar);

    // Script-defined functions must run some script code.
    if (funcPtr->isScript())
    {
        executeWithCallback(
            [funcPtr](LSVM & vm, const OpCode op, const UInt32 operandIndex)
            {
                if (op == OpCode::FuncEnd)
                {
                    if (vm.data[operandIndex].getAsFunction() == funcPtr)
                    {
                        // Execute the FuncEnd and stop.
                        vm.executeSingleInstruction(op, operandIndex);
                        return false;
                    }
                }
                // Continue the execution.
                return true;
            }, pc + 1);
    }

    pc = savedPC;
    return funcPtr->hasReturnVal() ? rvr : Variant{};
}

#if LAVASCRIPT_SAVE_SCRIPT_CALLSTACK
void LSVM::printStackTrace(std::ostream & os) const
{
    os << color::white() << "[[ stack trace ]]" << color::restore() << "\n";

    if (callstack.isEmpty())
    {
        os << "(empty)\n";
        return;
    }

    int index = 0;
    auto s = callstack.slice(0, callstack.getCurrSize());

    for (const Variant * var = s.first(); var != nullptr; var = s.next(), ++index)
    {
        os << std::setw(3) << index << ": " << var->getAsFunction()->name->chars << "\n";
    }
}
#endif // LAVASCRIPT_SAVE_SCRIPT_CALLSTACK

// ========================================================
// GC class methods and helpers:
// ========================================================

static void markVar(const Variant var);
static void markObject(LSObject * obj);
static void markArray(LSArray * array);

static void markVar(const Variant var)
{
    switch (var.type)
    {
    case Variant::Type::LSStr    : markObject(var.getAsString()); break;
    case Variant::Type::LSObject : markObject(var.getAsObject()); break;
    case Variant::Type::LSArray  : markArray(var.getAsArray());   break;
    case Variant::Type::Any      : markVar(var.unwrapAny());      break;
    default : break; // Not a GC object.
    } // switch (var.type)
}

static void markObject(LSObject * obj)
{
    if (obj == nullptr)
    {
        return;
    }

    // If already marked, we're done. Check this first
    // to avoid recursing on cycles in the object graph.
    if (obj->isReachable)
    {
        return;
    }
    obj->isReachable = true;

    // Member sub-objects must also be marked:
    const int memberCount = obj->getMemberCount();
    for (int m = 0; m < memberCount; ++m)
    {
        LSObject::Member member = obj->getMemberAt(m);
        markVar(member.data);
    }
}

static void markArray(LSArray * array)
{
    if (array == nullptr)
    {
        return;
    }
    if (array->isReachable)
    {
        return;
    }
    array->isReachable = true;

    // Arrays must mark each index if the array is storing GC'd objects.
    const Variant::Type type = array->getDataType();
    if (type == Variant::Type::LSStr    ||
        type == Variant::Type::LSObject ||
        type == Variant::Type::LSArray  ||
        type == Variant::Type::Any)
    {
        const int count = array->getArrayLength();
        for (int i = 0; i < count; ++i)
        {
            markVar(array->getIndex(i));
        }
    }
}

void LSGC::markAll(LSVM & vm)
{
    // Globals are always reachable.
    for (Variant var : vm.data)
    {
        markVar(var);
    }

    // Mark the stack variables:
    if (!vm.stack.isEmpty())
    {
        auto s = vm.stack.slice(0, vm.stack.getCurrSize());
        for (const Variant * var = s.first(); var != nullptr; var = s.next())
        {
            markVar(*var);
        }
    }
    if (!vm.locals.isEmpty())
    {
        auto s = vm.locals.slice(0, vm.locals.getCurrSize());
        for (const Variant * var = s.first(); var != nullptr; var = s.next())
        {
            markVar(*var);
        }
    }
}

void LSGC::sweep()
{
    LSObject ** object = &gcListHead;
    while (*object)
    {
        if (!(*object)->isReachable && !(*object)->isPersistent)
        {
            // This object wasn't reached, so remove it from the list and free it.
            LSObject * unreachable = *object;
            *object = unreachable->gcNext;
            LSGC::free(unreachable);
        }
        else
        {
            // This object was reached, so unmark it (for the next GC) and move on to the next.
            (*object)->isReachable = false;
            object = &(*object)->gcNext;
        }
    }
}

void LSGC::collectGarbage(LSVM & vm)
{
    #if LAVASCRIPT_DEBUG
    const int savedNumAlive = gcAliveCount;
    #endif // LAVASCRIPT_DEBUG

    markAll(vm);
    sweep();

    // Next GC will take longer.
    gcMaxCount = std::max(1, gcAliveCount * 2);

    #if LAVASCRIPT_DEBUG
    logStream() << "LavaScript: GC collected " << savedNumAlive - gcAliveCount << " objects. "
                << gcAliveCount << " remaining, " << gcMaxCount << " new max alive count.\n";
    #endif // LAVASCRIPT_DEBUG
}

void LSGC::print(std::ostream & os) const
{
    os << color::white() << "[[ alive GC objects ]]" << color::restore() << "\n\n";
    for (const LSObject * obj = gcListHead; obj != nullptr; obj = obj->gcNext)
    {
        obj->print(os);
        os << "\n";
    }
    os << color::white() << "----------------------------------" << color::restore() << "\n";
}

// ========================================================
// Debug printing helpers:
// ========================================================

static void dumpVariant(const Variant var, const int index, std::ostream & os)
{
    std::string valStr  = toString(var);
    std::string typeStr = toString(var.type);

    if (var.type == Variant::Type::LSStr)
    {
        valStr = unescapeString(valStr.c_str());
    }

    os << strPrintF("%s[ %3i ]%s %s" LAVASCRIPT_X64_PRINT_FMT "%s (%s%s%s) => %s\n",
                    color::cyan(), index, color::restore(), color::yellow(),
                    static_cast<UInt64>(var.value.asInteger), color::restore(),
                    color::red(), valStr.c_str(), color::restore(), typeStr.c_str());
}

void printCodeVector(const CodeVector & code, std::ostream & os)
{
    OpCode op = OpCode::NoOp;
    UInt32 operandIndex = 0;
    UInt32 instrIndex   = 0;

    os << color::white() << "[[ begin code vector dump ]]" << color::restore() << "\n";

    for (auto instr : code)
    {
        unpackInstruction(instr, op, operandIndex);

        os << color::cyan() << "[ " << std::setw(3) << instrIndex << " ] "
           << color::red() << toString(op) << color::restore() << " (operand=" << operandIndex << ")\n";

        ++instrIndex;
    }

    os << color::white() << "[[ listed " << code.size() << " instructions ]]" << color::restore() << "\n";
}

void printDataVector(const DataVector & data, std::ostream & os)
{
    os << color::white() << "[[ begin data vector dump ]]" << color::restore() << "\n";

    int index = 0;
    for (Variant var : data)
    {
        dumpVariant(var, index++, os);
    }

    os << color::white() << "[[ printed " << data.size() << " variants ]]" << color::restore() << "\n";
}

void LSVM::print(std::ostream & os) const
{
    os << color::white() << "[[ ---- LSVM state dump ---- ]]" << color::restore() << "\n";
    os << color::red() << "PC       = " << color::restore() << getProgramCounter() << "\n";
    os << color::red() << "Ret Addr = " << color::restore() << getReturnAddress()  << "\n";

    os << "\n";
    printDataVector(data, os);
    os << "\n";

    os << color::white() << "[[ LSVM stack ]]" << color::restore() << "\n";
    if (!stack.isEmpty())
    {
        int index = 0;
        auto s = stack.slice(0, stack.getCurrSize());
        for (const Variant * var = s.first(); var != nullptr; var = s.next())
        {
            dumpVariant(*var, index++, os);
        }
    }
    os << color::white() << "[[ printed " << stack.getCurrSize() << " variants ]]" << color::restore() << "\n";
    os << color::white() << "\n[[ ----------------------- ]]" << color::restore() << "\n";
}

std::string toString(const OpCode op)
{
    // OpCode to printable string (no colors added)
    static const std::string opCodeNames[]
    {
        "NOOP",
        "PROG_START",
        "PROG_END",
        "JMP",
        "JMP_IF_TRUE",
        "JMP_IF_FALSE",
        "JMP_RETURN",
        "TYPEOF",
        "TYPECAST",
        "CALL",
        "CALL_LOCAL",
        "NEW_VAR",
        "NEW_RANGE",
        "NEW_ARRAY",
        "NEW_OBJ",
        "ENUM_DYN_INIT",
        "FUNC_START",
        "FUNC_END",
        "FOR_LOOP_PREP",
        "FOR_LOOP_TEST",
        "FOR_LOOP_STEP",
        "MATCH_TEST",
        "MATCH_END",
        "ARRAY_SUBSCRIPT",
        "MEMBER_REF",
        "LOAD_RVR",
        "STORE_RVR",
        "LOAD_GLOBAL",
        "STORE_GLOBAL",
        "MEMBER_STORE_GLOBAL",
        "LOAD_LOCAL",
        "STORE_LOCAL",
        "MEMBER_STORE_LOCAL",
        "STORE_ARRAYSUB_LOCAL",
        "STORE_ARRAYSUB_GLOBAL",
        "STORE_ARRAYSUB_STK",
        "STORE_SETTYPE_LOCAL",
        "STORE_SETTYPE_GLOBAL",
        "CMP_NOT_EQUAL",
        "CMP_EQUAL",
        "CMP_GREATER_EQUAL",
        "CMP_GREATER",
        "CMP_LESS_EQUAL",
        "CMP_LESS",
        "LOGIC_OR",
        "LOGIC_AND",
        "SUB",
        "ADD",
        "MOD",
        "DIV",
        "MUL",
        "LOGIC_NOT",
        "NEGATE",
        "PLUS"
    };
    static_assert(arrayLength(opCodeNames) == UInt32(OpCode::Count),
                  "Keep this array in sync with the enum declaration!");

    return opCodeNames[UInt32(op)];
}

} // namespace lavascript {}
