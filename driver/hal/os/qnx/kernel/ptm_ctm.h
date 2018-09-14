#ifndef __QNX_PTM_CTM_H
#define __QNX_PTM_CTM_H

struct ptm;
/* contig memory, it is used as a memory pool for gpu server        */
/* ctm could & should be reused to avoid free/alloc again for clients*/
struct ctm {
    int          shm_fd;
    unsigned     shm_special; /* shm ctl special flags */
    size_t       size;

    int          prot;        /* mapping prot */
    int          flags;       /* mapping flags */
    void       * vaddr;
    void       * vaddr_posix; /* if SHMCTL_TYMEM is not available */

    off64_t      paddr;
    struct ptm * parent;
    struct ctm * next;
    struct ctm * prev;
};

int ptm_init(void);
struct ctm * ptm_alloc_ctm(size_t size, int shm_fd, int shm_special);
void ptm_fini(void);

int  ctm_map(struct ctm * m, int prot, int flags);
void ctm_unmap(struct ctm  * m);
void ctm_free(struct ctm * m);

#endif
