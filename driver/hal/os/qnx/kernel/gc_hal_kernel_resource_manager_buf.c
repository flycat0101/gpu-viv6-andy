#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <errno.h>
#include <stdarg.h>

#include "gc_hal_kernel_qnx.h"
#include "gc_hal_driver.h"
#include "gc_hal_kernel_resource_manager.h"
#include "gc_hal_kernel_resource_manager_buf.h"

struct buf_msg
*buf_msg_create(size_t _allocated, struct thread_state *thread)
{
	struct buf_msg *buf_msg = calloc(1, sizeof(*buf_msg));

	buf_msg->buf = NULL;
	buf_msg->pos = 0;
	buf_msg->allocated = _allocated;

	buf_msg->thread = thread;
	return buf_msg;
}

void
buf_msg_clean(struct buf_msg **buf_msg)
{
	struct buf_msg *bmsg = *buf_msg;

	if (bmsg) {
		if (bmsg->buf) {
			free(bmsg->buf);

			bmsg->buf = NULL;
			bmsg->pos = 0;
			bmsg->allocated = 0;
		}
	}
}

void
buf_msg_destroy(struct buf_msg **buf_msg)
{
	if (*buf_msg) {
		buf_msg_clean(buf_msg);
		(*buf_msg)->thread = NULL;
		free(*buf_msg);
		*buf_msg = NULL;
	}
}

/*
 * helper function to continuously add output
 */
void
msg_buf_add_msg(struct buf_msg *buf_msg, const char *fmt, ...)
{
	size_t size = 0;
	va_list ap;

	/* determine initial required size */
	va_start(ap, fmt);
	size = vsnprintf(NULL, size, fmt, ap);
	va_end(ap);

	if (size < 0) {
		return;
	}

	/* add '\0' */
	size++;

	if (buf_msg->pos == 0) {
		buf_msg->buf = calloc(1, buf_msg->allocated * sizeof(char));
	} else if (buf_msg->pos + size >= buf_msg->allocated) {
		buf_msg->allocated *= 2;

		/*
		 *  We can't be sure that twice the buffer will actually be sufficient,
		 *  so we overwrite it with that value.
		 */
		if (buf_msg->allocated < buf_msg->msg_pos + size) {
			buf_msg->allocated = buf_msg->msg_pos + size;
		}

		buf_msg->buf = realloc(buf_msg->buf, 
				       buf_msg->allocated * sizeof(char));
	}

	if (buf_msg->buf == NULL) {
		return;
	}

	va_start(ap, fmt);
	size = vsnprintf(&buf_msg->buf[buf_msg->pos], size, fmt, ap);
	if (size < 0) {
		free(buf_msg->buf);
		return;
	}
	va_end(ap);

	buf_msg->pos += size;
}
