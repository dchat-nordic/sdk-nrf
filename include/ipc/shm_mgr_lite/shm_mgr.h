/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SHM_MGR_H_
#define SHM_MGR_H_

#include <stddef.h>

/**
 * @file
 * @brief Experimental lightweight shared memory manager.
 */

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Init Shared Memory Manager.
 *
 *  This function initializes header in current core scope of memory,
 *	initializes shared memory heap and heap functions security.
 *
 * 	@retval 0 If successful.
 * 	@retval -EALREADY The interface has been initialized before.
 */
int shm_mgr_init (void);

/** @brief Allocate from shared memory.
 *
 *  This function allocates a block of memory from shared memory
 *	heap. All allocations are word aligned.
 *
 *  @param[in] size Size of requested allocation size (in bytes).
 *  @param[out] ptr Pointer to variable storing address to allocated
 *					memory, or NULL if failed to allocate.
 *
 * 	@retval 0 If successful.
 * 	@retval -EPERM The Shared Memory Manager is not initialized.
 * 	@retval -EINVAL Either pointer is NULL, or requested allocation
 *			size is 0.
 * 	@retval -EIO The function has failed to allocate requested memory.
 */
int shm_alloc (size_t size, void **ptr);

/** @brief Free allocated shared memory.
 *
 *  This function frees previously allocated block from shared memory.
 *	The function checks validity of pointer.
 *
 *  @param[in] ptr Pointer to allocated shared memory.
 *
 * 	@retval 0 If successful.
 * 	@retval -EPERM The Shared Memory Manager is not initialized.
 * 	@retval -EINVAL Pointer is not in shared memory, or points
 * 			to invalid block.
 */
int shm_free (void *ptr);

/** @brief Set Channel Data Pointer
 *
 *  This function sets data pointer of requested channel number
 *	to provided pointer.
 *	Pointer validity is NOT checked.
 *
 *  @param[in] channel IPC channel number.
 *  @param[in] ptr Pointer to data in shared memory.
 *
 * 	@retval 0 If successful.
 * 	@retval -EPERM The Shared Memory Manager is not initialized.
 * 	@retval -EINVAL Channel number is not valid.
 */
int shm_set_chnl_data_ptr (int channel, void *ptr);

/** @brief Get Channel Data Pointer
 *
 *  This function retrieves data pointer of requested channel number
 *	and returns it in provided pointer.
 *
 *  @param[in] channel IPC channel number.
 *  @param[out] ptr Pointer variable storing pointer in shared memory.
 *
 * 	@retval 0 If successful.
 * 	@retval -EPERM The Shared Memory Manager is not initialized.
 * 	@retval -EINVAL Channel number is not valid.
 */
int shm_get_chnl_data_ptr (int channel, void **ptr);

#ifdef __cplusplus
}
#endif

#endif /* SHM_MGR_H_ */
