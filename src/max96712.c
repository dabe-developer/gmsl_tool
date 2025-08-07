/**
 * @file   max96712.c
 * @author DAB-Embedded
 * @date   25 Nov 2024
 * @brief  MAX96712 function source.
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

#define DESER_CTX_CHECK(x) if (x == NULL) { return -EFAULT; }

#define MIPI_TX_DELAY     (100 * 1000)
#define MIPI_RST_TIME     (150 * 1000)
#define LINK_WAIT_TIME    (20) /* 2 sec */

int max96712_init(pdeserializer_ctx pctx)
{
    uint8_t reg8 = 0;

    DESER_CTX_CHECK(pctx);
    (void)reg8;

    pctx->features = FEATURE_DES_6GBPS | FEATURE_DES_3GBPS;

    return 0;
}

int max96712_start(pdeserializer_ctx pctx)
{
    DESER_CTX_CHECK(pctx);

    return 0;
}

int max96712_set_mipi_tx_params(pdeserializer_ctx pctx,
        int port, int lanes, uint16_t lane_mapping, uint16_t lane_polarity, int deskew_en, int out_freq, int tunnel_mode_en)
{
    uint8_t reg8 = 0;

    DESER_CTX_CHECK(pctx);
    (void)reg8;

    return 0;
}

int max96712_wait_for_link(pdeserializer_ctx pctx)
{
    uint8_t reg8 = 0;
    (void)reg8;

    DESER_CTX_CHECK(pctx);

    return 0;
}

int max96712_set_link_speed_gbps(pdeserializer_ctx pctx, int speed)
{

    DESER_CTX_CHECK(pctx);

    switch(speed)
    {
    case 3:

        break;
    case 6:

        break;
    case 12:
        return -EINVAL;
    default:
        break;
    }

    return 0;
}

int max96712_reset_link(pdeserializer_ctx pctx)
{
    DESER_CTX_CHECK(pctx);

    return 0;
}

int max96712_get_stats(pdeserializer_ctx pctx)
{
    uint8_t reg8 = 0;

    DESER_CTX_CHECK(pctx);

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x001A, &reg8);
    pctx->deser_stats[0].global_status = reg8 & 0x04 ? -1 : 0;
    pctx->deser_stats[1].global_status = pctx->deser_stats[0].global_status;
    pctx->deser_stats[2].global_status = pctx->deser_stats[0].global_status;
    pctx->deser_stats[3].global_status = pctx->deser_stats[0].global_status;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x002A, &reg8);
    pctx->deser_stats[0].remote_error_flag = reg8 & (1 << 1) ? 1 : 0;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0108, &reg8);
    pctx->deser_stats[0].video_rx_blk_len_err_flag    = (reg8 & 0x80) ? 1 : 0;
    pctx->deser_stats[0].video_pipeline_locked_flag   = (reg8 & 0x40) ? 1 : 0;
    pctx->deser_stats[0].sufficient_video_rx_thr_flag = (reg8 & 0x20) ? 1 : 0;
    pctx->deser_stats[0].video_seq_error_flag         = (reg8 & 0x10) ? 1 : 0;
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x011A, &reg8);
    pctx->deser_stats[1].video_rx_blk_len_err_flag    = (reg8 & 0x80) ? 1 : 0;
    pctx->deser_stats[1].video_pipeline_locked_flag   = (reg8 & 0x40) ? 1 : 0;
    pctx->deser_stats[1].sufficient_video_rx_thr_flag = (reg8 & 0x20) ? 1 : 0;
    pctx->deser_stats[1].video_seq_error_flag         = (reg8 & 0x10) ? 1 : 0;
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x012C, &reg8);
    pctx->deser_stats[2].video_rx_blk_len_err_flag    = (reg8 & 0x80) ? 1 : 0;
    pctx->deser_stats[2].video_pipeline_locked_flag   = (reg8 & 0x40) ? 1 : 0;
    pctx->deser_stats[2].sufficient_video_rx_thr_flag = (reg8 & 0x20) ? 1 : 0;
    pctx->deser_stats[2].video_seq_error_flag         = (reg8 & 0x10) ? 1 : 0;
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x013E, &reg8);
    pctx->deser_stats[3].video_rx_blk_len_err_flag    = (reg8 & 0x80) ? 1 : 0;
    pctx->deser_stats[3].video_pipeline_locked_flag   = (reg8 & 0x40) ? 1 : 0;
    pctx->deser_stats[3].sufficient_video_rx_thr_flag = (reg8 & 0x20) ? 1 : 0;
    pctx->deser_stats[2].video_seq_error_flag         = (reg8 & 0x10) ? 1 : 0;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x040A, &reg8);
    pctx->deser_stats[0].video_rx_overflow_flag = (reg8 & 0x10) ? 1 : 0;
    pctx->deser_stats[1].video_rx_overflow_flag = (reg8 & 0x20) ? 1 : 0;
    pctx->deser_stats[2].video_rx_overflow_flag = (reg8 & 0x40) ? 1 : 0;
    pctx->deser_stats[3].video_rx_overflow_flag = (reg8 & 0x80) ? 1 : 0;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x08D0, &reg8);
    pctx->deser_stats[0].csi2_tx_packets_count = reg8 & 0x0F;
    pctx->deser_stats[1].csi2_tx_packets_count = (reg8 >> 4) & 0x0F;
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x08D1, &reg8);
    pctx->deser_stats[2].csi2_tx_packets_count = reg8 & 0x0F;
    pctx->deser_stats[3].csi2_tx_packets_count = (reg8 >> 4) & 0x0F;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x08D2, &reg8);
    pctx->deser_stats[0].mipi_phy_packets_count = reg8 & 0x0F;
    pctx->deser_stats[1].mipi_phy_packets_count = (reg8 >> 4) & 0x0F;
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x08D3, &reg8);
    pctx->deser_stats[2].mipi_phy_packets_count = reg8 & 0x0F;
    pctx->deser_stats[3].mipi_phy_packets_count = (reg8 >> 4) & 0x0F;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0040, &reg8);
    pctx->deser_stats[0].global_pkt_count = reg8;
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0041, &reg8);
    pctx->deser_stats[1].global_pkt_count = reg8;
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0042, &reg8);
    pctx->deser_stats[2].global_pkt_count = reg8;
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0043, &reg8);
    pctx->deser_stats[3].global_pkt_count = reg8;

    return 0;
}
