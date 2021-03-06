/*********************************************************************
 *
 * Copyright (C) 1999-2010,  Karlsruhe University
 * Copyright (C) 2008-2009,  Volkmar Uhlig, IBM Corporation
 *
 * File path:     bench/pingpong/pingpong.cc
 * Description:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id$
 *
 ********************************************************************/
#include <config.h>
#include <l4/kip.h>
#include <l4/ipc.h>
#include <l4/schedule.h>
#include <l4/kdebug.h>
#include <l4io.h>
#include <l4/arch.h>
#include <l4/sigma0.h>

#if defined(L4_ARCH_POWERPC64)
extern long _start_pager;
extern long _start_ping_thread;
extern long _start_pong_thread;
#define START_ADDR(func)    ((L4_Word_t) &_start_##func)
#else
#define START_ADDR(func)    ((L4_Word_t) func)
#endif

#define DEBUG 0

#define IPC_ARCH_OPTIMIZATION

//#define PERFMON
#define PING_PONG_PRIO      128
#define SMALLSPACE_SIZE     16

//memory allocation
#define KB(x) (x*1024)
#define MB(x) (x*1024*1024)
#define GB(x) (x*1024*1024*1024)

int MIGRATE = 0;
int INTER_AS = 1;
int SMALL_AS = 0;
int LIPC = 0;
int MEMORY = 0;
int ALL = 1;
int PRINT_TABLE = 0;

int hsched;
int NumProcessors;

L4_ThreadId_t s0tid, roottid, pager_tid, ping_tid, pong_tid;
L4_KernelInterfacePage_t *kip;

L4_Word_t ping_stack[2048] __attribute__ ((aligned (16)));
L4_Word_t pong_stack[2048] __attribute__ ((aligned (16)));
L4_Word_t pager_stack[2048] __attribute__ ((aligned (16)));

L4_Fpage_t kip_area, utcb_area;
L4_Word_t utcb_size;


#define UTCB(x) ((void*)(L4_Address(utcb_area) + (x) * utcb_size))
#define NOUTCB  ((void*)-1)

static L4_Word_t page_bits;

extern "C" void memset (char *p, char c, int size)
{
    for (; size--;)
        *(p++) = c;
}


void ping_thread (void);
void pong_thread (void);
void menu_options(char);
void alloc_mem (void);

/*
 * Architecture dependent function stubs.
 */

#if defined(L4_ARCH_IA32)
#include "ia32.h"
#elif defined(L4_ARCH_POWERPC)
#include "powerpc.h"
#elif defined(L4_ARCH_POWERPC64)
#include "powerpc64.h"
#elif defined(L4_ARCH_AMD64)
#include "amd64.h"
#endif
#include <l4/tracebuffer.h>

#define debug_printf(x...) L4_Tbuf_RecordEvent (1, x)

static inline void rdpmc (int no, L4_Word64_t *res)
{
#if defined(L4_ARCH_AMD64) || defined(L4_ARCH_IA32)
    L4_Word32_t __eax, __edx, dummy;

    __asm__ __volatile__ (
#if defined(PERFMON)
        "rdpmc  \n\t"
#else
        ""
#endif
        : "=a"(__eax), "=d"(__edx), "=c"(dummy)
        : "c"(no)
        : "memory");

    *res = ( (((L4_Word64_t) __edx) << 32) | ( (L4_Word64_t) __eax));
#endif /* defined(L4_ARCH_AMD64) || defined(L4_ARCH_X86) */
}



/*
 * Default arch dependent definitions and function stubs.
 */

#if !defined(UTCB_ADDRESS)
#define UTCB_ADDRESS    (0x80000000UL)
#endif

#if !defined(HAVE_HANDLE_ARCH_PAGEFAULT)
L4_INLINE L4_Fpage_t handle_arch_pagefault (L4_MsgTag_t tag, L4_Word_t faddr, L4_Word_t fip, L4_Word_t log2size)
{
    return L4_FpageLog2 (faddr, log2size);
}
#endif

#if !defined(HAVE_READ_CYCLES)
L4_INLINE L4_Word64_t read_cycles (void)
{
    return 0;
}
#endif

#if !defined(HAVE_READ_INSTRS)
L4_INLINE L4_Word_t read_instrs (void)
{
    return 0;
}
#endif

#if !defined(HAVE_ARCH_IPC)
L4_INLINE L4_Word_t pingpong_ipc (L4_ThreadId_t dest, L4_Word_t untyped)
{
    L4_MsgTag_t tag;
    tag = L4_Niltag;
    tag.X.u = untyped;
    L4_Set_MsgTag (tag);
    return L4_UntypedWords (L4_Call (dest));
}
#endif

#if !defined(HAVE_ARCH_LIPC)
L4_INLINE L4_Word_t pingpong_lipc (L4_ThreadId_t dest, L4_Word_t untyped)
{
    L4_MsgTag_t tag (L4_Niltag);
    tag.X.u = untyped;
    L4_Set_MsgTag (tag);
    return L4_UntypedWords (L4_Lcall (dest));
}
#endif


void pong_thread (void)
{
    L4_Word_t untyped = 0;

    if (LIPC)
    {
        L4_ThreadId_t ping_ltid = L4_LocalId (ping_tid);
        for (;;)
            untyped = pingpong_lipc (ping_ltid, untyped);
    }
    else
        for (;;)
        {
            debug_printf("pong ipc\n");
            untyped = pingpong_ipc (ping_tid, untyped);
        }
}


#define ROUNDS (100)
#define FACTOR      (8)
#define MRSTEPPING  1
#define ITERATIONS (100000)

void ping_thread (void)
{
    int j = 0, untyped = 0;
    L4_Word64_t avgcycles = 0;
    L4_Word64_t avginstrs = 0;
    L4_Word64_t avgus = 0;

    L4_Word64_t cycles1, cycles2;
    L4_Clock_t usec1, usec2;
    L4_Word_t instrs1, instrs2;

    // Wait for pong thread to come up
    L4_Receive (pong_tid);
    L4_ThreadId_t pong_ltid = L4_LocalId (pong_tid);
    if (MEMORY)
    {
        cycles1 = read_cycles ();
        usec1 = L4_SystemClock ();
        instrs1 = read_instrs ();

        alloc_mem();

        cycles2 = read_cycles ();
        usec2 = L4_SystemClock ();
        instrs2 = read_instrs ();

        avgcycles += cycles2 - cycles1;
        avgus += (usec2 - usec1).raw;
        avginstrs += instrs2 - instrs1;
    }
    else
    {
        printf("Benchmarking %s IPC...\n",
               MIGRATE  ? "XCPU" :
               INTER_AS ? "Inter-AS" : "Intra-AS");
        for (; j < ITERATIONS; j++)
        {

            L4_Word_t i = ROUNDS;
            i /= FACTOR;
            i *= FACTOR;


                cycles1 = read_cycles ();
                usec1 = L4_SystemClock ();
                instrs1 = read_instrs ();
            if (LIPC)
            {
                pingpong_lipc (pong_tid, untyped);
                
            }
            else
            {
                debug_printf( "ping ipc\n");
                pingpong_ipc (pong_tid, untyped);
            }
	
		cycles2 = read_cycles ();
                usec2 = L4_SystemClock ();
                instrs2 = read_instrs ();


            avgcycles += cycles2 - cycles1;
            avgus += (usec2 - usec1).raw;
            avginstrs += instrs2 - instrs1;
            if (DEBUG && j <= 20)
            {
                printf ("IPC : %lu cycles, %lu us, %lu instrs\n",
                        ((unsigned long)(cycles2 - cycles1)),
                        ((unsigned long)((usec2 - usec1).raw)) ,
                        ((unsigned long)(instrs2 - instrs1)));
            }

        }
    }
    printf ("Average IPC : %lu cycles, %lu us, %lu instrs\n",
            ((avgcycles)) / (ITERATIONS),
            ((avgus)) / (ITERATIONS),
            ((avginstrs)) / (ITERATIONS));

    // Tell master that we're finished
    L4_Set_MsgTag (L4_Niltag);
    L4_Send (roottid);

    for (;;)
        L4_Sleep (L4_Never);

    /* NOTREACHED */

}





static void send_startup_ipc (L4_ThreadId_t tid, L4_Word_t ip, L4_Word_t sp)
{
#if 0
    printf ("sending startup message to %lx, (ip=%lx, sp=%lx)\n",
            (long) tid.raw, (long) ip, (long) sp);
#endif
    L4_Msg_t msg;
    L4_Clear (&msg);
    L4_Append (&msg, ip);
    L4_Append (&msg, sp);
    L4_Load (&msg);
    L4_Send (tid);
}

void pager (void)
{

    L4_ThreadId_t tid;
    L4_MsgTag_t tag;
    L4_Msg_t msg;

    for (;;)
    {
        tag = L4_Wait (&tid);

        for (;;)
        {
            L4_Store (tag, &msg);

            debug_printf( "Pager got msg from %p\n", (L4_Word_t) tid.raw);
            debug_printf( "\tmsg  (%p, %p, %p, %p)\n",
                          (L4_Word_t) tag.raw,
                          (L4_Word_t) L4_Get (&msg, 0),
                          (L4_Word_t) L4_Get (&msg, 1),
                          (L4_Word_t) L4_Get (&msg, 2));


            if (L4_GlobalId (tid) == roottid)
            {
                // Startup notification, start ping and pong thread
                send_startup_ipc (ping_tid, L4_Get (&msg, 0),
                                  (L4_Word_t) ping_stack +
                                  sizeof (ping_stack) - 32);
                send_startup_ipc (pong_tid, L4_Get (&msg, 1),
                                  (L4_Word_t) pong_stack +
                                  sizeof (pong_stack) - 32);
                break;
            }

            if (L4_UntypedWords (tag) != 2 || L4_TypedWords (tag) != 0 ||
                    !L4_IpcSucceeded (tag))
            {
                L4_KDB_Enter ("malformed pf");
                printf ("pingpong: malformed pagefault IPC from %p (tag=%p)\n",
                        (void *) tid.raw, (void *) tag.raw);
                L4_KDB_Enter ("malformed pf");
                break;
            }

            L4_Word_t faddr = L4_Get (&msg, 0);
            L4_Word_t fip   = L4_Get (&msg, 1);

            L4_Fpage_t fpage = handle_arch_pagefault (tag, faddr, fip, page_bits);

            L4_Clear (&msg);
            L4_Append (&msg, L4_MapItem (fpage + L4_FullyAccessible, faddr));
            L4_Load (&msg);
            tag = L4_ReplyWait (tid, &tid);
        }
    }
}

int main (void)
{
    L4_Word_t control;
    L4_Msg_t msg;

    kip = (L4_KernelInterfacePage_t *) L4_KernelInterface ();
    NumProcessors = L4_NumProcessors(kip);
    int nextoption = 0;
    char c;
    printf("\nNum Processors: %d\n\n", NumProcessors);
    /* Me and sigma0 */
    roottid = L4_Myself();
    s0tid = L4_GlobalId (kip->ThreadInfo.X.UserBase, 1);

    /* Find smallest supported page size. There's better at least one
     * bit set. */
    for (page_bits = 0;
            !((1 << page_bits) & L4_PageSizeMask(kip));
            page_bits++);

    // Size for one UTCB
    utcb_size = L4_UtcbSize (kip);

#if defined(KIP_ADDRESS)
    // Put kip in different location (e.g., to allow for small
    // spaces).
    kip_area = L4_FpageLog2 (KIP_ADDRESS, L4_KipAreaSizeLog2 (kip));
#else
    // Put the kip at the same location in all AS to make sure we can
    // reuse the syscall jump table.
    kip_area = L4_FpageLog2 ((L4_Word_t) kip, L4_KipAreaSizeLog2 (kip));
#endif

    // Touch the memory to make sure we never get pagefaults
    extern L4_Word_t _end, _start;
    for (L4_Word_t *x = (&_start); x < &_end; x++)
    {
        volatile L4_Word_t q;
        q = *(volatile L4_Word_t *) x;
    }

    // We need a maximum of two threads per task
    utcb_area = L4_FpageLog2 ((L4_Word_t) UTCB_ADDRESS,
                              L4_UtcbAreaSizeLog2 (kip) + 1);

    debug_printf ("kip_area = %lx, utcb_area = %lx, utcb_size = %lx\n",
                  kip_area.raw, utcb_area.raw, utcb_size);


    // Create pager
    pager_tid = L4_GlobalId (L4_ThreadNo (roottid) + 1, 2);
    ping_tid = L4_GlobalId (L4_ThreadNo (roottid) + 2, 2);
    pong_tid = L4_GlobalId (L4_ThreadNo (roottid) + 3, 2);

    L4_ThreadId_t scheduler_tid[2] = { roottid, roottid };

    hsched  = L4_HasFeature("hscheduling");


    // VU: calculate UTCB address -- this has to be revised
    L4_Word_t pager_utcb = L4_MyLocalId().raw;
    pager_utcb = (pager_utcb & ~(utcb_size - 1)) + utcb_size;
    debug_printf("local id = %lx, pager UTCB = %lx\n", L4_MyLocalId().raw,
                 pager_utcb);

    L4_ThreadControl (pager_tid, L4_Myself (), scheduler_tid[0], L4_Myself (), (void *)pager_utcb);


    L4_Start (pager_tid, (L4_Word_t) pager_stack + sizeof(pager_stack) - 32,
              START_ADDR (pager));

    if (hsched)
    {
        L4_Word_t sched_control = 0, old_sched_control = 0, result = 0;
        L4_Word_t prio = 50, stride = 63;;

        //New Subqueue below me

        sched_control = 1;

        result = L4_HS_Schedule (pager_tid, sched_control, L4_Myself(), prio, stride, &old_sched_control);

        //Restride
        sched_control = 16;
        old_sched_control = 0;
        stride = 500;
        result = L4_HS_Schedule (L4_Myself(), sched_control, pager_tid, 0, stride, &old_sched_control);
    }



    const char *str = "Pingpong started %x\n";
    printf(str, L4_Myself());
    debug_printf( str, L4_Myself().raw);

    for (;;)
    {
        bool printmenu = true;

        if (ALL)
        {
            nextoption = 0;
        }

        for (;;)
        {
            if ((printmenu && DEBUG) )
            {
                //printf("\nL4_NumProcessors: %d\n", L4_NumProcessors(kip));
                //printf("L4_NumMemoryDescriptors: %d\n", L4_NumMemoryDescriptors(kip));
                printf ("\nPlease select ipc type:\n");
                printf ("\r\n"
                        "1: INTER-AS\r\n"
                        "2: INTRA-AS (IPC)\r\n"
                        "3: INTRA-AS (LIPC)\r\n"
                        "4: XCPU\r\n"
#ifdef HAVE_ARCH_SPECIFIC
                        "a: architecture specific\r\n"
#endif
                        "s: print SQLite table\r\n"
                       );
            }
            printmenu = false;


            if (nextoption < 0)
            {
                c = getc ();
            }
            else
            {
                //runs through all image options until the end. then goes back to user input
                switch (nextoption)
                {
                case 0:
                    c = '1';
                    nextoption++;
                    break;
                case 1:
                    c = '2';
                    nextoption ++;
                    break;
                case 2:
                    c = '3';
                    nextoption ++;
                    break;
                case 3:
                    c = '4';
                    nextoption = -1;
                    break;
                }
            }
            menu_options(c);
            break;


        }
        if (ALL)
        {
            printf("Printing all:\n");
        }
        else
        {
            if (INTER_AS)
            {
                L4_ThreadControl (ping_tid, ping_tid, scheduler_tid[0],
                                  L4_nilthread, UTCB(0));
                L4_ThreadControl (pong_tid, pong_tid, scheduler_tid[0],
                                  L4_nilthread, UTCB(1));
                L4_SpaceControl (ping_tid, 0, kip_area, utcb_area, L4_nilthread,
                                 &control);
                L4_SpaceControl (pong_tid, 0, kip_area, utcb_area, L4_nilthread,
                                 &control);
                L4_ThreadControl (ping_tid, ping_tid, scheduler_tid[0], pager_tid,
                                  NOUTCB);
                L4_ThreadControl (pong_tid, pong_tid, scheduler_tid[0], pager_tid,
                                  NOUTCB);

#if defined(L4_ARCH_IA32)
                if (SMALL_AS)
                {
                    L4_SpaceControl (ping_tid, (1UL << 31) |
                                     L4_SmallSpace (0, SMALLSPACE_SIZE),
                                     L4_Nilpage, L4_Nilpage, L4_nilthread,
                                     &control);
                    L4_SpaceControl (pong_tid, (1UL << 31) |
                                     L4_SmallSpace (SMALLSPACE_SIZE,
                                                    SMALLSPACE_SIZE),
                                     L4_Nilpage, L4_Nilpage, L4_nilthread,
                                     &control);
                }
#endif /* defined(L4_ARCH_IA32) */
            }
            else
            {
                // Intra-as -- put both into the same space
                L4_ThreadControl (ping_tid, ping_tid, scheduler_tid[0], L4_nilthread,
                                  UTCB(0));
                L4_SpaceControl (ping_tid, 0, kip_area, utcb_area, L4_nilthread,
                                 &control);
                L4_ThreadControl (ping_tid, ping_tid, scheduler_tid[0], pager_tid,
                                  NOUTCB);
                L4_ThreadControl (pong_tid, ping_tid, scheduler_tid[0], pager_tid,
                                  UTCB(1));
            }

#if defined(L4_ARCH_IA32)
            if (hsched)
            {
                L4_Set_Logid (ping_tid, 4);
                L4_Set_Logid (pong_tid, 5);
            }
#endif

            if (MIGRATE)
            {
                if (2 >= NumProcessors)
                {
                    L4_Set_ProcessorNo (pong_tid, (L4_ProcessorNo() + 1) % 2);
                    L4_ThreadControl (pong_tid, pong_tid, scheduler_tid[1], pager_tid, NOUTCB);
                    // Send message to notify pager to startup both threads
                    L4_Clear (&msg);
                    L4_Append (&msg, START_ADDR (ping_thread));
                    L4_Append (&msg, START_ADDR (pong_thread));
                    L4_Load (&msg);
                    L4_Send (pager_tid);

                    L4_Receive (ping_tid);

                    // Kill both threads
                    L4_ThreadControl (ping_tid, L4_nilthread, L4_nilthread,
                                      L4_nilthread, NOUTCB);
                    L4_ThreadControl (pong_tid, L4_nilthread, L4_nilthread,
                                      L4_nilthread, NOUTCB);
                }
                else
                {
                    for (int i = 2; i < NumProcessors; i++)
                    {
                        printf("Migrating from processor %d to %d\n", 1, i);
                        // MIGRATE -- two spaces. loop through each core against single core.
                        L4_ThreadControl (ping_tid, ping_tid, scheduler_tid[0], L4_nilthread,
                                          UTCB(0));
                        L4_SpaceControl (ping_tid, 0, kip_area, utcb_area, L4_nilthread,
                                         &control);
                        L4_ThreadControl (ping_tid, ping_tid, scheduler_tid[0], pager_tid,
                                          NOUTCB);
                        L4_Set_ProcessorNo (pong_tid, (i) % NumProcessors);
                        L4_ThreadControl (pong_tid, pong_tid, scheduler_tid[1], pager_tid, NOUTCB);


                        // Send message to notify pager to startup both threads
                        L4_Clear (&msg);
                        L4_Append (&msg, START_ADDR (ping_thread));
                        L4_Append (&msg, START_ADDR (pong_thread));
                        L4_Load (&msg);
                        L4_Send (pager_tid);

                        L4_Receive (ping_tid);

                        // Kill both threads
                        L4_ThreadControl (ping_tid, L4_nilthread, L4_nilthread,
                                          L4_nilthread, NOUTCB);
                        L4_ThreadControl (pong_tid, L4_nilthread, L4_nilthread,
                                          L4_nilthread, NOUTCB);
                    }
                }
            }
            else
            {
                // Send message to notify pager to startup both threads
                L4_Clear (&msg);
                L4_Append (&msg, START_ADDR (ping_thread));
                L4_Append (&msg, START_ADDR (pong_thread));
                L4_Load (&msg);
                L4_Send (pager_tid);

                L4_Receive (ping_tid);

                // Kill both threads
                L4_ThreadControl (ping_tid, L4_nilthread, L4_nilthread,
                                  L4_nilthread, NOUTCB);
                L4_ThreadControl (pong_tid, L4_nilthread, L4_nilthread,
                                  L4_nilthread, NOUTCB);
            }

        }
    }

    for (;;)
        L4_KDB_Enter ("EOW");
}



void alloc_mem ()
{

    L4_Word_t tsize = 0;

    for (L4_Word_t s = sizeof (L4_Word_t) * 8 - 1; s >= 10; s--)
    {
        L4_Fpage_t f;
        int n = -1;

        do
        {
            f = L4_Sigma0_GetAny (L4_nilthread, s, L4_CompleteAddressSpace);
            n++;
        }
        while (! L4_IsNilFpage (f));

        L4_Word_t size = n * (1UL << s);
        tsize += size;

        if (n)
            printf ("Allocated %d pages of %3ld%cB (log2size %2ld) [%ld%cB]\n",
                    n,
                    s >= 30 ? 1UL << (s - 30) :
                    s >= 20 ? 1UL << (s - 20) : 1UL << (s - 10),
                    s >= 30 ? 'G' : s >= 20 ? 'M' : 'K',
                    s,
                    size >= GB(1) ? size / GB(1) :
                    size >= MB(1) ? size / MB(1) : size / KB(1),
                    size >= GB(1) ? 'G' : size >= MB(1) ? 'M' : 'K');
    }

    // Avoid using floating point
    printf ("\nTotal memory: %ld.%ldGB | %ld.%ldMB | %ldKB\n",
            tsize / GB(1), ((tsize * 100) / GB(1)) % 100,
            tsize / MB(1), ((tsize * 100) / MB(1)) % 100,
            tsize / KB(1));

}

void menu_options(char c)
{
    if (c == '1')
    {
        INTER_AS = 1;
        MIGRATE = 0;
        SMALL_AS = 0;
        LIPC = 0;
        MEMORY = 0;
        ALL = 0;
    }
    if (c == '2')
    {
        INTER_AS = 0;
        MIGRATE = 0;
        SMALL_AS = 0;
        LIPC = 0;
        MEMORY = 0;
        ALL = 0;
    }
    if (c == '3')
    {
        INTER_AS = 0;
        MIGRATE = 0;
        SMALL_AS = 0;
        LIPC = 1;
        MEMORY = 0;
        ALL = 0;
    }
    if (c == '4')
    {
        INTER_AS = 0;
        MIGRATE = 1;
        SMALL_AS = 0;
        LIPC = 0;
        MEMORY = 0;
        ALL = 0;
    }
    if (c == '5')
    {
        INTER_AS = 0;
        MIGRATE = 1;
        SMALL_AS = 0;
        LIPC = 0;
        MEMORY = 1;
        ALL = 0;
    }

    if (c == 'a')
    {
        INTER_AS = 0;
        MIGRATE = 0;
        SMALL_AS = 0;
        LIPC = 0;
        MEMORY = 0;
        ALL = 1;
    }


}
