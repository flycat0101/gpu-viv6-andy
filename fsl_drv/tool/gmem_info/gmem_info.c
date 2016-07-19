/*
 *  Copyright (C) 2013 Freescale Semiconductor, Inc.
 *  All Rights Reserved.
 *
 *  The following programs are the sole property of Freescale Semiconductor Inc.,
 *  and contain its proprietary and confidential information.
 *
 */
/*
 *	gmem_info.c
 *	The sampe test code is implemented to trace gpu memory info.
 *	History :
 *	Date(y.m.d)        Author            Version        Description
 *	2013-09-05         Li Xianzhong      0.1            Created
 *	2013-12-11         Li Xianzhong      0.2            Enhanced for 5.x driver
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <gc_hal.h>
#include <gc_hal_base.h>
#include <gc_hal_driver.h>

static int query_idle=1;
static int query_gpuinfo(int pid, int *reserved, int *contiguous, int *virtual, int *nonpaged, float *idle)
{
    gceSTATUS status;
    gcsHAL_INTERFACE iface;
    gceHARDWARE_TYPE currentType = gcvHARDWARE_3D;

    memset(&iface, 0, sizeof(iface));

    /* Query current hardware type. */
    gcoHAL_GetHardwareType(gcvNULL, &currentType);

    gcmVERIFY_OK(
        gcoHAL_SetHardwareType(gcvNULL, gcvHARDWARE_3D));

    iface.command = gcvHAL_DATABASE;
    iface.u.Database.processID = (gctUINT32)(gctUINTPTR_T)pid;
    iface.u.Database.validProcessID = gcvTRUE;

    /* Call kernel service. */
    gcmONERROR(gcoOS_DeviceControl(
        gcvNULL,
        IOCTL_GCHAL_INTERFACE,
        &iface, gcmSIZEOF(iface),
        &iface, gcmSIZEOF(iface)
        ));

    if(reserved)
    {
        *reserved = (int)iface.u.Database.vidMemPool[0].counters.bytes;
    }

    if(contiguous)
    {
        *contiguous = (int)iface.u.Database.vidMemPool[1].counters.bytes;
    }

    if(virtual)
    {
        *virtual = (int)iface.u.Database.vidMemPool[2].counters.bytes;
    }

    if(nonpaged)
    {
        *nonpaged = (int)iface.u.Database.contiguous.counters.bytes
                  + (int)iface.u.Database.nonPaged.counters.bytes;
    }

    if(idle && query_idle)
    {
        *idle = iface.u.Database.gpuIdle.time / 1000000.0f;
        query_idle = 0;
    }

    gcmVERIFY_OK(
        gcoHAL_SetHardwareType(gcvNULL, currentType));

    return 0;

OnError:

   return -1;
}

struct gmem_node
{
   int pid;
   int total;
   int reserved;
   int contiguous;
   int virtual;
   int nonpaged;
   char name[512];
};

static int gmem_node_compare(const void *p1, const void *p2)
{
  struct gmem_node *g1 = *(struct gmem_node **)p1;
  struct gmem_node *g2 = *(struct gmem_node **)p2;

  return g1->total > g2->total ? 1 : -1;
}

static int format_d(int digit, char *p, int size)
{
    int i,j=11;

    if(!p) return -1;
    if(size < 16) return -1;

    memset(p, 0, 16);

    for(i=0; i<9; i++,j--)
    {
        if(!i || digit)
        {
            p[j] = '0' + (digit % 10);

            digit /= 10;
        }
        else
        {
             p[j] = ' ';
        }

        if((i+1) % 3 == 0)
        {
            if(digit)
            {
                p[--j] = ',';
            }
            else
            {
                p[--j] = ' ';
            }
        }
    }

    return 0;
}

static int print_vidmem_info(char *pid)
{
    int i=0;
    FILE* fd=NULL;
 //   char *p0, *p1;
    char /*str[16],*/info[512];
/*
    char cmdline[512];

    sprintf(cmdline, "/proc/%s/cmdline", pid);
    fd = fopen(cmdline, "rb");
    if(!fd) return -1;

    memset(&cmdline[0], 0, sizeof(cmdline));
    fread(cmdline, 1, sizeof(cmdline)-1, fd);
    fclose(fd);

    printf("Process %s <%s>\n", pid, cmdline);
*/
    fd = fopen("/sys/kernel/debug/gpu/vidmem", "w+");
    fwrite(pid, 1, strlen(pid), fd);

    for(i=0; i<3; i++)
    {
        memset(&info[0], 0, sizeof(info));
        fgets(&info[0], sizeof(info)-1, fd);

        printf("%s", info + (i ? 10 : 0));
    }
/*
    memset(&info[0], 0, sizeof(info));
    fgets(&info[0], sizeof(info)-1, fd);

    printf("%s", info);

    p1 = &info[0];
    p0 = strstr(p1, " ");

    while(p0 = strstr(p1, " "))
    {
        while(*p0 == ' ') p0++;

        p1 = strstr(p0, " ");
        if(!p1) p1 = strstr(p0, "\n");

        *p1++ = 0;

        format_d(atoi(p0), &str[0], sizeof(str));

        printf("%s ", str);
    }

    printf("\n");
*/
    fclose(fd);

    return 0;
}

int main(int argc, char **argv)
{
    DIR * dir;
    gcoOS Os = gcvNULL;
    struct dirent * ptr;
    int pid,num,total,i=0;
    int reserved,contiguous,virtual,nonpaged;
    struct gmem_node *list[4096];
    char str0[16],str1[16],str2[16],str3[16],str4[16];
    char cmdline[512];
    FILE* fd=NULL;
    float idle=0.0;
    gceSTATUS status = gcvSTATUS_OK;

    if(argc > 1 && argv[1])
    {
       return print_vidmem_info(argv[1]);
    }

    chdir("/proc");
    dir = opendir("/proc");

    memset(list, 0, sizeof(list));

    status = gcoOS_Construct(gcvNULL, &Os);
    if (gcmIS_ERROR(status)) { return status; }

    while ((ptr = readdir(dir)) != NULL && i < (int)(sizeof(list)/sizeof(list[0])))
    {
       char *p = ptr->d_name;
       if(!p) continue;

       while(*p)
       {
          if(*p < '0' || *p > '9') break;
          p++;
       }

       if(p[0]) continue;

       pid = atoi(ptr->d_name);
       if(pid == getpid()) continue;

       reserved=0;
       contiguous=0;
       virtual=0;
       nonpaged=0;

       if(!query_gpuinfo(pid, &reserved, &contiguous, &virtual, &nonpaged, &idle))
       {
           total = reserved + contiguous + virtual + nonpaged;
           if(!total) continue;

           sprintf(cmdline, "/proc/%s/cmdline", ptr->d_name);
           fd = fopen(cmdline, "rb");
           if(!fd) continue;

           memset(&cmdline[0], 0, sizeof(cmdline));
           fread(cmdline, 1, sizeof(cmdline)-1, fd);
           fclose(fd);

           list[i] = (struct gmem_node *)malloc(sizeof(struct gmem_node));

           list[i]->pid = pid;
           list[i]->total = total;
           list[i]->reserved = reserved;
           list[i]->contiguous = contiguous;
           list[i]->virtual = virtual;
           list[i]->nonpaged = nonpaged;
           snprintf(list[i]->name, sizeof(list[i]->name), "%s", cmdline);

           i++;
       }
    }
    closedir(dir);
    gcoOS_Destroy(Os);

    qsort(&list[0], i, sizeof(list[0]), gmem_node_compare);

    total=0;
    reserved=0;
    contiguous=0;
    virtual=0;
    nonpaged=0;
    num = i;

    printf(" Pid          Total      Reserved    Contiguous       Virtual      Nonpaged    Name\n");
    while(i--)
    {
        format_d(list[i]->total, str0, sizeof(str0));
        format_d(list[i]->reserved, str1, sizeof(str1));
        format_d(list[i]->contiguous, str2, sizeof(str2));
        format_d(list[i]->virtual, str3, sizeof(str3));
        format_d(list[i]->nonpaged, str4, sizeof(str4));

        printf(" %4d  %s  %s  %s  %s  %s    %s\n",
               list[i]->pid, str0, str1, str2, str3, str4, list[i]->name);

        total += list[i]->total;
        reserved += list[i]->reserved;
        contiguous += list[i]->contiguous;
        virtual += list[i]->virtual;
        nonpaged += list[i]->nonpaged;

        free(list[i]);
        list[i] = NULL;
    }

    printf(" ------------------------------------------------------------------------------\n");

    format_d(total, str0, sizeof(str0));
    format_d(reserved, str1, sizeof(str1));
    format_d(contiguous, str2, sizeof(str2));
    format_d(virtual, str3, sizeof(str3));
    format_d(nonpaged, str4, sizeof(str4));

    printf(" %4d  %s  %s  %s  %s  %s    Summary\n",
             num, str0, str1, str2, str3, str4);
#ifndef __QNXNTO__
    fd = fopen("/sys/module/galcore/parameters/contiguousSize", "rb");
    if(!fd) return 0;

    memset(&cmdline[0], 0, sizeof(cmdline));
    fread(cmdline, 1, sizeof(cmdline)-1, fd);
    fclose(fd);

    format_d(atoi(cmdline) - reserved, str0, sizeof(str0));

    printf("    -             -  %s             -             -             -    Available\n", str0);
#endif

    printf("GPU Idle time:  %.6f ms\n", idle);

    return 0;
}
