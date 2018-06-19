/*
 * Copyright (c) 2018, Microsoft Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307 USA.
 *
 * Authors:
 *   Alex Ng 		<alexng@microsoft.com>
 *   Sean Spratt	<seansp@microsoft.com>
 */
#ifndef ELF_H
#define ELF_H

#include <stdint.h>

typedef int pid_t;

/*
 * Segment register layout in coredumps.
 */
struct user_regs_struct {
	uint64_t	r15;
	uint64_t	r14;
	uint64_t	r13;
	uint64_t	r12;
	uint64_t	bp;
	uint64_t	bx;
	uint64_t	r11;
	uint64_t	r10;
	uint64_t	r9;
	uint64_t	r8;
	uint64_t	ax;
	uint64_t	cx;
	uint64_t	dx;
	uint64_t	si;
	uint64_t	di;
	uint64_t	orig_ax;
	uint64_t	ip;
	uint64_t	cs;
	uint64_t	flags;
	uint64_t	sp;
	uint64_t	ss;
	uint64_t	fs_base;
	uint64_t	gs_base;
	uint64_t	ds;
	uint64_t	es;
	uint64_t	fs;
	uint64_t	gs;
};

typedef uint64_t elf_greg_t;
#define ELF_NGREG (sizeof(struct user_regs_struct)/sizeof(elf_greg_t))
typedef elf_greg_t elf_gregset_t[ELF_NGREG];

struct elf_siginfo {
	int si_signo;	/* signal number */
	int si_code;	/* extra code */
	int si_errno;	/* errno */
};

typedef struct timevalelf {
	int64_t tv_sec;
	int64_t tv_usec;
} timevalelf;

/*
 * Definitions to generate Intel SVR4-like core files.
 * These mostly have the same names as the SVR4 types with "elf_"
 * tacked on the front to prevent clashes with linux definitions,
 * and the typedef forms have been avoided.  This is mostly like
 * the SVR4 structure, but more Linuxy, with things that Linux does
 * not support and which gdb doesn't really use excluded.
 * Fields present but not used are marked with "XXX".
 */
#pragma pack(push,1)
struct elf_prstatus {
#if 0
	int64_t	pr_flags;	/* XXX Process flags */
	short	pr_why;		/* XXX Reason for process halt */
	short	pr_what;	/* XXX More detailed reason */
#endif
	struct elf_siginfo pr_info;	/* Info associated with signal */   // 12 bytes
	short pr_cursig;		/* Current signal */                    // 2 bytes
	uint64_t pr_sigpend;	/* Set of pending signals */        // 8 bytes
	uint64_t pr_sighold;	/* Set of held signals */           // 8 bytes
#if 0
	struct sigaltstack pr_altstack;	/* Alternate stack info */
	struct sigaction pr_action;	/* Signal action for current sig */
#endif
	pid_t pr_pid;   // 4 bytes
	pid_t pr_ppid;  // 4 bytes
	pid_t pr_pgrp;  // 4 bytes
	pid_t pr_sid;   // 4 bytes
	struct timevalelf pr_utime; 	/* User time */ 
	struct timevalelf pr_stime; 	/* System time */
	struct timevalelf pr_cutime;	/* Cumulative user time */
	struct timevalelf pr_cstime;	/* Cumulative system time */
#if 0
	int64_t	pr_instr;		/* Current instruction */
#endif
	elf_gregset_t pr_reg;	/* GP registers */
	int pr_fpvalid;		/* True if math co-processor being used.  */    // 4 bytes
};
#pragma pack(pop)

/* These constants define the different elf file types */
#define ET_NONE   0
#define ET_REL    1
#define ET_EXEC   2
#define ET_DYN    3
#define ET_CORE   4
#define ET_LOPROC 0xff00
#define ET_HIPROC 0xffff

#define EI_NIDENT 16

#define	EI_MAG0		0		/* e_ident[] indexes */
#define	EI_MAG1		1
#define	EI_MAG2		2
#define	EI_MAG3		3
#define	EI_CLASS	4
#define	EI_DATA		5
#define	EI_VERSION	6
#define	EI_OSABI	7
#define	EI_PAD		8

struct elf32_hdr {
	char e_ident[EI_NIDENT];
	uint16_t e_type;
	uint16_t e_machine;
	uint32_t e_version;
	uint32_t e_entry;
	uint32_t e_phoff;
	uint32_t e_shoff;
	uint32_t e_flags;
	uint16_t e_ehsize;
	uint16_t e_phentsize;
	uint16_t e_phnum;
	uint16_t e_shentsize;
	uint16_t e_shnum;
	uint16_t e_shstrndx;
};

struct elf64_hdr {
        char e_ident[EI_NIDENT];
        uint16_t e_type;
        uint16_t e_machine;
        uint32_t e_version;
        uint64_t e_entry;
        uint64_t e_phoff;
        uint64_t e_shoff;
        uint32_t e_flags;
        uint16_t e_ehsize;
        uint16_t e_phentsize;
        uint16_t e_phnum;
        uint16_t e_shentsize;
        uint16_t e_shnum;
        uint16_t e_shstrndx;
};

#define PF_R 0x4
#define PF_W 0x2
#define PF_X 0x1

struct elf32_phdr {
	uint32_t p_type;
	uint32_t p_offset;
	uint32_t p_vaddr;
	uint32_t p_paddr;
	uint32_t p_filesz;
	uint32_t p_memsz;
	uint32_t p_flags;
	uint32_t p_align;
};

struct elf64_phdr {
	uint32_t p_type;
	uint32_t p_flags;
	uint64_t p_offset;
	uint64_t p_vaddr;
	uint64_t p_paddr;
	uint64_t p_filesz;
	uint64_t p_memsz;
	uint64_t p_align;
};

#define	ELFMAG0		0x7f		/* EI_MAG */
#define	ELFMAG1		'E'
#define	ELFMAG2		'L'
#define	ELFMAG3		'F'
#define	ELFMAG		"\177ELF"
#define	SELFMAG		4

#define	ELFCLASSNONE	0		/* EI_CLASS */
#define	ELFCLASS32	1
#define	ELFCLASS64	2
#define	ELFCLASSNUM	3

#define ELFDATANONE	0		/* e_ident[EI_DATA] */
#define ELFDATA2LSB	1
#define ELFDATA2MSB	2

#define EV_NONE		0		/* e_version, EI_VERSION */
#define EV_CURRENT	1
#define EV_NUM		2

#define ELFOSABI_NONE	0
#define ELFOSABI_LINUX	3

struct elf32_note {
	uint32_t n_namesz;	/* Name size */
	uint32_t n_descsz;	/* Content size */
	uint32_t n_type;	/* Content type */
};

struct elf64_note {
	uint32_t n_namesz;
	uint32_t n_descsz;
	uint32_t n_type;
};

/* These constants define the various ELF target machines */
#define EM_NONE		0
#define EM_M32		1
#define EM_SPARC	2
#define EM_386		3
#define EM_68K		4
#define EM_88K		5
#define EM_486		6	/* Perhaps disused */
#define EM_860		7
#define EM_MIPS		8	/* MIPS R3000 (officially, big-endian only) */
				/* Next two are historical and binaries and
				 * modules of these types will be rejected by
				 * Linux.  */
#define EM_MIPS_RS3_LE	10	/* MIPS R3000 little-endian */
#define EM_MIPS_RS4_BE	10	/* MIPS R4000 big-endian */

#define EM_PARISC	15	/* HPPA */
#define EM_SPARC32PLUS	18	/* Sun's "v8plus" */
#define EM_PPC		20	/* PowerPC */
#define EM_PPC64	21	 /* PowerPC64 */
#define EM_SPU		23	/* Cell BE SPU */
#define EM_ARM		40	/* ARM 32 bit */
#define EM_SH		42	/* SuperH */
#define EM_SPARCV9	43	/* SPARC v9 64-bit */
#define EM_H8_300	46	/* Renesas H8/300 */
#define EM_IA_64	50	/* HP/Intel IA-64 */
#define EM_X86_64	62	/* AMD x86-64 */
#define EM_S390		22	/* IBM S/390 */
#define EM_CRIS		76	/* Axis Communications 32-bit embedded processor */
#define EM_M32R		88	/* Renesas M32R */
#define EM_MN10300	89	/* Panasonic/MEI MN10300, AM33 */
#define EM_OPENRISC     92     /* OpenRISC 32-bit embedded processor */
#define EM_BLACKFIN     106     /* ADI Blackfin Processor */
#define EM_ALTERA_NIOS2	113	/* Altera Nios II soft-core processor */
#define EM_TI_C6000	140	/* TI C6X DSPs */
#define EM_AARCH64	183	/* ARM 64 bit */
#define EM_TILEPRO	188	/* Tilera TILEPro */
#define EM_MICROBLAZE	189	/* Xilinx MicroBlaze */
#define EM_TILEGX	191	/* Tilera TILE-Gx */
#define EM_BPF		247	/* Linux BPF - in-kernel virtual machine */
#define EM_FRV		0x5441	/* Fujitsu FR-V */
#define EM_AVR32	0x18ad	/* Atmel AVR32 */

/*
 * Notes used in ET_CORE. Architectures export some of the arch register sets
 * using the corresponding note types via the PTRACE_GETREGSET and
 * PTRACE_SETREGSET requests.
 */
#define NT_PRSTATUS	1
#define NT_PRFPREG	2
#define NT_PRPSINFO	3
#define NT_TASKSTRUCT	4
#define NT_AUXV		6

/* These constants are for the segment types stored in the image headers */
#define PT_NULL    0
#define PT_LOAD    1
#define PT_DYNAMIC 2
#define PT_INTERP  3
#define PT_NOTE    4
#define PT_SHLIB   5
#define PT_PHDR    6
#define PT_TLS     7               /* Thread local storage segment */
#define PT_LOOS    0x60000000      /* OS-specific */
#define PT_HIOS    0x6fffffff      /* OS-specific */
#define PT_LOPROC  0x70000000
#define PT_HIPROC  0x7fffffff
#define PT_GNU_EH_FRAME		0x6474e550

#define PT_GNU_STACK	(PT_LOOS + 0x474e551)

#endif
