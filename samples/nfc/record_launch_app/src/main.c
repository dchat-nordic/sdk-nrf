/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr.h>
#include <sys/reboot.h>

#include <nfc_t2t_lib.h>
#include <nfc/ndef/msg.h>
#include <nfc/ndef/launchapp_msg.h>

#include <dk_buttons_and_leds.h>

#define MAX_REC_COUNT		3
#define NDEF_MSG_BUF_SIZE	256

#define NFC_FIELD_LED		DK_LED1

/** @snippet [NFC Launch App usage_0] */
/* nRF Toolbox Android application package name */
static const uint8_t m_android_package_name[] = {'n', 'o', '.', 'n', 'o', 'r', 'd', 'i', 'c', 's',
                                                 'e', 'm', 'i', '.', 'a', 'n', 'd', 'r', 'o', 'i',
                                                 'd', '.', 'n', 'r', 'f', 't', 'o', 'o', 'l', 'b',
                                                 'o', 'x'};

/* nRF Toolbox application ID for Windows phone */
static const uint8_t m_windows_application_id[] = {'{', 'e', '1', '2', 'd', '2', 'd', 'a', '7', '-',
                                                   '4', '8', '8', '5', '-', '4', '0', '0', 'f', '-',
                                                   'b', 'c', 'd', '4', '-', '6', 'c', 'b', 'd', '5',
                                                   'b', '8', 'c', 'f', '6', '2', 'c', '}'};

/* Buffer used to hold an NFC NDEF message. */
static uint8_t ndef_msg_buf[NDEF_MSG_BUF_SIZE];


static void nfc_callback(void *context,
			 enum nfc_t2t_event event,
			 const uint8_t *data,
			 size_t data_length)
{
	ARG_UNUSED(context);
	ARG_UNUSED(data);
	ARG_UNUSED(data_length);

	switch (event) {
	case NFC_T2T_EVENT_FIELD_ON:
		dk_set_led_on(NFC_FIELD_LED);
		break;
	case NFC_T2T_EVENT_FIELD_OFF:
		dk_set_led_off(NFC_FIELD_LED);
		break;
	default:
		break;
	}
}

int main(void)
{
	uint32_t len = sizeof(ndef_msg_buf);

	printk("Starting NFC LaunchApp example\n");

	/* Configure LED-pins as outputs */
	if (dk_leds_init() < 0) {
		printk("Cannot init LEDs!\n");
		goto fail;
	}

	/* Set up NFC */
	if (nfc_t2t_setup(nfc_callback, NULL) < 0) {
		printk("Cannot setup NFC T2T library!\n");
		goto fail;
	}


	/* Encode welcome message */
	if (nfc_launchapp_msg_encode(m_android_package_name,
                                            sizeof(m_android_package_name),
                                            m_windows_application_id,
                                            sizeof(m_windows_application_id),
                                            ndef_msg_buf,
                                            &len) < 0) {
		printk("Cannot encode message!\n");
		goto fail;
	}


	/* Set created message as the NFC payload */
	if (nfc_t2t_payload_set(ndef_msg_buf, len) < 0) {
		printk("Cannot set payload!\n");
		goto fail;
	}


	/* Start sensing NFC field */
	if (nfc_t2t_emulation_start() < 0) {
		printk("Cannot start emulation!\n");
		goto fail;
	}
	printk("NFC configuration done\n");

	return 0;

fail:
#if CONFIG_REBOOT
	sys_reboot(SYS_REBOOT_COLD);
#endif /* CONFIG_REBOOT */

	return -EIO;
}
