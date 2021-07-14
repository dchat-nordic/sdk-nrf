/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 *
 */
#include <ipc/shm_mgr_lite/shm_mgr.h>
#include <ipc/shm_mgr_lite/boards.h>
#include <device.h>
#include <string.h>
#include <sys/util.h>
#include <sys/dlist.h>

#define CHANNELS_PER_CORE	(CONFIG_SHM_MGR_LITE_CHANNEL_COUNT / CONFIG_SHM_MGR_LITE_CORE_COUNT)
#define MEMORY_PER_CORE	(ROUND_DOWN((CONFIG_SHM_MGR_LITE_SHM_SIZE / CONFIG_SHM_MGR_LITE_CORE_COUNT),sizeof(void *)))
#define	CORE_SHM_ADDR	(ROUND_UP((CONFIG_SHM_MGR_LITE_CORE_ID * MEMORY_PER_CORE) + CONFIG_SHM_MGR_LITE_BASE_ADDRESS,sizeof(void *)))
#define CORE_SHM_END	(CORE_SHM_ADDR + MEMORY_PER_CORE)
#define	CORE_CHANNEL_OFFSET (CONFIG_SHM_MGR_LITE_CORE_ID * CHANNELS_PER_CORE)

struct shm_header{
	size_t channel_count;
	void *channel_data[CHANNELS_PER_CORE];
};

struct container_node{
	sys_dnode_t node;
	void *ptr;
};

K_MUTEX_DEFINE(shm_mutex);
static struct k_heap shm_heap;
static struct shm_header *core_header[CONFIG_SHM_MGR_LITE_CORE_COUNT] = CORE_HEADERS;
static bool shm_initialized = false;
static sys_dlist_t alloc_list;

int shm_mgr_init (void)
{
	k_mutex_lock(&shm_mutex, K_FOREVER);

	if (shm_initialized) {
		k_mutex_unlock(&shm_mutex);
		return -EALREADY;
	}

	core_header[CONFIG_SHM_MGR_LITE_CORE_ID]->channel_count = CHANNELS_PER_CORE;
	memset((void*) &core_header[CONFIG_SHM_MGR_LITE_CORE_ID]->channel_data, 0,
			(sizeof(void *) * CHANNELS_PER_CORE));

	sys_dlist_init(&alloc_list);
	k_heap_init(&shm_heap, (void *) (CORE_SHM_ADDR + sizeof (struct shm_header)),
									(MEMORY_PER_CORE - sizeof (struct shm_header)));

	shm_initialized = true;
	k_mutex_unlock(&shm_mutex);

	return 0;
}

int shm_alloc (size_t size, void **ptr)
{
	int retval;
	struct container_node *alloc_node;

	if (!shm_initialized) {
		return -EPERM;
	}

	if ((!ptr) || (0 == size)) {
		return -EINVAL;
	}

	k_mutex_lock(&shm_mutex, K_FOREVER);

	*ptr = k_heap_aligned_alloc(&shm_heap, sizeof(void *), size, K_FOREVER);

	if (NULL == *ptr){
		retval = -EIO;
	}
	else{
		alloc_node = (struct container_node *) k_aligned_alloc(sizeof(void *),
												sizeof(struct container_node));
		if (NULL == alloc_node){
			k_heap_free(&shm_heap, *ptr);
			retval = -EIO;
		}
		else{
			sys_dnode_init(&alloc_node->node);
			alloc_node->ptr = *ptr;
			sys_dlist_append(&alloc_list, &alloc_node->node);
			retval = 0;
		}
	}

	k_mutex_unlock(&shm_mutex);
	return retval;
}

int shm_free (void *ptr)
{
	struct container_node *cnode;
	bool node_found;
	int retval;

	if (!shm_initialized) {
		return -EPERM;
	}

	if ((((size_t) ptr) >= CORE_SHM_END) || (((size_t) ptr) < CORE_SHM_ADDR)){
		return -EINVAL;
	}

	k_mutex_lock(&shm_mutex, K_FOREVER);

	node_found = false;
	SYS_DLIST_FOR_EACH_CONTAINER(&alloc_list, cnode, node){
		if (cnode->ptr == ptr){
			node_found = true;
			break;
		}
	}

	if (node_found){
		k_heap_free(&shm_heap, ptr);
		sys_dlist_remove(&cnode->node);
		retval = 0;
	}
	else{
		retval = -EINVAL;
	}

	k_mutex_unlock(&shm_mutex);
	return retval;
}

static int shm_acc_chnl_data_ptr (int channel, void ***data_ptr)
{
	int core_channer_nr, core_nr;

	if (!shm_initialized) {
		return -EPERM;
	}

	if ((channel >= CONFIG_SHM_MGR_LITE_CHANNEL_COUNT) || (channel < 0) || (!data_ptr)){
		return -EINVAL;
	}

	core_nr = (channel / CHANNELS_PER_CORE);
	core_channer_nr = (channel % CHANNELS_PER_CORE);

	*data_ptr =  &core_header[core_nr]->channel_data[core_channer_nr];
	return 0;
}

int shm_set_chnl_data_ptr (int channel, void *ptr)
{
	int retval;
	void **chnl_data_ptr;

	retval = shm_acc_chnl_data_ptr(channel, &chnl_data_ptr);
	if (retval != 0){
		return retval;
	}

	k_mutex_lock(&shm_mutex, K_FOREVER);

	*chnl_data_ptr = ptr;

	k_mutex_unlock(&shm_mutex);
	return 0;
}

int shm_get_chnl_data_ptr (int channel, void **ptr)
{
	int retval;
	void **chnl_data_ptr;

	if (!ptr){
		retval = -EINVAL;
	}

	retval = shm_acc_chnl_data_ptr(channel, &chnl_data_ptr);
	if (retval != 0){
		return retval;
	}

	k_mutex_lock(&shm_mutex, K_FOREVER);

	*ptr = *chnl_data_ptr;

	k_mutex_unlock(&shm_mutex);
	return 0;
}
