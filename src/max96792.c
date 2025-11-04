/**
 * @file   max96792.c
 * @author DAB-Embedded
 * @date   25 Nov 2024
 * @brief  MAX96792 function source.
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

int max96792_init(pdeserializer_ctx pctx)
{
    uint8_t reg8 = 0;

    DESER_CTX_CHECK(pctx);
    (void)reg8;

    /* RX0 Counting Video packets only */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x002C, 0x01);
 
    /* Coax drive */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0011, 0x0F);

    pctx->features = FEATURE_DES_12GBPS | FEATURE_DES_6GBPS;

    return 0;
}

int max96792_start(pdeserializer_ctx pctx)
{
    DESER_CTX_CHECK(pctx);

    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0313, 0x02);

    /* VIDEO_PIPE_EN to 1 */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0160, 0x03);

    return 0;
}

int max96792_set_mipi_tx_params(pdeserializer_ctx pctx,
        int port, int lanes, uint16_t lane_mapping, uint16_t lane_polarity, int deskew_en, int out_freq, int tunnel_mode_en)
{
    uint8_t reg8 = 0;

    DESER_CTX_CHECK(pctx);

    /* BACKTOP : BACKTOP12 | CSI_OUT_EN (CSI_OUT_EN): CSI output disabled */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0313, 0x00);

    /* VIDEO_PIPE_SEL stream Y -> Link A streamID 0, Z-> Link B steamID 1 */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0161, 0x28);

    /* MIPI_TX10: Set Lane count */
    reg8 = ((lanes - 1) << 6) | 0x10;
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x044A, reg8);
    
    /* MIPI_PHY3 */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0333, lane_mapping & 0xFF);
    /* MIPI_PHY4 */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0334, lane_mapping & 0xFF);
    /* MIPI_PHY5 */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0335, lane_polarity & 0x3F);
    /* MIPI_PHY6 */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0336, lane_polarity & 0x3F);

    reg8 = 0x20;
    if (out_freq <= 80) {
        /* 0 value */
    } else {
        reg8 |= ((out_freq / 100) & 0x1F);
    }
    /* BACKTOP22: Set MIPI TX speed */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x031D, reg8);
    /* BACKTOP25: Set MIPI TX speed */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0320, reg8);

    if (tunnel_mode_en != 0)
    {
        /* MIPI_TX52 - Tunnel ON */
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0474, 0x09);
        /* Reset link */
        //i2c_write_reg8a16(pctx->i2c_slave_address, 0x0010, 0x31);
        //usleep(500 * 100);
    }

    usleep(MIPI_TX_DELAY);

    if (deskew_en != 0) {
        /* MIPI_TX3 */
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0443, 0x81);
        /* MIPI_TX4 */
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0444, 0xB9);
        /* MIPI_TX50 - VC0 only */
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0472, 0x80);
    }
    /* MIPI PHY16 - Reporting on */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0340, 0x39);

    /* LCRC on */
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0112, &reg8);
    reg8 |= (1 << 1);
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0112, reg8);

    return 0;
}

int max96792_wait_for_link(pdeserializer_ctx pctx)
{
    uint8_t reg8 = 0;
    int i;

    DESER_CTX_CHECK(pctx);

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

int max96792_set_link_speed_gbps(pdeserializer_ctx pctx, int speed)
{
    uint8_t reg8 = 0;
    
    DESER_CTX_CHECK(pctx);

    /* Keep link in reset */
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0010, &reg8);
    reg8 |= (1 << 6);
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0010, reg8);

    switch(speed)
    {
    case 3:
        /* Enable 3G */
        i2c_read_reg8a16(pctx->i2c_slave_address, 0x0001, &reg8);
        reg8 &= ~0x3;
        reg8 |= (1 << 0);
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0001, reg8);

        printf("MAX96792A prepared for 3G.\r\n");
        break;
    case 6:
        /* Enable 6G */
        i2c_read_reg8a16(pctx->i2c_slave_address, 0x0001, &reg8);
        reg8 &= ~0x3;
        reg8 |= (2 << 0);
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0001, reg8);

        /* Disable FEC */
        i2c_read_reg8a16(pctx->i2c_slave_address,  0x0028, &reg8);
        reg8 &= ~(1 << 1);
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0028, reg8);

        /* Enable GMSL2 */
        i2c_read_reg8a16(pctx->i2c_slave_address,  0x0004, &reg8);
        reg8 &= ~(3 << 6);
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0004, reg8);
        

        printf("MAX96792A prepared for 6G.\r\n");
        break;
    case 12:
        /* Enable 12G */
        i2c_read_reg8a16(pctx->i2c_slave_address, 0x0001, &reg8);
        reg8 &= ~0x3;
        reg8 |= (3 << 0);
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0001, reg8);

        /* Enable FEC */
        i2c_read_reg8a16(pctx->i2c_slave_address,  0x0028, &reg8);
        reg8 |= (1 << 1);
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0028, reg8);

        /* Disable GMSL2 */
        i2c_read_reg8a16(pctx->i2c_slave_address,  0x0004, &reg8);
        reg8 |= (3 << 6);
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0004, reg8);

        printf("MAX96792A prepared for 12G.\r\n");
        break;
    default:
        break;
    }

    /* Release link */
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0010, &reg8);
    reg8 &= ~(1 << 6);
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0010, reg8);

    return 0;
}

int max96792_reset_link(pdeserializer_ctx pctx)
{
    uint8_t reg8 = 0;

    DESER_CTX_CHECK(pctx);

    /* Reset one shot A */
    i2c_read_reg8a16(pctx->i2c_slave_address,  0x0010, &reg8);
    reg8 |= (1 << 5);
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0010, reg8);

    /* Reset one shot B */
    //i2c_read_reg8a16(pctx->i2c_slave_address,  0x0012, &reg8);
    //reg8 |= (1 << 5);
    //i2c_write_reg8a16(pctx->i2c_slave_address, 0x0012, reg8);

    return 0;
}

int max96792_get_stats(pdeserializer_ctx pctx)
{
    uint8_t reg8 = 0;

    DESER_CTX_CHECK(pctx);

    /* RX0 Counting Video packets only */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x002C, 0x01);

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0013, &reg8);
    pctx->deser_stats[0].global_status = reg8 & 0x04 ? -1 : 0;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x001B, &reg8);
    pctx->deser_stats[0].remote_error_flag = reg8 & (1 << 5) ? 1 : 0;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x011A, &reg8);
    pctx->deser_stats[0].video_rx_blk_len_err_flag    = (reg8 & 0x80) ? 1 : 0;
    pctx->deser_stats[0].video_pipeline_locked_flag   = (reg8 & 0x40) ? 1 : 0;
    pctx->deser_stats[0].sufficient_video_rx_thr_flag = (reg8 & 0x20) ? 1 : 0;
    pctx->deser_stats[0].video_seq_error_flag         = (reg8 & 0x10) ? 1 : 0;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x012C, &reg8);
    pctx->deser_stats[1].video_rx_blk_len_err_flag    = (reg8 & 0x80) ? 1 : 0;
    pctx->deser_stats[1].video_pipeline_locked_flag   = (reg8 & 0x40) ? 1 : 0;
    pctx->deser_stats[1].sufficient_video_rx_thr_flag = (reg8 & 0x20) ? 1 : 0;
    pctx->deser_stats[1].video_seq_error_flag         = (reg8 & 0x10) ? 1 : 0;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x011C, &reg8);
    pctx->deser_stats[0].video_rx_overflow_flag = (reg8 & 0x80) ? 1 : 0;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x012E, &reg8);
    pctx->deser_stats[1].video_rx_overflow_flag = (reg8 & 0x80) ? 1 : 0;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0341, &reg8);
    pctx->deser_stats[0].video_rx_tun_overflow_flag = reg8 & 1;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0342, &reg8);
    pctx->deser_stats[0].csi2_tx_packets_count = reg8 & 0x0F;
    pctx->deser_stats[1].csi2_tx_packets_count = (reg8 >> 4) & 0x0F;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0344, &reg8);
    pctx->deser_stats[0].mipi_phy_packets_count = reg8;
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0345, &reg8);
    pctx->deser_stats[1].mipi_phy_packets_count = reg8;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0025, &reg8);
    pctx->deser_stats[0].global_pkt_count = reg8;
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0025, 0x00);

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0474, &reg8);
    pctx->deser_stats[0].video_tunnel_flag = reg8 & (1 << 0) ? 1 : 0;

    return 0;
}
