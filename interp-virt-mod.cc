#include "libc.h"

#define dSP register int *sp = stack_pointer
#define TOP (*sp)
int stack;
int *stack_pointer = &stack;
int *var;

struct op {
  op* op_next;
  int* op_val;

  virtual op* execute() = 0;
};

op* op;

struct op_null : public op {
  op *execute() {
    return op_next;
  }
};

struct op_decr : public op {
  op *execute() {
    dSP;
    TOP--;
    return op_next;
  }
};

struct op_check : public op {
  op *execute() {
    dSP;
    return (*sp ? op_next : 0);
  }
};

struct op_get : public op {
  op *execute() {
    dSP;
    TOP = *op_val;
    return op_next;
  }
};

struct op_set : public op {
  op *execute() {
    dSP;
    *op_val = TOP;
    return op_next;
  }
};

struct op * initops(void)
{
  struct op *opg = new op_get();
  opg->op_val = var;

  struct op *opd = new op_decr();

  struct op *ops = new op_set();
  ops->op_val = var;

  struct op *opc = new op_check();

  opc->op_next = opg;
  opg->op_next = opd;
  opd->op_next = ops;
  ops->op_next = opc;
 
  return opg;
}

void runops(void)
{
  while ( (op = op->execute()) ) ;
}

int main(int argc, char **argv)
{
  int var_value;
  
  var = &var_value;
  *var = 100000000;
  op = initops();
  runops();
  printf("virtual call\n");
  return 0;
}
