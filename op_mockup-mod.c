#include "libc.h"

#define dSP register int *sp = stack_pointer
#define TOP (*sp)
int stack;
int *stack_pointer = &stack;
int *var;

#define OP_NULL 0
#define OP_DECR 1
#define OP_CHECK 2
#define OP_GET 3
#define OP_SET 4

typedef struct op OP;

struct op {
  OP* op_next;
  OP* (*op_ppaddr)();
  unsigned short op_type;
  int* op_val;
};

OP* op;
OP all_ops[10];

OP* pp_null(void)
{
  return op->op_next;
}

OP* pp_decr(void)
{
  dSP;
  TOP--;
  return op->op_next;
}

OP* pp_check(void)
{
  dSP;
  return (*sp ? op->op_next : 0);
}

OP* pp_get(void)
{
  dSP;
  TOP = *(op->op_val);
  return op->op_next;
}

OP* pp_set(void)
{
  dSP;
  *(op->op_val) = TOP;
  return op->op_next;
}

OP* initops(void)
{
  OP *opd, *opc, *opg, *ops;

  opg = &all_ops[0];
  opg->op_ppaddr = pp_get;
  opg->op_type = OP_GET;
  opg->op_val = var;

  opd = &all_ops[1];
  opd->op_ppaddr = pp_decr;
  opd->op_type = OP_DECR;

  ops = &all_ops[2];
  ops->op_ppaddr = pp_set;
  ops->op_type = OP_SET;
  ops->op_val = var;

  opc = &all_ops[3];
  opc->op_ppaddr = pp_check;
  opc->op_type = OP_CHECK;
  opc->op_next = opg;

  opg->op_next = opd;
  opd->op_next = ops;
  ops->op_next = opc;
 
  return opg;
}

void runops(void)
{
  while ( (op = (*op->op_ppaddr)()) ) ;
}

#define INSN_NULL 0
#define INSN_DECR 1
#define INSN_CHECK 2
#define INSN_JUMP 3
#define INSN_GET 4
#define INSN_SET 5

/* enum insn_type {INSN_NULL, INSN_DECR, INSN_CHECK, INSN_JUMP, INSN_GET,
		INSN_SET}; */

typedef struct insn INSN;

struct insn {
  unsigned short in_type;
  unsigned char in_flags;
  unsigned char in_private;
  void *in_x1;
};

INSN insns[10];

INSN* initinsns(void)
{
  INSN* code = insns;
  code[0].in_type = INSN_GET; code[0].in_x1 = var;
  code[1].in_type = INSN_DECR;
  code[2].in_type = INSN_SET; code[2].in_x1 = var;
  code[3].in_type = INSN_CHECK;
  code[4].in_type = INSN_JUMP; code[4].in_x1 = &code[0] - 1;
  return code;
}

void runinsns(INSN *code)
{
  register int *sp = stack_pointer;
  register INSN *p = code;
  for (;;)
    {
      switch (p->in_type)
	{
	case INSN_NULL:
	  break;
	case INSN_DECR:
	  TOP--;
	  break;
	case INSN_CHECK:
	  if (!TOP) return;
	  break;
	case INSN_JUMP:
	  p = (INSN *)(p->in_x1);
	  break;
	case INSN_GET:
	  TOP = *(int *)(p->in_x1);
	  break;
	case INSN_SET:
	  *(int *)(p->in_x1) = TOP;
	  break;
	}
      p++;
    }
}

int main()
{
  int var_value;
  
  var = &var_value;
  *var = 100000000;
  op = initops();
  runops();
  printf("%s", "pointer\n");

  *var = 100000000;
  runinsns(initinsns());
  printf("%s", "switch\n");

  return 0;
}
