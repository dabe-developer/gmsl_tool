/**
 * @file   serializer.c
 * @author DAB-Embedded
 * @date   21 Nov 2024
 * @brief  Serializer source.
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

const serializer_entry serializers[] = {
  /* MAX96717 */
  {
    .ser_devid             = 0xBF,
    .ser_name              = "MAX96717",
    .rx_ports              = 1,
    .init                  = max96717_init,
    .start                 = max96717_start,
    .set_mipi_rx_params    = max96717_set_mipi_rx_params,
    .reset_link            = max96717_reset_link,
    .wait_for_link         = max96717_wait_for_link,
    .set_link_speed_gbps   = max96717_set_link_speed_gbps,
    .get_stats             = max96717_get_stats,
  },
  /* MAX9295D */
  {
    .ser_devid             = 0x95,
    .ser_name              = "MAX9295D",
    .rx_ports              = 2,
    .init                  = max9295d_init,
    .start                 = max9295d_start,
    .set_mipi_rx_params    = max9295d_set_mipi_rx_params,
    .reset_link            = max9295d_reset_link,
    .wait_for_link         = max9295d_wait_for_link,
    .set_link_speed_gbps   = max9295d_set_link_speed_gbps,
    .get_stats             = max9295d_get_stats,
  },
};

const uint8_t serializer_default_i2c_slave[] =
{
  0x42U, 0x40U
};

/* Variables */
static serializer_ctx ser_content;
static int found_inx_ser = -1;

int serializer_search_chip(void)
{
    uint8_t reg8 = 0, sa_ser = 0;
    int i;

    if (found_inx_ser >= 0) return 0;

    if (app_params.ser_i2c_sa == 0)
    {
        for (i = 0; i < ARRAY_SIZE(serializer_default_i2c_slave); i++)
        {
            sa_ser = serializer_default_i2c_slave[i];

            if (i2c_read_reg8a16(sa_ser, 0x000D, &reg8) == 0)
            {
                ser_content.i2c_slave_address = sa_ser;
                break;
            }
        }
    } else {
        sa_ser = app_params.ser_i2c_sa;

        if (i2c_read_reg8a16(sa_ser, 0x000D, &reg8) < 0)
        {
            goto serializer_search_chip_end;
        }

        ser_content.i2c_slave_address = sa_ser;
    }

    for (i = 0; i < ARRAY_SIZE(serializers); i++)
    {
        if (reg8 == serializers[i].ser_devid)
        {
            /* Found */
            found_inx_ser = i;
            break;
        }
    }

serializer_search_chip_end:
    return (found_inx_ser >= 0) ? 0 : -1;
}

int serializer_init(uint32_t features)
{
    int ret = 0, selected_speed = 0;

    serializer_search_chip();

    if (found_inx_ser < 0) return -1;

    ret = serializers[found_inx_ser].init(&ser_content);
    if (ret < 0)
        return ret;

    ret = serializers[found_inx_ser].set_mipi_rx_params(
            &ser_content, 0,
            app_params.mipi_rx_lanes,
            app_params.mipi_rx_map,
            app_params.mipi_rx_pol,
            app_params.mipi_rx_skew_en, 1);
    if (ret < 0)
        return ret;

    if (features & FEATURE_DES_12GBPS)
    {
        ret = serializers[found_inx_ser].set_link_speed_gbps(&ser_content, 12);
        if (ret < 0)
        {
            if (features & FEATURE_DES_6GBPS)
            {
                ret = serializers[found_inx_ser].set_link_speed_gbps(&ser_content, 6);
                if (ret < 0)
                {
                    ret = serializers[found_inx_ser].set_link_speed_gbps(&ser_content, 3);
                    if (ret < 0)
                        return ret;
                    selected_speed = 3;
                } else selected_speed = 6;
            } else {
                ret = serializers[found_inx_ser].set_link_speed_gbps(&ser_content, 3);
                if (ret < 0)
                    return ret;
                selected_speed = 3;
            }
        } else selected_speed = 12;
    } else if (features & FEATURE_DES_6GBPS) {
        ret = serializers[found_inx_ser].set_link_speed_gbps(&ser_content, 6);
        if (ret < 0)
        {
            ret = serializers[found_inx_ser].set_link_speed_gbps(&ser_content, 3);
            if (ret < 0)
                return ret;
            selected_speed = 3;
        } else selected_speed = 6;
    } else {
        ret = serializers[found_inx_ser].set_link_speed_gbps(&ser_content, 3);
        if (ret < 0)
            return ret;
        selected_speed = 3;
    }

    ret = serializers[found_inx_ser].reset_link(&ser_content);
    if (ret < 0)
        return ret;

    ret = deserializer_set_link_speed(selected_speed);
    if (ret < 0)
        return ret;

    ret = serializers[found_inx_ser].wait_for_link(&ser_content);
    if (ret < 0)
        return ret;

    ret = serializers[found_inx_ser].start(&ser_content);
    if (ret < 0)
        return ret;

    if (ret == 0)
    {
        printf("Found serializer %s\r\n", serializers[found_inx_ser].ser_name);
    }

    return ret;
}

int serializer_get_stat(void)
{
    int i;

    serializers[found_inx_ser].get_stats(&ser_content);

    printf("======== Statistics [Ser] ================\r\n");

    for (i = 0; i < serializers[found_inx_ser].rx_ports; i++)
    {
        printf("Stats of MIPI RX channel #%d\r\n", i);

        printf("Status: %s\r\n",
                (ser_content.ser_stats[i].global_status == -1) ? "Error" : "OK");

        if (ser_content.ser_stats[i].tx_fifo_warn_flag)
            printf("TX Warning detected\r\n");

        if (ser_content.ser_stats[i].tx_fifo_overflow_flag)
            printf("TX Overflow detected\r\n");

        if (ser_content.ser_stats[i].tx_pclk_drift_flag)
            printf("TX Drift detected\r\n");

        if (ser_content.ser_stats[i].tx_pclk_det_flag)
            printf("TX Clock detected\r\n");

        printf("MIPI RX L LP status: %x\r\n", ser_content.ser_stats[i].mipi_rx_l_lp_errors);

        printf("MIPI RX L HS error: %x\r\n", ser_content.ser_stats[i].mipi_rx_l_hs_errors);

        printf("MIPI RX H LP status: %x\r\n", ser_content.ser_stats[i].mipi_rx_h_lp_errors);

        printf("MIPI RX H HS error: %x\r\n", ser_content.ser_stats[i].mipi_rx_h_hs_errors);

        printf("CTRL1 CSI L status: %x\r\n", ser_content.ser_stats[i].ctrl1_csi_l_errors);

        printf("CTRL1 CSI H error: %x\r\n", ser_content.ser_stats[i].ctrl1_csi_h_errors);

        if (ser_content.ser_stats[i].mipi_dphy_rx_count != -1)
            printf("MIPI DHPY1 packets received: %d\r\n", ser_content.ser_stats[i].mipi_dphy_rx_count);

        if (ser_content.ser_stats[i].mipi_pkt_processed != -1)
            printf("MIPI CSI1 packets processed: %d\r\n", ser_content.ser_stats[i].mipi_pkt_processed);

        if (ser_content.ser_stats[i].mipi_clk_rx_count != -1)
            printf("MIPI CSI1 clock received: %d\r\n", ser_content.ser_stats[i].mipi_clk_rx_count);

        printf("Mode: %s\r\n",
                ser_content.ser_stats[i].is_tunnel_mode ? "Tunnel" : "Pixel");

        if (ser_content.ser_stats[i].is_tunnel_mode != 0)
        {
            printf("Tunnel status: %s\r\n",
                    ser_content.ser_stats[i].is_tunnel_overflow ? "Overflow" : "OK");

            printf("MIPI tunnel packets processed: %d\r\n", ser_content.ser_stats[i].tunnel_pkt_processed);
        }

        printf("Packet counter   : %d\r\n", ser_content.ser_stats[i].global_pkt_count);

    }

    return 0;
}
