// SPDX-License-Identifier: GPL-2.0-only

#include <linux/delay.h>
#include <linux/module.h>
#include <linux/of.h>

#include <drm/display/drm_dsc.h>
#include <drm/display/drm_dsc_helper.h>
#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>
#include <drm/drm_probe_helper.h>

struct xiaomi_p12x {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct drm_dsc_config dsc;
	bool prepared;
};

static inline struct xiaomi_p12x *to_xiaomi_p12x(struct drm_panel *panel)
{
	return container_of(panel, struct xiaomi_p12x, panel);
}

static const struct drm_display_mode xiaomi_p12x_60hz_mode = {
	.clock = 289267,
	.hdisplay = 1280,
	.hsync_start = 1280 + 408,
	.hsync_end = 1280 + 408 + 2,
	.htotal = 1280 + 408 + 2 + 4,
	.vdisplay = 2772,
	.vsync_start = 2772 + 54,
	.vsync_end = 2772 + 54 + 10,
	.vtotal = 2772 + 54 + 10 + 10,
	.type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED,
};

static int xiaomi_p12x_prepare(struct drm_panel *panel)
{
	struct xiaomi_p12x *ctx = to_xiaomi_p12x(panel);
	struct drm_dsc_picture_parameter_set pps;
	struct device *dev = &ctx->dsi->dev;
	int ret;

	if (ctx->prepared)
		return 0;

	ctx->dsi->mode_flags |= MIPI_DSI_MODE_LPM;
	drm_dsc_pps_payload_pack(&pps, &ctx->dsc);

	ret = mipi_dsi_picture_parameter_set(ctx->dsi, &pps);
	if (ret < 0) {
		dev_err(dev, "failed to transmit DSC PPS: %d\n", ret);
		return ret;
	}

	ret = mipi_dsi_compression_mode(ctx->dsi, true);
	if (ret < 0) {
		dev_err(dev, "failed to enable DSC: %d\n", ret);
		return ret;
	}

	msleep(28);
	ctx->prepared = true;
	dev_info(dev, "adopted ABL-initialized P12X panel state\n");

	return 0;
}

static int xiaomi_p12x_unprepare(struct drm_panel *panel)
{
	struct xiaomi_p12x *ctx = to_xiaomi_p12x(panel);

	ctx->prepared = false;
	return 0;
}

static int xiaomi_p12x_get_modes(struct drm_panel *panel,
				 struct drm_connector *connector)
{
	connector->display_info.width_mm = 73;
	connector->display_info.height_mm = 158;

	return drm_connector_helper_get_modes_fixed(connector,
						    &xiaomi_p12x_60hz_mode);
}

static const struct drm_panel_funcs xiaomi_p12x_panel_funcs = {
	.prepare = xiaomi_p12x_prepare,
	.unprepare = xiaomi_p12x_unprepare,
	.get_modes = xiaomi_p12x_get_modes,
};

static int xiaomi_p12x_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct xiaomi_p12x *ctx;
	int ret;

	ctx = devm_drm_panel_alloc(dev, struct xiaomi_p12x, panel,
				   &xiaomi_p12x_panel_funcs,
				   DRM_MODE_CONNECTOR_DSI);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);

	ctx->dsi = dsi;
	mipi_dsi_set_drvdata(dsi, ctx);

	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_NO_EOT_PACKET |
			  MIPI_DSI_CLOCK_NON_CONTINUOUS;

	ctx->panel.prepare_prev_first = true;
	drm_panel_add(&ctx->panel);

	dsi->dsc = &ctx->dsc;
	ctx->dsc.dsc_version_major = 1;
	ctx->dsc.dsc_version_minor = 2;
	ctx->dsc.slice_height = 12;
	ctx->dsc.slice_width = 640;
	ctx->dsc.slice_count = 2;
	ctx->dsc.bits_per_component = 10;
	ctx->dsc.bits_per_pixel = 8 << 4;
	ctx->dsc.block_pred_enable = true;

	ret = mipi_dsi_attach(dsi);
	if (ret < 0) {
		drm_panel_remove(&ctx->panel);
		return dev_err_probe(dev, ret, "failed to attach DSI host\n");
	}

	return 0;
}

static void xiaomi_p12x_remove(struct mipi_dsi_device *dsi)
{
	struct xiaomi_p12x *ctx = mipi_dsi_get_drvdata(dsi);

	mipi_dsi_detach(dsi);
	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id xiaomi_p12x_of_match[] = {
	{ .compatible = "xiaomi,p12x-42-02-0a" },
	{ }
};
MODULE_DEVICE_TABLE(of, xiaomi_p12x_of_match);

static struct mipi_dsi_driver xiaomi_p12x_driver = {
	.probe = xiaomi_p12x_probe,
	.remove = xiaomi_p12x_remove,
	.driver = {
		.name = "panel-xiaomi-p12x",
		.of_match_table = xiaomi_p12x_of_match,
	},
};
module_mipi_dsi_driver(xiaomi_p12x_driver);

MODULE_DESCRIPTION("Xiaomi P12X bootloader-handoff DSC panel driver");
MODULE_LICENSE("GPL");
