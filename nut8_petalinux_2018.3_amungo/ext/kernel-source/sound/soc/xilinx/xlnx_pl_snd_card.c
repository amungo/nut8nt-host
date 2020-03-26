// SPDX-License-Identifier: GPL-2.0
/*
 * Xilinx ASoC sound card support
 *
 * Copyright (C) 2018 Xilinx, Inc.
 */

#include <linux/clk.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

#include "xlnx_snd_common.h"

#define I2S_CLOCK_RATIO 384

enum {
	I2S_AUDIO = 0,
	HDMI_AUDIO,
	SDI_AUDIO,
	XLNX_MAX_IFACE,
};

static const char *dev_compat[][XLNX_MAX_IFACE] = {
	[XLNX_PLAYBACK] = {
		"xlnx,i2s-transmitter-1.0",
		"xlnx,v-hdmi-tx-ss-3.1",
		"xlnx,v-uhdsdi-audio-2.0",
	},
	[XLNX_CAPTURE] = {
		"xlnx,i2s-receiver-1.0",
		"xlnx,v-hdmi-rx-ss-3.1",
		"xlnx,v-uhdsdi-audio-2.0",
	},
};

static struct snd_soc_card xlnx_card = {
	.name = "xilinx FPGA sound card",
	.owner = THIS_MODULE,
};

static int xlnx_sdi_card_hw_params(struct snd_pcm_substream *substream,
				   struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct pl_card_data *prv = snd_soc_card_get_drvdata(rtd->card);
	u32 sample_rate = params_rate(params);

	switch (sample_rate) {
	case 32000:
		prv->mclk_ratio = 576;
		break;
	case 44100:
		prv->mclk_ratio = 418;
		break;
	case 48000:
		prv->mclk_ratio = 384;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int xlnx_hdmi_card_hw_params(struct snd_pcm_substream *substream,
				    struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct pl_card_data *prv = snd_soc_card_get_drvdata(rtd->card);
	u32 sample_rate = params_rate(params);

	switch (sample_rate) {
	case 32000:
	case 44100:
	case 48000:
	case 96000:
	case 176400:
	case 192000:
		prv->mclk_ratio = 768;
		break;
	case 88200:
		prv->mclk_ratio = 192;
		break;
	default:
		return -EINVAL;
	}

	prv->mclk_val = prv->mclk_ratio * sample_rate;
	return clk_set_rate(prv->mclk, prv->mclk_val);
}

static int xlnx_i2s_card_hw_params(struct snd_pcm_substream *substream,
				   struct snd_pcm_hw_params *params)
{
	int ret, clk_div;
	u32 ch, data_width, sample_rate;
	struct pl_card_data *prv;

	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;

	ch = params_channels(params);
	data_width = params_width(params);
	sample_rate = params_rate(params);

	/* only 2 channels supported */
	if (ch != 2)
		return -EINVAL;

	prv = snd_soc_card_get_drvdata(rtd->card);
	if (substream->stream == SNDRV_PCM_STREAM_CAPTURE) {
		switch (sample_rate) {
		case 5512:
		case 8000:
		case 11025:
		case 16000:
		case 22050:
		case 32000:
		case 44100:
		case 48000:
		case 64000:
		case 88200:
		case 96000:
			prv->mclk_ratio = 384;
			break;
		default:
			return -EINVAL;
		}
	} else {
		switch (sample_rate) {
		case 32000:
		case 44100:
		case 48000:
		case 88200:
		case 96000:
			prv->mclk_ratio = 384;
			break;
		case 64000:
		case 176400:
		case 192000:
			prv->mclk_ratio = 192;
			break;
		default:
			return -EINVAL;
		}
	}

	clk_div = DIV_ROUND_UP(prv->mclk_ratio, 2 * ch * data_width);
	ret = snd_soc_dai_set_clkdiv(cpu_dai, 0, clk_div);

	return ret;
}

static const struct snd_soc_ops xlnx_sdi_card_ops = {
	.hw_params = xlnx_sdi_card_hw_params,
};

static const struct snd_soc_ops xlnx_i2s_card_ops = {
	.hw_params = xlnx_i2s_card_hw_params,
};

static const struct snd_soc_ops xlnx_hdmi_card_ops = {
	.hw_params = xlnx_hdmi_card_hw_params,
};

static struct snd_soc_dai_link xlnx_snd_dai[][XLNX_MAX_PATHS] = {
	[I2S_AUDIO] = {
		{
			.name = "xilinx-i2s_playback",
			.codec_dai_name = "snd-soc-dummy-dai",
			.codec_name = "snd-soc-dummy",
			.ops = &xlnx_i2s_card_ops,
		},
		{
			.name = "xilinx-i2s_capture",
			.codec_dai_name = "snd-soc-dummy-dai",
			.codec_name = "snd-soc-dummy",
			.ops = &xlnx_i2s_card_ops,
		},
	},
	[HDMI_AUDIO] = {
		{
			.name = "xilinx-hdmi-playback",
			.codec_dai_name = "i2s-hifi",
			.codec_name = "hdmi-audio-codec.0",
			.cpu_dai_name = "snd-soc-dummy-dai",
		},
		{
			.name = "xilinx-hdmi-capture",
			.codec_dai_name = "xlnx_hdmi_rx",
			.cpu_dai_name = "snd-soc-dummy-dai",
		},
	},
	[SDI_AUDIO] = {
		{
			.name = "xlnx-sdi-playback",
			.codec_dai_name = "xlnx_sdi_tx",
			.cpu_dai_name = "snd-soc-dummy-dai",
		},
		{
			.name = "xlnx-sdi-capture",
			.codec_dai_name = "xlnx_sdi_rx",
			.cpu_dai_name = "snd-soc-dummy-dai",
		},

	},
};

static int find_link(struct device_node *node, int direction)
{
	int ret;
	u32 i, size;
	const char **link_names = dev_compat[direction];

	size = ARRAY_SIZE(dev_compat[direction]);

	for (i = 0; i < size; i++) {
		ret = of_device_is_compatible(node, link_names[i]);
		if (ret)
			return i;
	}
	return -ENODEV;
}

static int xlnx_snd_probe(struct platform_device *pdev)
{
	u32 i;
	int ret, audio_interface;
	struct snd_soc_dai_link *dai;
	struct pl_card_data *prv;
	struct platform_device *iface_pdev;

	struct snd_soc_card *card = &xlnx_card;
	struct device_node **node = pdev->dev.platform_data;

	/*
	 * TODO:support multi instance of sound card later. currently,
	 * single instance supported.
	 */
	if (!node || card->instantiated)
		return -ENODEV;

	card->dev = &pdev->dev;

	card->dai_link = devm_kzalloc(card->dev,
				      sizeof(*dai) * XLNX_MAX_PATHS,
				      GFP_KERNEL);
	if (!card->dai_link)
		return -ENOMEM;

	prv = devm_kzalloc(card->dev,
			   sizeof(struct pl_card_data),
			   GFP_KERNEL);
	if (!prv)
		return -ENOMEM;

	card->num_links = 0;
	for (i = XLNX_PLAYBACK; i < XLNX_MAX_PATHS; i++) {
		struct device_node *pnode = of_parse_phandle(node[i],
							     "xlnx,snd-pcm", 0);
		if (!pnode) {
			dev_err(card->dev, "platform node not found\n");
			of_node_put(pnode);
			return -ENODEV;
		}

		/*
		 * Check for either playback or capture is enough, as
		 * same clock is used for both.
		 */
		if (i == XLNX_PLAYBACK) {
			iface_pdev = of_find_device_by_node(pnode);
			if (!iface_pdev) {
				of_node_put(pnode);
				return -ENODEV;
			}

			prv->mclk = devm_clk_get(&iface_pdev->dev, "aud_mclk");
			if (IS_ERR(prv->mclk))
				return PTR_ERR(prv->mclk);
		}
		of_node_put(pnode);

		dai = &card->dai_link[i];
		audio_interface = find_link(node[i], i);
		switch (audio_interface) {
		case I2S_AUDIO:
			*dai = xlnx_snd_dai[I2S_AUDIO][i];
			dai->platform_of_node = pnode;
			dai->cpu_of_node = node[i];
			card->num_links++;
			snd_soc_card_set_drvdata(card, prv);
			dev_dbg(card->dev, "%s registered\n",
				card->dai_link[i].name);
			break;
		case HDMI_AUDIO:
			*dai = xlnx_snd_dai[HDMI_AUDIO][i];
			dai->platform_of_node = pnode;
			if (i == XLNX_CAPTURE)
				dai->codec_of_node = node[i];
			card->num_links++;
			/* TODO: support multiple sampling rates */
			prv->mclk_ratio = 384;
			snd_soc_card_set_drvdata(card, prv);
			dev_dbg(card->dev, "%s registered\n",
				card->dai_link[i].name);
			break;
		case SDI_AUDIO:
			*dai = xlnx_snd_dai[SDI_AUDIO][i];
			dai->platform_of_node = pnode;
			dai->codec_of_node = node[i];
			card->num_links++;
			/* TODO: support multiple sampling rates */
			prv->mclk_ratio = 384;
			snd_soc_card_set_drvdata(card, prv);
			dev_dbg(card->dev, "%s registered\n",
				card->dai_link[i].name);
			break;
		default:
			dev_err(card->dev, "Invalid audio interface\n");
			return -ENODEV;
		}
	}

	if (card->num_links) {
		ret = devm_snd_soc_register_card(card->dev, card);
		if (ret) {
			dev_err(card->dev, "%s registration failed\n",
				card->name);
			return ret;
		}
	}
	dev_info(card->dev, "%s registered\n", card->name);

	return 0;
}

static struct platform_driver xlnx_snd_driver = {
	.driver = {
		.name = "xlnx_snd_card",
	},
	.probe = xlnx_snd_probe,
};

module_platform_driver(xlnx_snd_driver);

MODULE_DESCRIPTION("Xilinx FPGA sound card driver");
MODULE_AUTHOR("Maruthi Srinivas Bayyavarapu");
MODULE_LICENSE("GPL v2");
