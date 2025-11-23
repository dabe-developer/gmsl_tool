/**
 * @file   max96724.c
 * @author DAB-Embedded
 * @date   21 Nov 2025
 * @brief  MAX96724 function source.
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

int max96724_init(pdeserializer_ctx pctx)
{
    uint8_t reg8 = 0;

    DESER_CTX_CHECK(pctx);

    (void)reg8;

    /* RX0 Counting Video packets only */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x1004, 0x01);
 
    /* Coax drive */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0022, 0xFF);

    /* GMSL2, Enable all links */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0006, 0xFF);

    pctx->features = FEATURE_DES_6GBPS | FEATURE_DES_3GBPS;

    return 0;
}

int max96724_start(pdeserializer_ctx pctx)
{
    DESER_CTX_CHECK(pctx);

    i2c_write_reg8a16(pctx->i2c_slave_address, 0x040B, 0x02);

    /* VIDEO_PIPE_EN to 1 */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x00F4, 0x1F);

    return 0;
}

int max96724_set_mipi_tx_params(pdeserializer_ctx pctx,
        int port, int lanes, uint16_t lane_mapping, uint16_t lane_polarity, int deskew_en, int out_freq, int tunnel_mode_en)
{
    uint8_t reg8 = 0;

    DESER_CTX_CHECK(pctx);

    /* BACKTOP : BACKTOP12 | CSI_OUT_EN (CSI_OUT_EN): CSI output disabled */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x040B, 0x00);

    /* VIDEO_PIPE_SEL stream Y -> Link A streamID 0, Z-> Link B steamID 1 */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x00F0, 0x61);
    /* VIDEO_PIPE_SEL stream X -> Link C streamID 2, U-> Link D steamID 3 */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x00F1, 0xF8);

    /* MIPI_TX10: Set Lane count */
    reg8 = ((lanes - 1) << 6);
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x090A, reg8);
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x094A, reg8);
    
    /* MIPI_PHY3 */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x08A3, lane_mapping & 0xFF);
    /* MIPI_PHY4 */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x08A4, lane_mapping & 0xFF);
    /* MIPI_PHY5 */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x08A5, lane_polarity & 0x3F);
    /* MIPI_PHY6 */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x08A6, lane_polarity & 0x3F);

    reg8 = 0x20;
    if (out_freq <= 80) {
        /* 0 value */
    } else {
        reg8 |= ((out_freq / 100) & 0x1F);
    }
    /* BACKTOP22: Set MIPI TX speed */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0415, reg8);
    /* BACKTOP25: Set MIPI TX speed */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0418, reg8);
    /* BACKTOP28: Set MIPI TX speed */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x041B, reg8);
    /* BACKTOP31: Set MIPI TX speed */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x041E, reg8);

    if (tunnel_mode_en != 0)
    {
        /* MIPI_TX54 - Tunnel ON */
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0936, 0x09);
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0976, 0x09);
        /* Reset link */
        //i2c_write_reg8a16(pctx->i2c_slave_address, 0x0010, 0x31);
        //usleep(500 * 100);
    }

    usleep(MIPI_TX_DELAY);

    if (deskew_en != 0) {
        /* MIPI_TX3 */
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0903, 0x81);
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0943, 0x81);
        /* MIPI_TX4 */
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0904, 0xB9);
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0944, 0xB9);
        /* MIPI_TX50 - VC0 only */
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0932, 0x80);
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0972, 0x80);
    }
    /* MIPI PHY16 - Reporting on */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x08B0, 0x78);

    /* LCRC on */
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0100, &reg8);
    reg8 |= (1 << 1);
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0100, reg8);
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0112, &reg8);
    reg8 |= (1 << 1);
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0112, reg8);
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0124, &reg8);
    reg8 |= (1 << 1);
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0124, reg8);
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0136, &reg8);
    reg8 |= (1 << 1);
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0136, reg8);

    return 0;
}

int max96724_wait_for_link(pdeserializer_ctx pctx)
{
    uint8_t reg8a[4];
    int i;

    DESER_CTX_CHECK(pctx);

    /* CTRL3 */
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x001A, &reg8a[0]);
    /* CTRL12 */
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x000A, &reg8a[1]);
    /* CTRL13 */
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x000B, &reg8a[2]);
    /* CTRL14 */
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x000C, &reg8a[3]);

    if ((reg8a[0] & 0x08) && 
        (reg8a[1] & 0x08) && 
        (reg8a[2] & 0x08) && 
        (reg8a[3] & 0x08))
    {
        return 0;
    }

    i = 0;
    while(i++ < LINK_WAIT_TIME)
    {
        /* CTRL3 */
        i2c_read_reg8a16(pctx->i2c_slave_address, 0x001A, &reg8a[0]);
        /* CTRL12 */
        i2c_read_reg8a16(pctx->i2c_slave_address, 0x000A, &reg8a[1]);
        /* CTRL13 */
        i2c_read_reg8a16(pctx->i2c_slave_address, 0x000B, &reg8a[2]);
        /* CTRL14 */
        i2c_read_reg8a16(pctx->i2c_slave_address, 0x000C, &reg8a[3]);

        if ((reg8a[0] & 0x08) && 
            (reg8a[1] & 0x08) && 
            (reg8a[2] & 0x08) && 
            (reg8a[3] & 0x08))
        {
            break;
        }

        usleep(100 * 1000);
    }

    if ((reg8a[0] & 0x08) ||
        (reg8a[1] & 0x08) || 
        (reg8a[2] & 0x08) || 
        (reg8a[3] & 0x08))
    {
        return 0;
    }

    return -1;
}

int max96724_set_link_speed_gbps(pdeserializer_ctx pctx, int speed)
{
    uint8_t reg8 = 0;

    DESER_CTX_CHECK(pctx);

    /* Keep link in reset */
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0018, &reg8);
    reg8 |= (0xF << 4);
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0018, reg8);

    switch(speed)
    {
    case 3:
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0010, 0x11);
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0011, 0x11);
        
        printf("MAX96724 prepared for 3G.\r\n");
        break;
    case 6:
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0010, 0x22);
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0011, 0x22);
        
        printf("MAX96724 prepared for 6G.\r\n");
        break;
    case 12:
        return -EINVAL;
    default:
        break;
    }

    /* Release link */
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0018, &reg8);
    reg8 &= ~(0xF << 4);
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0018, reg8);

    return 0;
}

int max96724_reset_link(pdeserializer_ctx pctx)
{
    uint8_t reg8 = 0;

    DESER_CTX_CHECK(pctx);

    /* Reset one shot A~D */
    i2c_read_reg8a16(pctx->i2c_slave_address,  0x0018, &reg8);
    reg8 |= (0x0F);
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0018, reg8);

    return 0;
}

int max96724_get_stats(pdeserializer_ctx pctx)
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
