/*
 * Copyright (c) 2026 Aerlync Labs Inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/post/post.h>
#include <zephyr/shell/shell.h>
#include <zephyr/logging/log.h>

#if defined(CONFIG_SHELL) && defined(CONFIG_POST_MTB_STL_SAVE_STARTUP_CFGREG)
#include <errno.h>
#include <mtb_stl/startup.h>
#endif

LOG_MODULE_REGISTER(post_sample, LOG_LEVEL_INF);

#if defined(CONFIG_SHELL) && defined(CONFIG_POST_MTB_STL_SAVE_STARTUP_CFGREG)
static int cmd_startup_save(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	int ret = mtb_stl_startup_provision();

	if (ret == 0) {
		shell_print(sh, "Startup reference save: PASS");
	} else if (ret == -EALREADY) {
		shell_error(sh, "Startup reference save: NOT SAVED "
			    "(reference is not blank; no write performed)");
	} else {
		shell_error(sh, "Startup reference save: FAIL (%d)", ret);
	}

	return ret;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
	startup_cmds,
	SHELL_CMD_ARG(save, NULL, "Save the current known-good startup reference to blank flash",
		      cmd_startup_save, 1, 0),
	SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(stl_startup, &startup_cmds, "Explicit startup reference provisioning", NULL);
#endif

static struct {
	uint32_t passed;
	uint32_t failed;
	uint32_t skipped;
} summary;

int main(void)
{
	printk("\n");
	printk("====================================\n");
	printk("Starting POST sample application\n");
	printk("====================================\n");
	printk("\n");

	LOG_INF("POST subsystem initialized");
	LOG_INF("MTB-STL tests registered during boot");

	post_get_summary(&summary.passed, &summary.failed, &summary.skipped);
	printk("POST summary: %u passed, %u failed, %u skipped\n",
	       summary.passed, summary.failed, summary.skipped);

	printk("Available shell commands:\n");
	printk("  post list   - List all registered POST tests\n");
	printk("  post run <id> | --all - Run runtime POST tests\n");
	printk("  post status - Show POST test results\n");
#if defined(CONFIG_SHELL) && defined(CONFIG_POST_MTB_STL_SAVE_STARTUP_CFGREG)
	printk("  stl_startup save - Explicitly save a startup reference to blank flash\n");
#endif
	printk("\n");

	/* Application main loop */
	while (1) {
		k_sleep(K_SECONDS(5));
		LOG_DBG("Application running...");
	}

	return 0;
}
