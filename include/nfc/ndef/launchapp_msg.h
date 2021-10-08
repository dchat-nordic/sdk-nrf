/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef NFC_LAUNCHAPP_MSG_H__
#define NFC_LAUNCHAPP_MSG_H__

/** @file
 *
 * @defgroup nfc_launchapp_msg Launch app messages
 * @{
 * @ingroup nfc_ndef_messages
 *
 * @brief Generation of NFC NDEF messages that can be used to launch apps.
 *
 */

#include <stdint.h>
#include "nfc/ndef/msg.h"

#ifdef __cplusplus
extern "C" {
#endif


/** @brief Function for encoding an NFC NDEF launch app message.
 *
 * This function encodes an NFC NDEF message into a buffer.
 *
 * @param[in]  p_android_package_name       Pointer to the Android package name string.
 *                                          If NULL, the Android Application Record will be skipped.
 * @param[in]  android_package_name_length  Length of the Android package name.
 * @param[out] p_buf                        Pointer to the buffer for the message.
 * @param[in,out] p_len                     Size of the available memory for the message as input.
 *                                          Size of the generated message as output.
 *
 * @retval 0                        If the description was successfully created.
 * @retval -EINVAL                  If both p_android_package_name and windows_application_id were
 *                                  invalid (equal to NULL).
 * @retval -ENOMEM                  If the predicted message size is bigger than the provided
 *                                  buffer space.
 * @retval Other                    Other codes might be returned depending on
 *                                  the function @ref nfc_ndef_msg_encode
 */
int nfc_launchapp_msg_encode(uint8_t const * p_android_package_name,
                                    uint8_t         android_package_name_length,
                                    uint8_t       * p_buf,
                                    uint32_t      * p_len);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

 #endif // NFC_LAUNCHAPP_MSG_H__
