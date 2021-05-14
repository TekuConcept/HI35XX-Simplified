/**
 * Created by TekuConcept on May 5, 2021
 */

#ifndef HISILICON_MPP_TYPES_H
#define HISILICON_MPP_TYPES_H

namespace hisilicon {

    enum class pixel_format_t {
        RGB_444 = 0,
        RGB_555,
        RGB_565,
        RGB_888,

        BGR_444,
        BGR_555,
        BGR_565,
        BGR_888,

        ARGB_1555,
        ARGB_4444,
        ARGB_8565,
        ARGB_8888,
        ARGB_2BPP,

        ABGR_1555,
        ABGR_4444,
        ABGR_8565,
        ABGR_8888,

        RGB_BAYER_8BPP,
        RGB_BAYER_10BPP,
        RGB_BAYER_12BPP,
        RGB_BAYER_14BPP,
        RGB_BAYER_16BPP,

        YVU_PLANAR_422,
        YVU_PLANAR_420,
        YVU_PLANAR_444,

        YVU_SEMIPLANAR_422,
        YVU_SEMIPLANAR_420,
        YVU_SEMIPLANAR_444,

        YUV_SEMIPLANAR_422,
        YUV_SEMIPLANAR_420,
        YUV_SEMIPLANAR_444,

        YUYV_PACKAGE_422,
        YVYU_PACKAGE_422,
        UYVY_PACKAGE_422,
        VYUY_PACKAGE_422,
        YYUV_PACKAGE_422,
        YYVU_PACKAGE_422,
        UVYY_PACKAGE_422,
        VUYY_PACKAGE_422,

        YUV_400,
        UV_420,

        BGR_888_PLANAR,
        HSV_888_PACKAGE,
        HSV_888_PLANAR,
        LAB_888_PACKAGE,
        LAB_888_PLANAR,
        S8C1,
        S8C2_PACKAGE,
        S8C2_PLANAR,
        S8C3_PLANAR,
        S16C1,
        U8C1,
        U16C1,
        S32C1,
        U32C1,
        U64C1,
        S64C1
    };

    enum class payload_type_t {
        PCMU          = 0,
        /* 1: reserved */
        G721          = 2,
        GSM           = 3,
        G723          = 4,
        DVI4_8K       = 5,
        DVI4_16K      = 6,
        LPC           = 7,
        PCMA          = 8,
        G722          = 9,
        S16BE_STEREO  = 10,
        S16BE_MONO    = 11,
        QCELP         = 12,
        CN            = 13,
        MPEGAUDIO     = 14,
        G728          = 15,
        DVI4_3        = 16,
        DVI4_4        = 17,
        G729          = 18,
        G711A         = 19,
        G711U         = 20,
        G726          = 21,
        G729A         = 22,
        LPCM          = 23,
        CelB          = 25,
        JPEG          = 26,
        CUSM          = 27,
        NV            = 28,
        PICW          = 29,
        CPV           = 30,
        H261          = 31,
        MPEGVIDEO     = 32,
        MPEG2TS       = 33,
        H263          = 34,
        SPEG          = 35,
        MPEG2VIDEO    = 36,
        AAC           = 37,
        WMA9STD       = 38,
        HEAAC         = 39,
        PCM_VOICE     = 40,
        PCM_AUDIO     = 41,
        MP3           = 43,
        ADPCMA        = 49,
        AEC           = 50,
        X_LD          = 95,
        H264          = 96,
        D_GSM_HR      = 200,
        D_GSM_EFR     = 201,
        D_L8          = 202,
        D_RED         = 203,
        D_VDVI        = 204,
        D_BT656       = 220,
        D_H263_1998   = 221,
        D_MP1S        = 222,
        D_MP2P        = 223,
        D_BMPEG       = 224,
        MP4VIDEO      = 230,
        MP4AUDIO      = 237,
        VC1           = 238,
        JVC_ASF       = 255,
        D_AVI         = 256,
        DIVX3         = 257,
        AVS           = 258,
        REAL8         = 259,
        REAL9         = 260,
        VP6           = 261,
        VP6F          = 262,
        VP6A          = 263,
        SORENSON      = 264,
        H265          = 265,
        VP8           = 266,
        MVC           = 267,
        PNG           = 268,
        /* add by hisilicon */
        AMR           = 1001,
        MJPEG         = 1002,
        AMRWB         = 1003,
        PRORES        = 1006,
        OPUS          = 1007,
    };

    enum class buffer_iface_t {
        YUV_FRAME,
        RAW_FRAME,
        ENCODED_FRAME
    };

    enum class bit_width_t {
        B8 = 0,
        B10,
        B12,
        B14,
        B16,
    };

} /* namespace hisilicon */

#endif /* HISILICON_MPP_TYPES_H */
