#include <assert.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>

#include "ptm_ctm.h"

/* posix typed memory bellow 4g, it's the factory of ctm */
struct ptm {
	int fd;
	struct ctm head;
	pthread_mutex_t mutex;
};

static struct ptm _ptm = {
	.fd = -1,
	.mutex = PTHREAD_MUTEX_INITIALIZER,
};

#define CTM_INIT(m)	do {				\
		(m)->next = (m);					\
		(m)->prev = (m);					\
	} while(0)
#define CTM_INSERT(prev_node, m)	do {	\
		(prev_node)->next->prev = (m);		\
		(m)->next = (prev_node)->next;		\
		(prev_node)->next = (m);			\
		(m)->prev = (prev_node);			\
	}while(0)
#define CTM_REMOVE(m)	do {				\
		(m)->prev->next = (m)->next;		\
		(m)->next->prev = (m)->prev;		\
	}while(0)
#define CTM_IS_EMPTY(h)    ((h)->next == (h))

int
ptm_init(void)
{
	struct ptm * ptm = &_ptm;
	assert(ptm->fd == -1);

	CTM_INIT(&ptm->head);
	ptm->fd = posix_typed_mem_open("/memory/below4G/ram/sysram",
			O_RDWR, POSIX_TYPED_MEM_ALLOCATE_CONTIG);
	if (ptm->fd == -1) {
		fprintf(stderr, "%s[%d]: posix_typed_mem_open failed (errno=%d)\n",
				__FUNCTION__, __LINE__, errno);
	}
	return (ptm->fd == -1) ? -1 : 0;
}

struct ctm *
ptm_alloc_ctm(size_t size, int shm_fd, int shm_special)
{
	int rc = 0;
	struct ptm * ptm = &_ptm;
	struct ctm * m = calloc(1, sizeof(*m));
	if (!m) {
		fprintf(stderr, "%s[%d]: calloc failed\n",
				__FUNCTION__, __LINE__);
		return NULL;
	}
	assert(ptm->fd != -1);

#ifdef SHMCTL_TYMEM
	/* all the codes depend on SHMCTL_TYMEM are not tested yet */
	rc = shm_ctl_special(shm_fd,
			SHMCTL_ANON | SHMCTL_PHYS | SHMCTL_TYMEM,
			ptm->fd, size, special_flags);
	assert(rc == 0);
	if (rc) {
		free(m);
		return NULL;
	}
#else
	int prot = PROT_READ | PROT_WRITE;
	int flags = MAP_SHARED;
	int pt_fildes = NOFD;
	size_t pt_contig_len = 0;

	/* alloc memory from posix 4g memory */
	m->vaddr_posix = mmap64(0, size, prot, flags, ptm->fd, 0);
	if (m->vaddr_posix == MAP_FAILED) {
		rc = -1;
		fprintf(stderr, "%s[%d]: mmap64 failed (errono:%d)\n",
				__FUNCTION__, __LINE__, errno);
		goto failed;
	}
	/* lock memory */
	rc = mlock(m->vaddr_posix, size);
	if (rc) {
		fprintf(stderr, "%s[%d]: mprotect failed (errono:%d)\n",
				__FUNCTION__, __LINE__, errno);
		goto failed;
	}
	/* get physical address */
	rc = posix_mem_offset64(m->vaddr_posix, size, &m->paddr,
			&pt_contig_len, &pt_fildes);
	if (rc) {
		fprintf(stderr, "%s[%d]: posix_mem_offset64 failed (errono:%d)\n",
				__FUNCTION__, __LINE__, errno);
		goto failed;
	}
	assert(pt_contig_len == size);
	/* stop anyone to touch the memory */
	rc = mprotect(m->vaddr_posix, size, PROT_NONE);
	if (rc) {
		fprintf(stderr, "%s[%d]: mprotect failed(errono:%d)\n",
				__FUNCTION__, __LINE__, errno);
		goto failed;
	}
	/* set physical address to shm_fd */
	rc = shm_ctl_special(shm_fd, SHMCTL_PHYS, m->paddr, size, shm_special);
	if (rc) {
		fprintf(stderr, "%s[%d]: shm_ctl_special failed (errono:%d)\n",
				__FUNCTION__, __LINE__, errno);
		goto failed;
	}
 failed:
	if (rc) {
		if (m->vaddr && m->vaddr != MAP_FAILED) {
			munmap(m->vaddr, size);
		}
		if (m->vaddr_posix && m->vaddr_posix != MAP_FAILED) {
			munlock(m->vaddr_posix, size);
			munmap(m->vaddr_posix, size);
		}
		free(m);
		return NULL;
	}
#endif
	m->size = size;
	m->shm_fd = shm_fd;
	m->shm_special = shm_special;
	m->parent = ptm;

	rc = pthread_mutex_lock(&ptm->mutex);
	assert(rc == 0);

	CTM_INIT(m);
	CTM_INSERT(&ptm->head, m);

	rc = pthread_mutex_unlock(&ptm->mutex);
	assert(rc == 0);

	return m;
}

int
ctm_map(struct ctm * m, int prot, int flags)
{
	assert((m->shm_fd  != -1) && !m->vaddr && m->size);
#ifndef SHMCTL_TYMEM
	assert(m->vaddr_posix);
#endif
	/* alloc vaddr */
	m->vaddr = mmap64(0, m->size, prot, flags, m->shm_fd, 0);
	if (m->vaddr == MAP_FAILED) {
		fprintf(stderr, "%s[%d]: mmap64 failed\n",
				__FUNCTION__, __LINE__);
		return -errno;
	}
#ifdef SHMCTL_TYMEM
	/* get paddr */
	size_t csize;
	mlock(m->vaddr, size);
	rc = mem_offset64(m->vaddr, NOFD, m->size, &m->paddr, &csize);
	assert(rc == 0 && csize == m->size);
	if (rc) {
		int err = errno;
		munmap(m->vaddr, m->size);
		m->vaddr = NULL;
		return -err;
	}
#endif

	m->prot = prot;
	m->flags= flags;

	return 0;
}

/* TODO. keep mapping & put it into a free list for reuse */
void
ctm_unmap(struct ctm * m)
{
#ifdef SHMCTL_TYMEM
	if (m->vaddr) {
		/* unlock */
		munlock(m->vaddr, m->size);
		munmap(m->vaddr, m->size);
		m->vaddr = NULL;
	}
#else
	if (m->vaddr) {
		munmap(m->vaddr, m->size);
		m->vaddr = NULL;
	}
	if (m->vaddr_posix) {
		assert(m->vaddr_posix);
		munlock(m->vaddr_posix, m->size);
		munmap(m->vaddr_posix,	m->size);
		m->vaddr_posix = NULL;
	}
#endif
}

/* TODO. keep it &  put it into a free list for reuse */
void
ctm_free(struct ctm * m)
{
	ctm_unmap(m);

	struct ptm * ptm = m->parent;
	int rc = pthread_mutex_lock(&ptm->mutex);
	assert(rc == 0);
	CTM_REMOVE(m);
	rc = pthread_mutex_unlock(&ptm->mutex);
	assert(rc == 0);

	free(m);
}

void
ptm_fini(void)
{
	struct ptm * ptm = &_ptm;
	int rc = pthread_mutex_lock(&ptm->mutex);
	assert(rc == 0);
	if (ptm->fd != -1) {
		struct ctm * n;
		struct ctm * m = ptm->head.next;
		while(m != &ptm->head) {
			ctm_unmap(m);
			n = m->next;
			free(m);
			m = n;
		}
		close(ptm->fd);
		ptm->fd = -1;
	}
	rc = pthread_mutex_unlock(&ptm->mutex);
	assert(rc == 0);
}
