#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <errno.h>
#include <stdarg.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <atomic.h>
#include <pthread.h>

//#define __QNX_RESOURCE_MANAGER_DEBUG

/* a sufficiently large size for the buff */
#define ALLOC_BUF_INITIAL_SIZE	(8 * 1024)

#include "gc_hal_kernel_qnx.h"
#include "gc_hal_driver.h"
#include "gc_hal_kernel_resource_manager.h"
#include "gc_hal_kernel_resource_manager_buf.h"
#include "gc_hal_kernel_resource_manager_info.h"


static resmgr_connect_funcs_t connect_funcs;
static resmgr_io_funcs_t io_funcs;
static iofunc_attr_t attr;

static struct thread_state thread;
static int thread_exit = 1;

static struct rsmgr resources[] = {
	{ -1, RESOURCE_INFO, R_INFO, gc_info_show, NULL },
	{ -1, RESOURCE_CLIENTS, R_CLIENTS, gc_clients_show, NULL },
	{ -1, RESOURCE_MEMINFO, R_MEMINFO, gc_meminfo_show, NULL },
	{ -1, RESOURCE_DATABASE, R_DATABASE, gc_db_show, NULL },
	{ -1, RESOURCE_VERSION, R_VERSION, gc_version_show, NULL },
	{ -1, RESOURCE_VIDMEM, R_VIDMEM, gc_vidmem_show, gc_vidmem_write },
	{ -1, RESOURCE_DUMP_TRIGGER, R_DUMP_TRIGGER, gc_dump_trigger_show, gc_dump_trigger_write },
	{ -1, RESOURCE_CLOCK, R_CLOCK, gc_clk_show, NULL },
};

static struct rsmgr
*rsmgr_get_by_id(int rid)
{
	int i;
	struct rsmgr *resource = NULL;

	for (i = 0; i < ARRAY_SIZE(resources); i++) {
		if (rid == resources[i].rsmgr_id) {
			resource = &resources[i];
			break;
		}
	}

	return resource;
}

static void
rsmgr_add_id(int rid)
{
	static size_t entry = 0;

	if (entry >= ARRAY_SIZE(resources)) {
		return;
	}

	resources[entry++].rsmgr_id = rid;
}

/*
 *
 * This was mostly copy-pasted from QNX documentation, modified to check which
 * cb to call and adjusted for variable buf size.
 *
 * See http://www.qnx.com/developers/docs/7.0.0/#com.qnx.doc.neutrino.resmgr/topic/read_write_Sample_IO_READ.html
 *
 */
int
io_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb)
{
	size_t nbytes, nleft;
	int nparts, status;

	if ((status = iofunc_read_verify(ctp, msg, ocb, NULL)) != EOK) {
		dprintf_rsmgr("Failed iofunc_read_verify %d\n", status);
		return status;
	}

	if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE) {
		dprintf_rsmgr("Invalid type 0x%x\n", msg->i.xtype);
		return ENOSYS;
	}

	/*
	 *  On all reads (first and subsequent), calculate how many bytes we
	 *  can return to the client, based upon the number of bytes available
	 *  (nleft) and the client's buffer size
	 *
	 *  N.B: This check only happens once, and given that we don't know
	 *  before hand how much our buffer will be, we invoke the cb and
	 *  determine the buffer size.
	 *  Subsequent runs will have ocb->attr->nbytes set. Further more
	 *  we set again at 0 when we no longer have anything to send (nbytes
	 *  == 0)
	 */
	if (ocb->attr->nbytes == 0) {
		struct rsmgr *resource = rsmgr_get_by_id(ctp->id);

		if (resource && resource->read) {
			struct buf_msg *buf_msg = THREAD_GET_BUF_MSG(thread);

			/*
			 * No retval to verify here as the driver only
			 * retrieves data and places it in buf_msg struct. In
			 * case there's no data ocb->attr->nbytes will be 0 and
			 * we'll return 0 to the client.
			 */
			resource->read(buf_msg);
			ocb->attr->nbytes = BUF_MSG_GET_POS(buf_msg);
		} else {
			dprintf_rsmgr("Resource not available or read method not installed!\n");
		}
	}

	nleft = ocb->attr->nbytes - ocb->offset;
	nbytes = min(_IO_READ_GET_NBYTES(msg), nleft);

	if (nbytes > 0) {
		struct buf_msg *buf_msg = THREAD_GET_BUF_MSG(thread);

		/* set up the return data IOV */
		SETIOV(ctp->iov, BUF_MSG_GET_BUF(buf_msg) + ocb->offset, nbytes);

		/* set up the number of bytes (returned by client's read()) */
		_IO_SET_READ_NBYTES(ctp, nbytes);

		/*
		 * advance the offset by the number of bytes
		 * returned to the client.
		 */
		ocb->offset += nbytes;
		nparts = 1;
	} else {
		struct buf_msg **buf_msg = &THREAD_GET_BUF_MSG(thread);
		/*
		 * they've asked for zero bytes or they've already previously
		 * read everything
		 */
		_IO_SET_READ_NBYTES(ctp, 0);
		nparts = 0;

		/*
		 * When we are done with it reset ocb->attr->nbytes so that new
		 * read() calls will be reset to 0 and go again through read cb
		 */
		ocb->attr->nbytes = 0;
		buf_msg_clean(buf_msg);
	}

	/* mark the access time as invalid (we just accessed it) */
	if (msg->i.nbytes > 0)
		ocb->attr->flags |= IOFUNC_ATTR_ATIME;

	return _RESMGR_NPARTS(nparts);
}

int
io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb)
{
	int status;
	char *buf = NULL;
	size_t nbytes = 0;
	struct rsmgr *resource = NULL;

	if ((status = iofunc_write_verify(ctp, msg, ocb, NULL)) != EOK) {
		dprintf_rsmgr("Failed iofunc_read_verify %d\n", status);
		return status;
	}

	if ((msg->i.xtype & _IO_XTYPE_MASK) != _IO_XTYPE_NONE) {
		dprintf_rsmgr("Invalid type 0x%x\n", msg->i.xtype);
		return ENOSYS;
	}

	/* identify which resource we should handle */
	resource = rsmgr_get_by_id(ctp->id);

	/* Extract the length of the client's message. */
	nbytes = _IO_WRITE_GET_NBYTES(msg);

	/* Filter out malicious write requests that attempt to write more
	   data than they provide in the message. */
	if (nbytes > (size_t) ctp->info.srcmsglen - (size_t) ctp->offset - sizeof(io_write_t)) {
		dprintf_rsmgr("Writing more than provided not allowed!\n");
		return EBADMSG;
	}

	/* set up the number of bytes (returned by client's write()) */
	_IO_SET_WRITE_NBYTES(ctp, nbytes);

	if (!resource || !resource->write) {
		dprintf_rsmgr("Resource not available or write method not installed\n");
		goto out;
	}


	buf = calloc(1, (nbytes + 1) * sizeof(char));
	if (buf == NULL)
		return ENOMEM;

	/*
	 * Reread the data from the sender's message buffer.  We're not
	 * assuming that all of the data fit into the resource manager
	 * library's receive buffer.
	 */
	resmgr_msgread(ctp, buf, nbytes, sizeof(msg->i));

	/* just in case the text is not NULL terminated */
	buf[nbytes] = '\0';

	resource->write(&(struct write_msg) { .buf = buf, .nbytes = nbytes } );

	free(buf);
out:
	if (nbytes > 0)
		ocb->attr->flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;

	return _RESMGR_NPARTS(0);
}

int
io_open(resmgr_context_t *ctp, io_open_t *msg,
	RESMGR_HANDLE_T *handle, void *extra)
{
	if (atomic_add_value(thread.refcnt, 1)) {
		/*
		 * If another client connects to us while we are servicing a
		 * read/write we would get 2, so we decrement it. Otherwise
		 * another legit client won't be able to connect once the
		 * previous client has closed its own connection. This function
		 * returns the previous value, which should normally be 0 when
		 * a client connects to us.
		 */
		atomic_sub(thread.refcnt, 1);
		return EBADF;
	}

	struct buf_msg **buf_msg = &THREAD_GET_BUF_MSG(thread);

	if (*buf_msg == NULL)
		*buf_msg = buf_msg_create(ALLOC_BUF_INITIAL_SIZE, &thread);

	return iofunc_open_default(ctp, msg, handle, extra);
}

int
io_close(resmgr_context_t *ctp, io_close_t *msg, RESMGR_OCB_T *ocb)
{
	/*
	 * More than 1 would yield that something went really wrong.
	 */
	if (atomic_sub_value(thread.refcnt, 1) > 1) {
		return EBADF;
	}

	buf_msg_destroy(&THREAD_GET_BUF_MSG(thread));
	return iofunc_close_dup_default(ctp, msg, ocb);
}

static void *
resource_manager_loop(void *arg)
{
	dispatch_context_t *dispatch_context = (dispatch_context_t *) arg;
	dprintf_rsmgr("Spawned thread %d:%d\n", getpid(), gettid());

	do {
		if ((dispatch_context = dispatch_block(dispatch_context)) == NULL) {
			return NULL;
		}
		dispatch_handler(dispatch_context);
	} while (thread_exit);

	dprintf_rsmgr("Resource manager loop exited\n");
	return NULL;
}

/*
 * For more info about resource manager see
 *
 * General:
 * http://www.qnx.com/developers/docs/7.0.0/#com.qnx.doc.neutrino.resmgr/topic/about.html
 * Basic skeleton:
 * http://www.qnx.com/developers/docs/7.0.0/#com.qnx.doc.neutrino.resmgr/topic/skeleton_SIMPLE_ST_EG.html
 * Message types:
 * http://www.qnx.com/developers/docs/7.0.0/#com.qnx.doc.neutrino.resmgr/topic/fleshing_out_Message_types.html
 */
int
resource_manager_init(gckGALDEVICE device)
{
	int ret, id;
	int i;

	resmgr_attr_t rattr = {};
	dispatch_t *dispatch = NULL;
	dispatch_context_t *dispatch_context = NULL;

	memset(&thread, 0, sizeof(struct thread_state));

	thread.device = device;
	thread.refcnt = calloc(1, sizeof(unsigned long));
	atomic_clr(thread.refcnt, 0);

	dispatch = dispatch_create();
	if (dispatch == NULL) {
		dprintf_rsmgr("Failed to init dispatch!\n");
		return EXIT_FAILURE;
	}

	memset(&rattr, 0, sizeof(resmgr_attr_t));
	rattr.nparts_max = 1;
	rattr.msg_max_size = 4096;

	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs,
			 _RESMGR_IO_NFUNCS, &io_funcs);

	io_funcs.read = io_read;
	io_funcs.read64 = io_read;

	io_funcs.write = io_write;
	io_funcs.write64 = io_write;

	connect_funcs.open = io_open;
	io_funcs.close_dup = io_close;

	iofunc_attr_init(&attr, S_IFNAM | 0666, 0, 0);
	/* we will adjust this dynamically based on output */
	attr.nbytes = 0;

	/* add entry for each resource */
	for (i = 0; i < ARRAY_SIZE(resources); i++) {
		id = resmgr_attach(dispatch, &rattr, resources[i].resource_name,
				  _FTYPE_ANY, 0, &connect_funcs, &io_funcs, &attr);

		/*
		 * The resoures array already has read/write cbs we just need
		 * the context id to be set-up in order to identify which cb
		 * matches which resource.
		 */
		rsmgr_add_id(id);
	}

	dispatch_context = dispatch_context_alloc(dispatch);

	/* spawn a thread as we'll be blocked */
	ret = pthread_create(&thread.tid, NULL, resource_manager_loop, dispatch_context);
	if (ret == -1) {
		dprintf_rsmgr("Failed to create thread to handle resource manager loop\n");
		return -1;
	}

	dprintf_rsmgr("Resource manager inited\n");
	return 0;
}

int
resource_manager_exit(void)
{
	thread_exit = 0;
	pthread_join(thread.tid, NULL);

	free((void *) thread.refcnt);
	dprintf_rsmgr("Resource manager exited\n");
	return 0;
}
