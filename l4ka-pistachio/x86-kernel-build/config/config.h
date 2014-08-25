/* Automatically generated, don't edit */
/* Generated on: ubuntu */
/* At: Mon, 25 Aug 2014 18:42:32 +0000 */
/* Linux version 3.13.0-24-generic (buildd@roseapple) (gcc version 4.8.2 (Ubuntu 4.8.2-19ubuntu1) ) #46-Ubuntu SMP Thu Apr 10 19:08:14 UTC 2014 */

/* Pistachio Kernel Configuration System */

/* Hardware */

/* Basic Architecture */
#define CONFIG_ARCH_X86 1
#undef  CONFIG_ARCH_POWERPC
#undef  CONFIG_ARCH_POWERPC64


/* X86 Processor Architecture */
#define CONFIG_SUBARCH_X32 1
#undef  CONFIG_SUBARCH_X64


/* Processor Type */
#undef  CONFIG_CPU_X86_I486
#undef  CONFIG_CPU_X86_I586
#undef  CONFIG_CPU_X86_I686
#define CONFIG_CPU_X86_P4 1
#undef  CONFIG_CPU_X86_K8
#undef  CONFIG_CPU_X86_C3
#undef  CONFIG_CPU_X86_SIMICS


/* Platform */
#define CONFIG_PLAT_PC99 1


/* Miscellaneous */
#define CONFIG_IOAPIC 1
#define CONFIG_MAX_IOAPICS 2
#define CONFIG_APIC_TIMER_TICK 1000

#define CONFIG_SMP 1
#define CONFIG_SMP_MAX_PROCS 40
#undef  CONFIG_SMP_IDLE_POLL


/* Kernel */
#undef  CONFIG_EXPERIMENTAL
#define CONFIG_IPC_FASTPATH 1
#undef  CONFIG_DEBUG
#undef  CONFIG_DEBUG_SYMBOLS
#undef  CONFIG_PERFMON
#define CONFIG_SPIN_WHEELS 1
#define CONFIG_NEW_MDB 1
#define CONFIG_STATIC_TCBS 1
#undef  CONFIG_X86_SMALL_SPACES
#undef  CONFIG_X86_IO_FLEXPAGES


/* Debugger */

/* Kernel Debugger Console */
#undef  CONFIG_KDB_CONS_COM

#undef  CONFIG_KDB_BREAKIN_BREAK
#undef  CONFIG_KDB_BREAKIN_ESCAPE


/* Code Generator Options */


/* Derived symbols */
#undef  CONFIG_HAVE_MEMORY_CONTROL
#define CONFIG_X86_PSE 1
#undef  CONFIG_BIGENDIAN
#undef  CONFIG_PPC_MMU_TLB
#define CONFIG_X86_SYSENTER 1
#define CONFIG_X86_PGE 1
#define CONFIG_X86_FXSR 1
#define CONFIG_IS_32BIT 1
#define CONFIG_X86_HTT 1
#define CONFIG_X86_PAT 1
#undef  CONFIG_PPC_BOOKE
#undef  CONFIG_IS_64BIT
#undef  CONFIG_MULTI_ARCHITECTURE
#undef  CONFIG_X86_EM64T
#undef  CONFIG_PPC_CACHE_L1_WRITETHROUGH
#undef  CONFIG_PPC_TLB_INV_LOCAL
#undef  CONFIG_PPC_CACHE_ICBI_LOCAL
#undef  CONFIG_X86_SMALL_SPACES_GLOBAL
#define CONFIG_X86_HVM 1
#undef  CONFIG_PPC_MMU_SEGMENTS
#define CONFIG_X86_TSC 1
/* That's all, folks! */
#define AUTOCONF_INCLUDED
