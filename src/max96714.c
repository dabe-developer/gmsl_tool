/**
 * @file   max96714.c
 * @author DAB-Embedded
 * @date   25 Nov 2024
 * @brief  MAX96714 function source.
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

int max96714_init(pdeserializer_ctx pctx)
{
    uint8_t reg8 = 0;

    DESER_CTX_CHECK(pctx);
    (void)reg8;

    /* 400kHz I2C, 16ms timeout */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x4C, 0x15);
    /* 400kHz I2C, 16ms timeout */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x4D, 0x55);

    /* RLMS45 */
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x1445, &reg8);
    reg8 &= ~1;
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x1445, reg8);

    /* RX0 Counting Video packets only */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x002C, 0x01);

    /* GPIO2 - Trigger input */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x02B6, 0x23);
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x02B7, 0xa8);
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x02B8, 0x48);
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x02C9, 0x02);
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x02CA, 0x02);

    pctx->features = FEATURE_DES_6GBPS | FEATURE_DES_3GBPS;

    return 0;
}

int max96714_start(pdeserializer_ctx pctx)
{
    DESER_CTX_CHECK(pctx);

    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0313, 0x02);

    /* VIDEO_PIPE_EN to 1 */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0160, 0x01);

    return 0;
}

int max96714_set_mipi_tx_params(pdeserializer_ctx pctx,
        int port, int lanes, uint16_t lane_mapping, uint16_t lane_polarity, int deskew_en, int out_freq, int tunnel_mode_en)
{
    uint8_t reg8 = 0;

    DESER_CTX_CHECK(pctx);

    if ((lanes == 0) || (lanes > 4)) lanes = 4;

    /* BACKTOP : BACKTOP12 | CSI_OUT_EN (CSI_OUT_EN): CSI output disabled */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0313, 0x00);

    /* VIDEO_PIPE_SEL stream 0 */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0161, 0x00);

    /* MIPI_TX10: Set Lane count */
    reg8 = ((lanes - 1) << 6) | 0x10;
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x044A, reg8);

    /* MIPI_PHY3 */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0333, lane_mapping & 0xFF);
    /* MIPI_PHY5 */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0335, lane_polarity & 0x3F);

    i2c_write_reg8a16(pctx->i2c_slave_address, 0x1D00, 0xF4);
    /* BACKTOP25: Set MIPI TX speed */
    reg8 = 0x20;
    if (out_freq <= 80) {
        /* 0 value */
    } else {
        reg8 |= ((out_freq / 100) & 0x1F);
    }
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0320, reg8);
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x1D00, 0xF5);

    if (tunnel_mode_en != 0)
    {
        /* MIPI_TX52 - Tunnel ON */
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0474, 0x09);
        /* Reset link */
        //i2c_write_reg8a16(pctx->i2c_slave_address, 0x0010, 0x31);
        //usleep(500 * 100);
    } else {
        /* MIPI_TX52 - Tunnel OFF */
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0474, 0x08);
        /* Reset link */
        //i2c_write_reg8a16(pctx->i2c_slave_address, 0x0010, 0x31);
        //usleep(500 * 100);

        /* Enable mapping */
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x044C, 0x0F);

        /* SRC_0 */
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x044D, 0x2B);
        /* DST_0 */
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x044E, 0x2B);

        /* SRC_1 */
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x044F, 0x2C);
        /* DST_1 */
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0450, 0x2C);

        /* SRC_0 */
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0451, 0x00);
        /* DST_0 */
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0452, 0x00);

        /* SRC_1 */
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0453, 0x01);
        /* DST_1 */
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0454, 0x01);

        /* Mapping to controller 1 */
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x046D, 0x55);
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

int max96714_wait_for_link(pdeserializer_ctx pctx)
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

int max96714_set_link_speed_gbps(pdeserializer_ctx pctx, int speed)
{
    uint8_t reg8 = 0;

    DESER_CTX_CHECK(pctx);

    switch(speed)
    {
    case 3:
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x147F, 0x68);
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x147E, 0xA8);
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x14A3, 0x30);
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x14D8, 0x07);
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x14A5, 0x70);

        /* Enable 3G */
        i2c_read_reg8a16(pctx->i2c_slave_address, 0x0001, &reg8);
        reg8 &= ~0x3;
        reg8 |= 0x1;
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0001, reg8);

        printf("MAX96714 prepared for 3G.\r\n");
        break;
    case 6:
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x143F, 0x3D);
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x143E, 0xFD);
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x1449, 0xF5);
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x14A3, 0x30);
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x14D8, 0x07);
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x14A5, 0x70);

        /* Enable 6G */
        i2c_read_reg8a16(pctx->i2c_slave_address, 0x0001, &reg8);
        reg8 &= ~0x3;
        reg8 |= 0x2;
        i2c_write_reg8a16(pctx->i2c_slave_address, 0x0001, reg8);

        printf("MAX96714 prepared for 6G.\r\n");
        break;
    case 12:
        return -EINVAL;
    default:
        break;
    }

    return 0;
}

int max96714_reset_link(pdeserializer_ctx pctx)
{
    DESER_CTX_CHECK(pctx);

    /* Reset one shot */
    i2c_write_reg8a16(pctx->i2c_slave_address, 0x0010, 0x31);

    return 0;
}

int max96714_get_stats(pdeserializer_ctx pctx)
{
    uint8_t reg8 = 0;

    DESER_CTX_CHECK(pctx);

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0013, &reg8);
    pctx->deser_stats[0].global_status = reg8 & 0x04 ? -1 : 0;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0308, &reg8);
    printf("REG0308: 0x%02X\r\n", reg8);
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x011a, &reg8);
    printf("REG011A: 0x%02X\r\n", reg8);
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0342, &reg8);
    printf("REG0342: 0x%02X\r\n", reg8);
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0344, &reg8);
    printf("REG0344: 0x%02X\r\n", reg8);
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0019, &reg8);
    printf("REG0019: 0x%02X\r\n", reg8);
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x001b, &reg8);
    printf("REG001B: 0x%02X\r\n", reg8);
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x001d, &reg8);
    printf("REG001D: 0x%02X\r\n", reg8);
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x001f, &reg8);
    printf("REG001F: 0x%02X\r\n", reg8);
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0313, &reg8);
    printf("REG0313: 0x%02X\r\n", reg8);
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0007, &reg8);
    printf("REG0007: 0x%02X\r\n", reg8);
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0008, &reg8);
    printf("REG0008: 0x%02X\r\n", reg8);
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x000D, &reg8);
    printf("REG000D: 0x%02X\r\n", reg8);
    i2c_read_reg8a16(pctx->i2c_slave_address, 0x000E, &reg8);
    printf("REG000E: 0x%02X\r\n", reg8);

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x001B, &reg8);
    pctx->deser_stats[0].remote_error_flag = reg8 & (1 << 5) ? 1 : 0;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x001F, &reg8);
    pctx->deser_stats[0].lcrc_error_flag = (reg8 & 0x8) ? 1 : 0;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x011A, &reg8);
    pctx->deser_stats[0].video_rx_blk_len_err_flag    = (reg8 & 0x80) ? 1 : 0;
    pctx->deser_stats[0].video_pipeline_locked_flag   = (reg8 & 0x40) ? 1 : 0;
    pctx->deser_stats[0].sufficient_video_rx_thr_flag = (reg8 & 0x20) ? 1 : 0;
    pctx->deser_stats[0].video_seq_error_flag         = (reg8 & 0x10) ? 1 : 0;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x011C, &reg8);
    pctx->deser_stats[0].video_rx_overflow_flag = (reg8 & 0x80) ? 1 : 0;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0341, &reg8);
    pctx->deser_stats[0].video_rx_tun_overflow_flag = reg8 & 1;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0342, &reg8);
    pctx->deser_stats[0].csi2_tx_packets_count = reg8 & 0x0F;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0344, &reg8);
    pctx->deser_stats[0].mipi_phy_packets_count = (reg8 >> 4) & 0x0F;

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0025, &reg8);
    pctx->deser_stats[0].global_pkt_count = reg8;
    printf("REG0025: 0x%02X\r\n", reg8);

    i2c_read_reg8a16(pctx->i2c_slave_address, 0x0474, &reg8);
    pctx->deser_stats[0].video_tunnel_flag = reg8 & (1 << 0) ? 1 : 0;

    return 0;
}
