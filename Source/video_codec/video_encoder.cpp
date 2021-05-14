/**
 * Created by TekuConcept on May 12, 2021
 */

#include "video_codec/video_encoder.h"

#include <cstring>
#include <string>
#include <stdexcept>
#include "mpi_venc.h"
#include "mpp_utils.h"

using namespace hisilicon;


video_encoder::video_encoder(
    HI_U32 __width,
    HI_U32 __height,
    HI_U32 __gop)
: m_channel_id(0),
  m_gop(__gop)
{
    m_pic_size.u32Width  = __width;
    m_pic_size.u32Height = __height;

    memset(&m_info, 0, sizeof(VENC_CHN_ATTR_S));

    _M_set_encoder_info();
    _M_set_bitrate_control_info();
    _M_set_group_of_pics_info();

    _M_start_encoder();
}


video_encoder::~video_encoder()
{
    try { _M_stop_encoder(); } catch (...) { }
}


void
video_encoder::_M_set_encoder_info()
{
    m_info.stVencAttr.enType          = PT_H264;
    m_info.stVencAttr.u32MaxPicWidth  = m_pic_size.u32Width;
    m_info.stVencAttr.u32MaxPicHeight = m_pic_size.u32Height;
    m_info.stVencAttr.u32BufSize      = m_pic_size.u32Width * m_pic_size.u32Height * 2;
    m_info.stVencAttr.u32Profile      = 0;
    m_info.stVencAttr.bByFrame        = HI_TRUE;
    m_info.stVencAttr.u32PicWidth     = m_pic_size.u32Width;
    m_info.stVencAttr.u32PicHeight    = m_pic_size.u32Height;
}


void
video_encoder::_M_set_bitrate_control_info()
{
    VENC_H264_CBR_S *info = &m_info.stRcAttr.stH264Cbr;

    m_info.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;

    info->u32Gop           = m_gop;
    info->u32StatTime      = 1;
    info->u32SrcFrameRate  = 30/*fps*/;
    info->fr32DstFrameRate = 30/*fps*/;

    if (m_pic_size.u32Width = 1280 && m_pic_size.u32Height == 720)
        info->u32BitRate = 1024 * 3 + 1024;
    else if (m_pic_size.u32Width = 1920 && m_pic_size.u32Height == 1080)
        info->u32BitRate = 1024 * 2 + 2048;
    else if (m_pic_size.u32Width = 2592 && m_pic_size.u32Height == 1944)
        info->u32BitRate = 1024 * 4 + 3072;
    else if (m_pic_size.u32Width = 3840 && m_pic_size.u32Height == 2160)
        info->u32BitRate = 1024 * 8 + 5120;
    else if (m_pic_size.u32Width = 4000 && m_pic_size.u32Height == 3000)
        info->u32BitRate = 1024 * 12 + 5120;
    else if (m_pic_size.u32Width = 7680 && m_pic_size.u32Height == 4320)
        info->u32BitRate = 1024 * 24 + 5120;
    else info->u32BitRate = 1024 * 24 + 5120;
}


void
video_encoder::_M_set_group_of_pics_info()
{
    VENC_GOP_ATTR_S *info = &m_info.stGopAttr;
    info->enGopMode = VENC_GOPMODE_NORMALP;
    info->stNormalP.s32IPQpDelta = 2;
}


void
video_encoder::_M_start_encoder()
{
    int result;
    std::string message;
    VENC_RC_PARAM_S bitrate_control;
    VENC_RECV_PIC_PARAM_S recv_count;

    result = HI_MPI_VENC_CreateChn(m_channel_id, &m_info);
    if (result != HI_SUCCESS) {
        message = "HI_MPI_VENC_CreateChn: ";
        goto error;
    }

    {
        result = HI_MPI_VENC_GetRcParam(m_channel_id, &bitrate_control);
        if (result != HI_SUCCESS) {
            message = "HI_MPI_VENC_GetRcParam: ";
            goto error2;
        }

        bitrate_control.stParamH264Cbr.s32MaxReEncodeTimes = 0;

        result = HI_MPI_VENC_SetRcParam(m_channel_id, &bitrate_control);
        if (result != HI_SUCCESS) {
            message = "HI_MPI_VENC_SetRcParam: ";
            goto error2;
        }
    }

    recv_count.s32RecvPicNum = -1; // continuous
    result = HI_MPI_VENC_StartRecvFrame(m_channel_id, &recv_count);
    if (result != HI_SUCCESS) {
        message = "HI_MPI_VENC_StartRecvFrame: ";
        goto error2;
    }

    return;
    error2:
    HI_MPI_VENC_DestroyChn(m_channel_id);
    error:
    throw std::runtime_error(message + mpp_err_string(result));
}


void
video_encoder::_M_stop_encoder()
{
    int result;
    std::string message;

    result = HI_MPI_VENC_StopRecvFrame(m_channel_id);
    if (result != HI_SUCCESS) {
        message = "HI_MPI_VENC_StopRecvFrame: ";
        goto error;
    }

    result = HI_MPI_VENC_DestroyChn(m_channel_id);
    if (result != HI_SUCCESS) {
        message = "HI_MPI_VENC_DestroyChn: ";
        goto error;
    }

    return;
    error:
    throw std::runtime_error(message + mpp_err_string(result));
}
