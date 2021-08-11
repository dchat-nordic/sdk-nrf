/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include "ipc/ipc_service_backend.h"
#include "sm_ipt.h"

#include <logging/log.h>
#include <zephyr.h>
#include <device.h>
#include <sys/slist.h>
#include <string.h>
#include <sys/crc.h>

LOG_MODULE_REGISTER(ipc_service_sm_ipt, CONFIG_IPC_SERVICE_LOG_LEVEL);

/* Internal definition of the structure ipc_ept */
struct ipc_ept {
	uint32_t id_hash;
	struct ipc_service_cb cb;
	void *priv;
	bool registered;
};

struct ept_list_container {
	sys_snode_t node;
	struct ipc_ept ept;
};

struct message {
	uint32_t hash;
	uint8_t data[];
};


static struct sm_ipt_ctx sm_ipt_ctx;
static bool sm_ipt_initialized;
static sys_slist_t ept_list;


static uint32_t jenkins_one_at_a_time_hash(const uint8_t* str)
{
	/* Jenkins one at a time hash is used to truncate
	 * endpoint names to constant size
	 */
	size_t curr_char = 0;
	uint32_t temp = 0;

	for(curr_char = 0; str[curr_char] != '\0'; curr_char++)
	{
		temp += str[curr_char];
		temp += temp << 10;
		temp ^= temp >> 6;
	}

	temp += temp << 3;
	temp ^= temp >> 11;
	temp += temp << 15;

	return temp;
}

static void sm_ipt_receive_handler(const uint8_t *packet, size_t len)
{
	struct ept_list_container *peek_container;
	struct message *message;
	bool ept_found = false;

	/* Check to see if sender hash is present in ept_list */
	message = ((struct message *) packet);
	SYS_SLIST_FOR_EACH_CONTAINER(&ept_list, peek_container, node) {
		if(peek_container->ept.id_hash == message->hash)
		{
			ept_found = true;
			break;
		}
	}

	if (len == sizeof(struct message)){
		/* If length of data is 0, then this is endpoint registration */
		if ((ept_found) && (peek_container->ept.registered)){
			/* If message is on the list and is registered on this side
			 * then a bound callback can be called
			 */
			peek_container->ept.cb.bound(peek_container->ept.priv);
		}
		else if (!ept_found){
			/* If hash is not on list, then add it (register from other side) */
			peek_container = k_malloc(sizeof(struct ept_list_container));
			peek_container->ept.id_hash = message->hash;
			peek_container->ept.registered = false;
			sys_slist_append(&ept_list, &peek_container->node);
		}
	}
	else{
		/* If length of data is >0, then this is normal message */
		if (ept_found && peek_container->ept.registered){
			peek_container->ept.cb.received(message->data, len-sizeof(struct message),
											peek_container->ept.priv);
		}
	}

	/* Always free the buffer */
	sm_ipt_free_rx_buf(&sm_ipt_ctx, packet);
}

static int send(struct ipc_ept *ept, const void *data, size_t len)
{
	/* Allocate buffer,
	 * add hash header, copy buffer
	 * and send via sm_ipt_send
	 */
	struct message *message;

	sm_ipt_alloc_tx_buf(&sm_ipt_ctx, (uint8_t **)&message, len+sizeof(struct message));

	message->hash = ept->id_hash;
	memcpy(message->data, data, len);

	return sm_ipt_send(&sm_ipt_ctx, (uint8_t *)message, len+sizeof(struct message));
}

static int register_ept(struct ipc_ept **ept, const struct ipc_ept_cfg *cfg)
{
	struct ept_list_container *ept_container;
	struct ept_list_container *peek_container;

	/* On first usage of this function
	 * initialize sm_ipt
	 */
	if (!sm_ipt_initialized){
		sm_ipt_init(&sm_ipt_ctx, sm_ipt_receive_handler);
		sm_ipt_initialized = true;
	}

	/* TODO:
	 * Check if another core (remote) is ON
	 */

	 /* Create endpoint data */
	ept_container = k_malloc(sizeof(struct ept_list_container));
	*ept = &ept_container->ept;

	(*ept)->id_hash = jenkins_one_at_a_time_hash(cfg->name);
	(*ept)->cb = cfg->cb;
	(*ept)->priv = cfg->priv;
	(*ept)->registered = true;
	//cfg->prio

	/* Chech if this hash is alreadly used.
	 * This could mean a collision occured
	 * or this endpoint is already registered
	 * on one side .
	 */
	SYS_SLIST_FOR_EACH_CONTAINER(&ept_list, peek_container, node) {
		if(peek_container->ept.id_hash == (*ept)->id_hash)
		{
			if (peek_container->ept.registered){
				/* Collision occured - name of at least
				 * one hash has to be changed
				 */
				 LOG_ERR("Collision occured with hash 0x%x", peek_container->ept.id_hash);
				return -EALREADY;
			}
			/* This endpoint is registered on the other side
			 * now register it on this side
			 */
			peek_container->ept.cb = cfg->cb;
			peek_container->ept.priv = cfg->priv;
			peek_container->ept.registered = true;
			/* Trigger bound callback on both sides */
			peek_container->ept.cb.bound(peek_container->ept.priv);
			send(*ept, NULL, 0);
			/* ept_container will NOT be used - free it */
			k_free(ept_container);
			return 0;
		}
	}

	/* This endpoint is registered only on this side
	 * append it to list and notify other side
	 */
	sys_slist_append(&ept_list, &ept_container->node);
	send(*ept, NULL, 0);

	return 0;
}

const static struct ipc_service_backend backend = {
	.name = "SM_IPT backend",
	.send = send,
	.register_endpoint = register_ept
};

static int backend_init(const struct device *dev)
{
	sm_ipt_initialized = false;
	sys_slist_init(&ept_list);

	return ipc_service_register_backend(&backend);
}

SYS_INIT(backend_init, POST_KERNEL, CONFIG_IPC_SERVICE_BACKEND_REG_PRIORITY);
