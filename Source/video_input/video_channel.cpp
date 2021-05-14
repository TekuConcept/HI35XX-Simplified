/**
 * Created by TekuConcept on May 5, 2021
 */

#include <cstring>
#include <string>
#include <stdexcept>
#include "mpi_vi.h"
#include "mpp_utils.h"
#include "video_input/video_channel.h"

using namespace hisilicon;


video_channel::video_channel(video_pipe* __pipeline)
: m_pipe(__pipeline),
  m_channel_id(0)
{
    if (m_pipe == nullptr)
        throw std::runtime_error("video_pipe is NULL");
    else m_device = m_pipe->sensor();

    try { _M_stop_channel(); } catch (...) { }

    _M_set_channel_info();
    _M_start_channel();
}


video_channel::~video_channel()
{ try { _M_stop_channel(); } catch (...) { } }


void
video_channel::_M_set_channel_info()
{
    memset(&m_channel_info, 0, sizeof(VI_CHN_ATTR_S));

    m_channel_info.stSize.u32Width             = m_device->mipi_info().img_rect.width;
    m_channel_info.stSize.u32Height            = m_device->mipi_info().img_rect.height;
    m_channel_info.enPixelFormat               = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    m_channel_info.enDynamicRange              = DYNAMIC_RANGE_SDR8;
    m_channel_info.enVideoFormat               = VIDEO_FORMAT_LINEAR;
    m_channel_info.enCompressMode              = COMPRESS_MODE_NONE;
    m_channel_info.bMirror                     = HI_FALSE;
    m_channel_info.bFlip                       = HI_FALSE;
    m_channel_info.u32Depth                    = 1;
    m_channel_info.stFrameRate.s32SrcFrameRate = -1;
    m_channel_info.stFrameRate.s32DstFrameRate = -1;
}


void
video_channel::_M_start_channel()
{
    int result;
    std::string message;

    result = HI_MPI_VI_SetChnAttr(m_pipe->id(), m_channel_id, &m_channel_info);
    if (result != HI_SUCCESS) {
        message = "HI_MPI_VI_SetChnAttr: ";
        goto error;
    }

    result = HI_MPI_VI_EnableChn(m_pipe->id(), m_channel_id);
    if (result != HI_SUCCESS) {
        message = "HI_MPI_VI_EnableChn";
        goto error;
    }

    return;
    error:
    throw std::runtime_error(message + mpp_err_string(result));
}


void
video_channel::_M_stop_channel()
{
    int result = HI_MPI_VI_DisableChn(m_pipe->id(), m_channel_id);
    if (result != HI_SUCCESS)
        throw std::runtime_error(
            std::string("HI_MPI_VI_DisableChn: ") +
            mpp_err_string(result));
}
