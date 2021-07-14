/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef SHM_MGR_BOARDS_H_
#define SHM_MGR_BOARDS_H_

#include <devicetree.h>

#if !DT_HAS_CHOSEN(zephyr_ipc_shm)
#error "Shared memory manager requires definition of shared memory"
#endif

#if CONFIG_HEAP_MEM_POOL_SIZE == 0
#error "Shared memory manager requires heap"
#endif

#define SHM_HEADER_PTR(addr) ((struct shm_header *) (addr))
#define CORE_SHM_HEADER_PTR(core) 	(SHM_HEADER_PTR(CONFIG_SHM_MGR_LITE_BASE_ADDRESS + (MEMORY_PER_CORE * core)))

#if defined(CONFIG_SOC_NRF5340_CPUAPP) || defined(CONFIG_SOC_NRF5340_CPUNET)
#define	CORE_HEADERS	{CORE_SHM_HEADER_PTR(0), \
						CORE_SHM_HEADER_PTR(1)}
#else
#error "Unsupported cpu"
#endif

#endif /* SHM_MGR_BOARDS_H_ */
