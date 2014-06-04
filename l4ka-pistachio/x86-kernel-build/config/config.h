/* Automatically generated, don't edit */
/* Generated on: ubuntu */
/* At: Wed, 04 Jun 2014 04:52:02 +0000 */
/* Linux version 3.11.0-22-generic (buildd@lamiak) (gcc version 4.6.3 (Ubuntu/Linaro 4.6.3-1ubuntu5) ) #38~precise1-Ubuntu SMP Fri May 16 20:50:12 UTC 2014 */

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
#define CONFIG_CPU_X86_I586 1
#undef  CONFIG_CPU_X86_I686
#undef  CONFIG_CPU_X86_P4
#undef  CONFIG_CPU_X86_K8
#undef  CONFIG_CPU_X86_C3
#undef  CONFIG_CPU_X86_SIMICS


/* Platform */
#define CONFIG_PLAT_PC99 1


/* Miscellaneous */
#undef  CONFIG_IOAPIC

#undef  CONFIG_SMP


/* Kernel */
#undef  CONFIG_EXPERIMENTAL
#undef  CONFIG_IPC_FASTPATH
#define CONFIG_DEBUG 1
#undef  CONFIG_DEBUG_SYMBOLS
#undef  CONFIG_SPIN_WHEELS
#undef  CONFIG_NEW_MDB
#undef  CONFIG_STATIC_TCBS
#undef  CONFIG_X86_SMALL_SPACES


/* Debugger */

/* Kernel Debugger Console */
#define CONFIG_KDB_CONS_COM 1
#define CONFIG_KDB_COMPORT 0x0
#define CONFIG_KDB_COMSPEED 115200
#undef  CONFIG_KDB_CONS_KBD
#define CONFIG_KDB_BOOT_CONS 0

#undef  CONFIG_KDB_DISAS
#undef  CONFIG_KDB_ON_STARTUP
#undef  CONFIG_KDB_BREAKIN
#undef  CONFIG_KDB_INPUT_HLT
#undef  CONFIG_KDB_NO_ASSERTS

/* Trace Settings */
#define CONFIG_VERBOSE_INIT 1
#define CONFIG_TRACEPOINTS 1
#define CONFIG_KMEM_TRACE 1
#define CONFIG_TRACEBUFFER 1



/* Code Generator Options */


/* Derived symbols */
#undef  CONFIG_HAVE_MEMORY_CONTROL
#define CONFIG_X86_PSE 1
#undef  CONFIG_BIGENDIAN
#undef  CONFIG_PPC_MMU_TLB
#undef  CONFIG_X86_SYSENTER
#undef  CONFIG_X86_PGE
#undef  CONFIG_X86_FXSR
#define CONFIG_IS_32BIT 1
#undef  CONFIG_X86_HTT
#undef  CONFIG_X86_PAT
#undef  CONFIG_PPC_BOOKE
#undef  CONFIG_IS_64BIT
#undef  CONFIG_MULTI_ARCHITECTURE
#undef  CONFIG_X86_EM64T
#undef  CONFIG_PPC_CACHE_L1_WRITETHROUGH
#undef  CONFIG_PPC_TLB_INV_LOCAL
#undef  CONFIG_PPC_CACHE_ICBI_LOCAL
#undef  CONFIG_X86_SMALL_SPACES_GLOBAL
#undef  CONFIG_X86_HVM
#undef  CONFIG_PPC_MMU_SEGMENTS
#define CONFIG_X86_TSC 1
/* That's all, folks! */
#define AUTOCONF_INCLUDED