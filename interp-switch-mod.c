#include "libc.h"

#define dSP register int *sp = stack_pointer
#define TOP (*sp)
int stack;
int *stack_pointer = &stack;
int *var;

#define INSN_NULL 0
#define INSN_DECR 1
#define INSN_CHECK 2
#define INSN_JUMP 3
#define INSN_GET 4
#define INSN_SET 5

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
  runinsns(initinsns());
  printf("%s", "switch\n");

  return 0;
}
