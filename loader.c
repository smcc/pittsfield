#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <libelf/libelf.h>
#include <assert.h>

int outside_esp;
int inside_esp;

int outside_ebp;
int inside_ebp;

int outside_ebx;
int outside_esi;
int outside_edi;

int call_in(void *addr) {
    int ret;
    asm("movl %0, %%eax" :: "r" (addr));
    asm("movl %esp, outside_esp");
    asm("movl %ebp, outside_ebp");
    asm("movl %ebx, outside_ebx");
    asm("movl %esi, outside_esi");
    asm("movl %edi, outside_edi");
    asm("movl inside_esp, %esp");
    asm("movl inside_ebp, %ebp");
    asm("call *%eax");
    asm("movl %esp, inside_esp");
    asm("movl %ebp, inside_ebp");
    asm("movl outside_esp, %esp");
    asm("movl outside_ebp, %ebp");
    asm("movl outside_ebx, %ebx");
    asm("movl outside_esi, %esi");
    asm("movl outside_edi, %edi");
    asm("movl %%eax, %0" : "=r" (ret));
    return ret;
}

int wrap_printf(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    return vprintf(fmt, ap);
}

int wrap_print_int(int i) {
    return printf("%d\n", i);
}

long long wrap_atoll(const char *nptr) {
    return atoll(nptr);
}

int wrap_putchar(int c) {
    return putchar(c);
}

extern unsigned long long
__udivdi3(unsigned long long a, unsigned long long b);
unsigned long long wrap___udivdi3(unsigned long long a, unsigned long long b) {
    return __udivdi3(a, b);
}

int (*stubs[])() = {
    (int (*)())&wrap_printf,
    (int (*)())&wrap_print_int,
    (int (*)())&wrap_atoll,
    (int (*)())&wrap_putchar,
    (int (*)())&wrap___udivdi3,
    0};

void install_stubs() {
    int addr = 0x10000021;
    int (**p)();
    for (p = stubs; *p; p++) {
	/* jmp is PC relative; specificially, relative to the PC after
	   the end of the jump instruction. */
	int offset = (int)*p - (addr + 4);
	*((int *)addr) = offset;
	addr += 0x10;
    }
}

int main(int argc, char **argv) {
    Elf *elf;
    Elf32_Ehdr *ehdr;
    Elf32_Phdr *phdr;
    Elf32_Shdr *shdr;
    Elf_Scn *scn;
    int fd;
    int ph_count;
    int data_size = 0;
    int data_len, data_offset;
    int code_len, code_offset;
    int ret;
    unsigned version;
    fd = open(argv[1], O_RDONLY);
    version = elf_version(EV_CURRENT);
    assert(version != EV_NONE);
    elf = elf_begin(fd, ELF_C_READ, 0);
    ehdr = elf32_getehdr(elf);
    ph_count = ehdr->e_phnum;
    assert(ph_count == 2);
    phdr = elf32_getphdr(elf);
    assert(phdr[0].p_type == PT_LOAD);
    assert(phdr[0].p_vaddr == 0x10000000);
    assert(phdr[0].p_paddr == 0x10000000);
    assert(phdr[0].p_memsz <= 0x01000000);
    assert(phdr[0].p_flags == (PF_R | PF_X));

    assert(phdr[1].p_type == PT_LOAD);
    assert(phdr[1].p_vaddr == 0x20000000);
    assert(phdr[1].p_paddr == 0x20000000);
    assert(phdr[1].p_memsz <= 0x01000000);
    assert(phdr[1].p_flags == (PF_R | PF_W));

    scn = 0;
    while ((scn = elf_nextscn(elf, scn)) != 0) {
	char *name;
	shdr = elf32_getshdr(scn);
	name = elf_strptr(elf, ehdr->e_shstrndx, shdr->sh_name);
	if (!name)
	    continue;
	/*printf("%s\n", name); */
	if (!strcmp(name, ".text")) {
	    void *ret;
	    assert(shdr->sh_size <= 0x01000000);
	    assert(shdr->sh_addr == 0x10000000);
	    assert(shdr->sh_type == SHT_PROGBITS);
	    assert(shdr->sh_flags == (SHF_ALLOC | SHF_EXECINSTR));
	    assert(!(shdr->sh_offset & 0xfff));
/* 	    mmap((void *)0x10000000, shdr->sh_size, PROT_EXEC | PROT_READ, */
/* 		 MAP_FIXED | MAP_SHARED, fd, shdr->sh_offset); */
	    ret = mmap((void *)0x10000000, shdr->sh_size,
		       PROT_EXEC | PROT_READ | PROT_WRITE,
		       MAP_FIXED | MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	    if (ret == MAP_FAILED)
		perror("mmap:");
	    code_len = shdr->sh_size;
	    code_offset = shdr->sh_offset;
	} else if (!strcmp(name, ".data")) {
	    assert(shdr->sh_size <= 0x01000000);
	    assert(shdr->sh_addr == 0x20000000);
	    assert(shdr->sh_type == SHT_PROGBITS);
	    assert(shdr->sh_flags == (SHF_WRITE | SHF_ALLOC));
	    assert(!(shdr->sh_offset & 0xfff));
	    data_len = shdr->sh_size;
	    data_offset = shdr->sh_offset;
	    data_size += shdr->sh_size;
	} else if (!strcmp(name, ".bss")) {
	    data_size += shdr->sh_size;
	}
    }
    assert(data_size <= 0x01000000);
    mmap((void *)0x20000000, 0x01000000, PROT_READ | PROT_WRITE,
	 MAP_FIXED | MAP_PRIVATE | MAP_NORESERVE | MAP_ANONYMOUS, 0, 0);
    elf_end(elf);
    lseek(fd, code_offset, SEEK_SET);
    read(fd, (void *)0x10000000, code_len);
    lseek(fd, data_offset, SEEK_SET);
    read(fd, (void *)0x20000000, data_len);
    close(fd);
    install_stubs();
    inside_ebp = inside_esp = 0x20fffffc;
    ret = call_in((void*)0x10000000);
    printf("%d\n", ret);
    return ret;
}
