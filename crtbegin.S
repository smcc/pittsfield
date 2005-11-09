.section	.ctors,"aw","progbits"
.align	4
.globl __CTOR_LIST__
__CTOR_LIST__:
	.long	0

.section	.dtors,"aw","progbits"
.align	4
.globl __DTOR_LIST__
__DTOR_LIST__:
	.long	0
	
