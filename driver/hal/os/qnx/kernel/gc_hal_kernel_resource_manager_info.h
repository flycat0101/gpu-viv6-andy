#ifndef __GC_HAL_KERNEL_RESOURCE_MANAGER_INFO_H
#define __GC_HAL_KERNEL_RESOURCE_MANAGER_INFO_H

int
gc_info_show(void *data);

int
gc_clients_show(void *data);

int
gc_meminfo_show(void *data);

int
gc_db_show(void *data);

int
gc_version_show(void *data);

int
gc_vidmem_show(void *data);

int
gc_dump_trigger_show(void *data);

int
gc_vidmem_write(void *data);

int
gc_dump_trigger_write(void *data);

#endif
