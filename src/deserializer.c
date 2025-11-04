/**
 * @file   deserializer.c
 * @author DAB-Embedded
 * @date   21 Nov 2024
 * @brief  Deserializer source.
 *
 */
#include <stdio.h>
#include <linux/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "serdes_head.h"

const deserializer_entry deserializers[] = {
  /* MAX96714 */
  {
    .deser_devid           = 0xC9,
    .deser_name            = "MAX96714",
    .tx_ports              = 1,
    .init                  = max96714_init,
    .start                 = max96714_start,
    .set_mipi_tx_params    = max96714_set_mipi_tx_params,
    .reset_link            = max96714_reset_link,
    .wait_for_link         = max96714_wait_for_link,
    .set_link_speed_gbps   = max96714_set_link_speed_gbps,
    .get_stats             = max96714_get_stats,
  },
  /* MAX96792 */
  {
    .deser_devid           = 0xB6,
    .deser_name            = "MAX96792",
    .tx_ports              = 2,
    .init                  = max96792_init,
    .start                 = max96792_start,
    .set_mipi_tx_params    = max96792_set_mipi_tx_params,
    .reset_link            = max96792_reset_link,
    .wait_for_link         = max96792_wait_for_link,
    .set_link_speed_gbps   = max96792_set_link_speed_gbps,
    .get_stats             = max96792_get_stats,
  },
  /* MAX96712 */
  {
    .deser_devid           = 0xA0,
    .deser_name            = "MAX96712",
    .tx_ports              = 4,
    .init                  = max96714_init,
    .start                 = max96714_start,
    .set_mipi_tx_params    = max96714_set_mipi_tx_params,
    .reset_link            = max96714_reset_link,
    .wait_for_link         = max96714_wait_for_link,
    .set_link_speed_gbps   = max96714_set_link_speed_gbps,
    .get_stats             = max96714_get_stats,
  },
};

const uint8_t deserializer_default_i2c_slave[] =
{
  0x6AU, 0x28U, 0x29
};

/* Variables */
static deserializer_ctx deser_content;
static int found_inx_deser = -1;

static int deserializer_search_chip(void)
{
    uint8_t reg8 = 0, sa_deser = 0;
    int i;

    if (found_inx_deser >= 0) return 0;

    if (app_params.deser_i2c_sa == 0)
    {
        for (i = 0; i < ARRAY_SIZE(deserializer_default_i2c_slave); i++)
        {
            sa_deser = deserializer_default_i2c_slave[i];

            if (i2c_read_reg8a16(sa_deser, 0x000D, &reg8) == 0)
            {
                deser_content.i2c_slave_address = sa_deser;
                break;
            }
        }
    } else {
        sa_deser = app_params.deser_i2c_sa;

        if (i2c_read_reg8a16(sa_deser, 0x000D, &reg8) < 0)
        {
            goto deserializer_search_chip_end;
        }

        deser_content.i2c_slave_address = sa_deser;
    }

    for (i = 0; i < ARRAY_SIZE(deserializers); i++)
    {
        if (reg8 == deserializers[i].deser_devid)
        {
            /* Found */
            found_inx_deser = i;
            break;
        }
    }

deserializer_search_chip_end:
    return (found_inx_deser >= 0) ? 0 : -1;
}

int deserializer_init(void)
{
    int ret = 0;

    deserializer_search_chip();

    if (found_inx_deser < 0) return -1;

    ret = deserializers[found_inx_deser].init(&deser_content);
    if (ret < 0)
        return ret;

    if (ret == 0)
    {
        printf("Found deserializer %s\r\n", deserializers[found_inx_deser].deser_name);
    }

    return ret;
}

int deserializer_search_for_serializer(void)
{
    int ret = 0;

    if (found_inx_deser < 0) return -1;

    if (deser_content.features & FEATURE_DES_12GBPS)
    {
        ret = deserializers[found_inx_deser].set_link_speed_gbps(&deser_content, 12);
        if (ret < 0)
        {
            ret = deserializers[found_inx_deser].set_link_speed_gbps(&deser_content, 6);
            if (ret < 0)
            {
                ret = deserializers[found_inx_deser].set_link_speed_gbps(&deser_content, 3);
            }
        }
    } else if (deser_content.features & FEATURE_DES_6GBPS)
    {
        ret = deserializers[found_inx_deser].set_link_speed_gbps(&deser_content, 6);
        if (ret < 0)
        {
            ret = deserializers[found_inx_deser].set_link_speed_gbps(&deser_content, 3);
        }
    } else if (deser_content.features & FEATURE_DES_3GBPS) {
        ret = deserializers[found_inx_deser].set_link_speed_gbps(&deser_content, 3);
    }
    if (ret < 0) return ret;

    ret = deserializers[found_inx_deser].wait_for_link(&deser_content);
    if (ret < 0)
    {
        /* Set lowest rate */
        if (deser_content.features & FEATURE_DES_6GBPS)
        {
            ret = deserializers[found_inx_deser].set_link_speed_gbps(&deser_content, 6);
            if (ret < 0) return ret;
        } else if (deser_content.features & FEATURE_DES_3GBPS) {
            ret = deserializers[found_inx_deser].set_link_speed_gbps(&deser_content, 3);
            if (ret < 0) return ret;
        }
        ret = deserializers[found_inx_deser].wait_for_link(&deser_content);
        if (ret < 0) return ret;
    }
    ret = deserializers[found_inx_deser].wait_for_link(&deser_content);
    if (ret < 0)
    {
        if (deser_content.features & FEATURE_DES_3GBPS)
        {
            /* Set 3Gbps rate */
            ret = deserializers[found_inx_deser].set_link_speed_gbps(&deser_content, 3);
            if (ret < 0) return ret;
            ret = deserializers[found_inx_deser].wait_for_link(&deser_content);
        }
    }
    
    if (ret >= 0)
        printf("Link with serializer is OK\r\n");

    return ret;
}

uint32_t deserializer_get_features(void)
{
    if (found_inx_deser < 0) return 0;
    return deser_content.features;
}

int deserializer_set_link_speed(int speed)
{
    int ret = 0;

    if (found_inx_deser < 0) return -1;

    ret = deserializers[found_inx_deser].set_link_speed_gbps(&deser_content, speed);
    if (ret < 0) return ret;

    ret = deserializers[found_inx_deser].wait_for_link(&deser_content);
    if (ret < 0) return ret;

    return ret;
}

int deserializer_start(void)
{
    int ret = 0;

    if (found_inx_deser < 0) return -1;

    ret = deserializers[found_inx_deser].set_mipi_tx_params(&deser_content,
        0, app_params.mipi_tx_lanes,
        app_params.mipi_tx_map,
        app_params.mipi_tx_pol,
        app_params.mipi_tx_deskew_en,
        app_params.mipi_tx_out_freq, 1);
    if (ret < 0) return ret;

    ret = deserializers[found_inx_deser].start(&deser_content);
    if (ret < 0) return ret;

    return ret;
}

int deserializer_get_stat(void)
{
    int i;

    deserializers[found_inx_deser].get_stats(&deser_content);

    printf("======== Statistics [Des] ================\r\n");

    for (i = 0; i < deserializers[found_inx_deser].tx_ports; i++)
    {
        printf("Stats of MIPI TX channel #%d\r\n", i);

        printf("Status: %s\r\n",
                (deser_content.deser_stats[i].global_status == -1) ? "Error" : "OK");

        if (deser_content.deser_stats[i].remote_error_flag)
            printf("Remote error detected\r\n");

        if (deser_content.deser_stats[i].video_rx_blk_len_err_flag)
            printf("Video Rx block-length error detected\r\n");

        if (deser_content.deser_stats[i].video_pipeline_locked_flag)
            printf("Video pipeline locked\r\n");

        if (deser_content.deser_stats[i].video_rx_overflow_flag)
            printf("Video RX overflow\r\n");

        if (deser_content.deser_stats[i].video_rx_tun_overflow_flag)
            printf("Video tunnel overflow\r\n");

        if (deser_content.deser_stats[i].sufficient_video_rx_thr_flag)
            printf("Sufficient throughput detected\r\n");

        if (deser_content.deser_stats[i].video_seq_error_flag)
            printf("Video Rx sequence error detected\r\n");

        if (deser_content.deser_stats[i].lcrc_error_flag)
            printf("LCRC error detected\r\n");

        printf("Mode: %s\r\n",
                deser_content.deser_stats[i].video_tunnel_flag ? "Tunnel" : "Pixel");

        printf("MIPI TX packets  : %d\r\n", deser_content.deser_stats[i].csi2_tx_packets_count);

        printf("MIPI MIPI PHY out: %d\r\n", deser_content.deser_stats[i].mipi_phy_packets_count);

        printf("Packet counter   : %d\r\n", deser_content.deser_stats[i].global_pkt_count);
    }

    return 0;
}
