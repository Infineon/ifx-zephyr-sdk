/*
 * Copyright (c) 2026 Aerlync Labs Inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/post/post.h>
#include <zephyr/shell/shell.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(post_adc_sample, LOG_LEVEL_INF);

int main(void)
{
	uint32_t passed;
	uint32_t failed;
	uint32_t skipped;

	printk("\n");
	printk("====================================\n");
	printk("Starting ADC POST sample application\n");
	printk("====================================\n");
	printk("\n");

	LOG_INF("POST subsystem initialized");
	LOG_INF("MTB-STL tests registered during boot");

	post_get_summary(&passed, &failed, &skipped);
	printk("POST summary: %u passed, %u failed, %u skipped\n", passed, failed, skipped);

	printk("Available shell commands:\n");
	printk("  post list   - List all registered POST tests\n");
	printk("  post run    - Run runtime POST tests\n");
	printk("  post status - Show POST test results\n");
	printk("\n");

	/* Application main loop */
	while (1) {
		k_sleep(K_SECONDS(5));
		LOG_DBG("Application running...");
	}

	return 0;
}
