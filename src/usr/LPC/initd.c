# include "code.h"
# include "compiler.h"

/*
 * initialize LPC compiler
 */
static void create(void)
{
    compile_object(OBJECT_PATH(LPCStmtBlock));
    compile_object(OBJECT_PATH(LPCStmtBreak));
    compile_object(OBJECT_PATH(LPCStmtCond));
    compile_object(OBJECT_PATH(LPCStmtContinue));
    compile_object(OBJECT_PATH(LPCStmtDoWhile));
    compile_object(OBJECT_PATH(LPCExp1));
    compile_object(OBJECT_PATH(LPCExp2));
    compile_object(OBJECT_PATH(LPCExp3));
    compile_object(OBJECT_PATH(LPCExpArray));
    compile_object(OBJECT_PATH(LPCExpCast));
    compile_object(OBJECT_PATH(LPCExpGlobalVar));
    compile_object(OBJECT_PATH(LPCExpInhFuncall));
    compile_object(OBJECT_PATH(LPCExpMapping));
    compile_object(OBJECT_PATH(LPCExpVar));
    compile_object(OBJECT_PATH(LPCStmtFor));
    compile_object(OBJECT_PATH(LPCFunction));
    compile_object(OBJECT_PATH(LPCStmtGoto));
    compile_object(OBJECT_PATH(LPCInherit));
    compile_object(OBJECT_PATH(LPCProgram));
    compile_object(OBJECT_PATH(LPCStmtReturn));
    compile_object(OBJECT_PATH(LPCStmtReturnExp));
    compile_object(OBJECT_PATH(LPCStmtRlimits));
    compile_object(OBJECT_PATH(LPCStmtCase));
    compile_object(OBJECT_PATH(LPCStmtCaseRange));
    compile_object(OBJECT_PATH(LPCStmtDefault));
    compile_object(OBJECT_PATH(LPCStmtExp));
    compile_object(OBJECT_PATH(LPCStmtLabel));
    compile_object(OBJECT_PATH(LPCStmtSwitch));
    compile_object(OBJECT_PATH(LPCType));
    compile_object(OBJECT_PATH(LPCVariable));

    compile_object(LPC_COMPILER);
}
