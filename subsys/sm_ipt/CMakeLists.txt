#
# Copyright (c) 2021 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

zephyr_include_directories(include)

zephyr_library()

zephyr_library_sources_ifdef(CONFIG_SM_IPT sm_ipt_os.c)
zephyr_library_sources_ifdef(CONFIG_IPC_SERVICE_BACKEND_SM_IPT ipc_service_sm_ipt.c)
