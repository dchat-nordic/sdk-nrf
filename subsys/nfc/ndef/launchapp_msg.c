/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "nfc/ndef/launchapp_rec.h"
#include "nfc/ndef/launchapp_msg.h"


int nfc_launchapp_msg_encode(uint8_t const * p_android_package_name,
                                    uint8_t         android_package_name_length,
                                    uint8_t       * p_buf,
                                    uint32_t      * p_len)
{
    int err_code;

    /* Create NFC NDEF message description, capacity - 2 records */
    NFC_NDEF_MSG_DEF(nfc_launchapp_msg, 2);

    /* Create NFC NDEF Android Application Record description */
    NFC_NDEF_ANDROID_LAUNCHAPP_RECORD_DESC_DEF(nfc_and_launchapp_rec,
                                               p_android_package_name,
                                               android_package_name_length);

    if (p_android_package_name != NULL)
    {
        /* Add Android Application Record as second record to message */
        err_code = nfc_ndef_msg_record_add(&NFC_NDEF_MSG(nfc_launchapp_msg),
                                           &NFC_NDEF_ANDROID_LAUNCHAPP_RECORD_DESC(
                                                                        nfc_and_launchapp_rec));
        if (err_code){
                return err_code;
        }
    }

    if (NFC_NDEF_MSG(nfc_launchapp_msg).record_count == 0){
            return -EINVAL;
    }

    /* Encode whole message into buffer */
    err_code = nfc_ndef_msg_encode(&NFC_NDEF_MSG(nfc_launchapp_msg),
                                   p_buf,
                                   p_len);

    return err_code;
}
