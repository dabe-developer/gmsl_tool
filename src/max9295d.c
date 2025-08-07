/**
 * @file   max9295d.c
 * @author DAB-Embedded
 * @date   21 Nov 2024
 * @brief  MAX96717 function source.
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

#define SER_CTX_CHECK(x) if (x == NULL) { return -EFAULT; }

#define MIPI_RX_DELAY     (100 * 1000)
#define MIPI_RST_TIME     (150 * 1000)
#define LINK_WAIT_TIME    (20)

int max9295d_init(pserializer_ctx pctx)
{
    uint8_t reg8 = 0;

    SER_CTX_CHECK(pctx);

    (void)reg8;

    return 0;
}

int max9295d_start(pserializer_ctx pctx)
{
    SER_CTX_CHECK(pctx);

    return 0;
}

int max9295d_set_mipi_rx_params(pserializer_ctx pctx,
        int port, int lanes, uint16_t lane_mapping, uint16_t lane_polarity, int skew_en, int tunnel_mode_en)
{
    uint8_t reg8 = 0;

    SER_CTX_CHECK(pctx);
    (void)port;
    (void)lanes;
    (void)lane_mapping;
    (void)lane_polarity;
    (void)skew_en;
    (void)tunnel_mode_en;
    (void)reg8;

    return 0;
}

int max9295d_wait_for_link(pserializer_ctx pctx)
{
    uint8_t reg8 = 0;
    int i;

    SER_CTX_CHECK(pctx);

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0013, &reg8);

    if (reg8 & 0x08) return 0;

    i = 0;
    while(i++ < LINK_WAIT_TIME)
    {
        i2c_read_reg8a16(pctx->i2c_slave_address, 0x0013, &reg8);

        if (reg8 & 0x08) return 0;

        usleep(100 * 1000);
    }

    return -1;
}

int max9295d_set_link_speed_gbps(pserializer_ctx pctx, int speed)
{
    uint8_t reg8 = 0;

    SER_CTX_CHECK(pctx);

    switch(speed)
    {
    case 3:
        /* Enable 3G */
        i2c_read_reg8a16(pctx->i2c_slave_address, 0x0001, &reg8);
        reg8 &= ~0xC;
        reg8 |= 0x4;
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0001, reg8);

        printf("MAX9295D prepared for 3G.\r\n");
        break;
    case 6:
        /* Enable 6G */
        i2c_read_reg8a16(pctx->i2c_slave_address, 0x0001, &reg8);
        reg8 &= ~0xC;
        reg8 |= 0x8;
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0001, reg8);

        printf("MAX9295D prepared for 6G.\r\n");
        break;
    case 12:
        return -EINVAL;
    default:
        break;
    }

    return 0;
}

int max9295d_reset_link(pserializer_ctx pctx)
{
    SER_CTX_CHECK(pctx);

    /* Reset one shot */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0010, 0x21);

    return 0;
}

int max9295d_get_stats(pserializer_ctx pctx)
{
    uint8_t reg8 = 0;

    SER_CTX_CHECK(pctx);

    /* RX0 Counting Video packets only */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x002C, 0x01);

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0013, &reg8);
    pctx->ser_stats[0].global_status = reg8 & 0x04 ? -1 : 0;
    pctx->ser_stats[1].global_status = pctx->ser_stats[0].global_status;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0339, &reg8);
    pctx->ser_stats[0].mipi_rx_l_lp_errors = reg8;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x033A, &reg8);
    pctx->ser_stats[0].mipi_rx_l_hs_errors = reg8;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x033B, &reg8);
    pctx->ser_stats[0].mipi_rx_h_lp_errors = reg8;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x033C, &reg8);
    pctx->ser_stats[0].mipi_rx_h_hs_errors = reg8;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x033D, &reg8);
    pctx->ser_stats[1].mipi_rx_l_lp_errors = reg8;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x033E, &reg8);
    pctx->ser_stats[1].mipi_rx_l_hs_errors = reg8;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x033F, &reg8);
    pctx->ser_stats[1].mipi_rx_h_lp_errors = reg8;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0340, &reg8);
    pctx->ser_stats[1].mipi_rx_h_hs_errors = reg8;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0341, &reg8);
    pctx->ser_stats[0].ctrl1_csi_l_errors = reg8;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0342, &reg8);
    pctx->ser_stats[0].ctrl1_csi_h_errors = reg8;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0343, &reg8);
    pctx->ser_stats[1].ctrl1_csi_l_errors = reg8;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0344, &reg8);
    pctx->ser_stats[1].ctrl1_csi_h_errors = reg8;

    pctx->ser_stats[0].mipi_dphy_rx_count = -1;
    pctx->ser_stats[0].mipi_pkt_processed = -1;
    pctx->ser_stats[0].mipi_clk_rx_count  = -1;
    pctx->ser_stats[1].mipi_dphy_rx_count = -1;
    pctx->ser_stats[1].mipi_pkt_processed = -1;
    pctx->ser_stats[1].mipi_clk_rx_count  = -1;

    pctx->ser_stats[0].is_tunnel_mode = 0;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0102, &reg8);
    pctx->ser_stats[0].tx_fifo_warn_flag        = reg8 & (1 << 4) ? 1 : 0;
    pctx->ser_stats[0].tx_fifo_overflow_flag    = reg8 & (1 << 5) ? 1 : 0;
    pctx->ser_stats[0].tx_pclk_drift_flag       = reg8 & (1 << 6) ? 1 : 0;
    pctx->ser_stats[0].tx_pclk_det_flag         = reg8 & (1 << 7) ? 1 : 0;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x010A, &reg8);
    pctx->ser_stats[1].tx_fifo_warn_flag        = reg8 & (1 << 4) ? 1 : 0;
    pctx->ser_stats[1].tx_fifo_overflow_flag    = reg8 & (1 << 5) ? 1 : 0;
    pctx->ser_stats[1].tx_pclk_drift_flag       = reg8 & (1 << 6) ? 1 : 0;
    pctx->ser_stats[1].tx_pclk_det_flag         = reg8 & (1 << 7) ? 1 : 0;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0112, &reg8);
    pctx->ser_stats[2].tx_fifo_warn_flag        = reg8 & (1 << 4) ? 1 : 0;
    pctx->ser_stats[2].tx_fifo_overflow_flag    = reg8 & (1 << 5) ? 1 : 0;
    pctx->ser_stats[2].tx_pclk_drift_flag       = reg8 & (1 << 6) ? 1 : 0;
    pctx->ser_stats[2].tx_pclk_det_flag         = reg8 & (1 << 7) ? 1 : 0;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x011A, &reg8);
    pctx->ser_stats[3].tx_fifo_warn_flag        = reg8 & (1 << 4) ? 1 : 0;
    pctx->ser_stats[3].tx_fifo_overflow_flag    = reg8 & (1 << 5) ? 1 : 0;
    pctx->ser_stats[3].tx_pclk_drift_flag       = reg8 & (1 << 6) ? 1 : 0;
    pctx->ser_stats[3].tx_pclk_det_flag         = reg8 & (1 << 7) ? 1 : 0;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0025, &reg8);
    pctx->ser_stats[0].global_pkt_count = reg8;
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0025, 0x00);

    return 0;
}
