/* Copyright (c) 2014-2016, Marel
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef SDO_REQ_H_
#define SDO_REQ_H_

#include <sys/queue.h>
#include <stddef.h>
#include <mloop.h>
#include "vector.h"
#include "canopen/sdo.h"
#include "arc.h"

#include "canopen/sdo_async.h"
#include "canopen/sdo_req_enums.h"
#include "type-macros.h"

struct sdo_req;
struct sock;

typedef void (*sdo_req_fn)(struct sdo_req*);
typedef void (*sdo_req_free_fn)(void*);

struct sdo_req_info {
	enum sdo_req_type type;
	int index, subindex;
	sdo_req_fn on_done;
	const void* dl_data;
	size_t dl_size;
	void* context;
};

struct sdo_req_queue;

struct sdo_req {
	int ref;
	TAILQ_ENTRY(sdo_req) links;
	enum sdo_req_type type;
	int index, subindex;
	struct vector data;
	enum sdo_req_status status;
	enum sdo_abort_code abort_code;
	sdo_req_fn on_done;
	struct sdo_req_queue* parent;
	void* context;
	sdo_req_free_fn context_free_fn;
	int is_size_indicated;
};

TAILQ_HEAD(sdo_req_list, sdo_req);

struct sdo_req_queue {
	pthread_mutex_t mutex;
	size_t size;
	size_t limit;
	struct sdo_req_list list;
	struct sdo_async sdo_client;
	struct mloop_idle* idle;
	int nodeid;
};

int sdo_req__queue_init(struct sdo_req_queue* self, const struct sock* sock,
			int nodeid, size_t, enum sdo_async_quirks_flags quirks);
void sdo_req__queue_destroy(struct sdo_req_queue* self);

int sdo_req_queues_init(const struct sock* sock, size_t limit,
			enum sdo_async_quirks_flags quirks);
void sdo_req_queues_cleanup();

struct sdo_req_queue* sdo_req_queue_get(int nodeid);
void sdo_req_queue_flush(struct sdo_req_queue* self);

struct sdo_req* sdo_req_new(struct sdo_req_info* info);
void sdo_req_free(struct sdo_req* self);

int sdo_req_start(struct sdo_req* self, struct sdo_req_queue* queue);
void sdo_req_wait(struct sdo_req* self);

int sdo_req_queue__enqueue(struct sdo_req_queue* self, struct sdo_req* req);
struct sdo_req* sdo_req_queue__dequeue(struct sdo_req_queue* self);

static inline
struct sdo_req_queue* sdo_req_queue__from_async(const struct sdo_async* async)
{
	return container_of(async, struct sdo_req_queue, sdo_client);
}

ARC_PROTOTYPE(sdo_req)

#endif /* SDO_REQ_H_ */

