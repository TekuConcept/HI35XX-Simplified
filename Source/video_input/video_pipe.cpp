/**
 * Created by TekuConcept on May 5, 2021
 */

#include <cstring>
#include <string>
#include <iostream>
#include <stdexcept>

#include "mpi_vi.h"

#include "mpp_utils.h"
#include "video_input/video_pipe.h"

using namespace hisilicon;
#define V(x) std::cout << x << std::endl
#define X(s) std::cout << #s ": " << s << std::endl


video_pipe::video_pipe(video_device* __video_device)
: m_video_device(__video_device),
  m_pipe_id(0)
{
    if (m_video_device == nullptr)
        throw std::runtime_error("video_device is NULL");

    try { _M_stop_pipe(); } catch (...) { }

    _M_set_pipe_info();
    _M_bind_to_device();
    _M_start_pipe();
}


video_pipe::~video_pipe()
{ try { _M_stop_pipe(); } catch (...) { } }


void
video_pipe::_M_set_pipe_info()
{
    memset(&m_pipe_info, 0, sizeof(VI_PIPE_ATTR_S));

    m_pipe_info.enPipeBypassMode            = VI_PIPE_BYPASS_NONE;
    m_pipe_info.bYuvSkip                    = HI_FALSE;
    m_pipe_info.bIspBypass                  = HI_FALSE;
    m_pipe_info.u32MaxW                     = m_video_device->mipi_info().img_rect.width;
    m_pipe_info.u32MaxH                     = m_video_device->mipi_info().img_rect.height;
    m_pipe_info.enPixFmt                    = PIXEL_FORMAT_RGB_BAYER_10BPP;
    m_pipe_info.enCompressMode              = COMPRESS_MODE_LINE;
    m_pipe_info.enBitWidth                  = DATA_BITWIDTH_10;
    m_pipe_info.bNrEn                       = HI_TRUE;
    m_pipe_info.stNrAttr.enPixFmt           = PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    m_pipe_info.stNrAttr.enBitWidth         = DATA_BITWIDTH_8;
    m_pipe_info.stNrAttr.enNrRefSource      = VI_NR_REF_FROM_RFR;
    m_pipe_info.stNrAttr.enCompressMode     = COMPRESS_MODE_NONE;
    m_pipe_info.bSharpenEn                  = HI_FALSE;
    m_pipe_info.stFrameRate.s32SrcFrameRate = -1;
    m_pipe_info.stFrameRate.s32DstFrameRate = -1;
    m_pipe_info.bDiscardProPic              = HI_FALSE;
}


void
video_pipe::_M_bind_to_device()
{
    VI_DEV_BIND_PIPE_S bind_info;
    bind_info.u32Num    = 1;
    bind_info.PipeId[0] = m_pipe_id;
    int result = HI_MPI_VI_SetDevBindPipe(m_video_device->id(), &bind_info);
    if (result != HI_SUCCESS)
        throw std::runtime_error(
            std::string("HI_MPI_VI_SetDevBindPipe: ") +
            mpp_err_string(result));
}


void
video_pipe::_M_start_pipe()
{
    int result;
    std::string message;

    result = HI_MPI_VI_CreatePipe(m_pipe_id, &m_pipe_info);
    if (result != HI_SUCCESS) {
        message = "HI_MPI_VI_CreatePipe: ";
        goto error;
    }

    result = HI_MPI_VI_StartPipe(m_pipe_id);
    if (result != HI_SUCCESS) {
        HI_MPI_VI_DestroyPipe(m_pipe_id);
        message = "HI_MPI_VI_StartPipe: ";
        goto error;
    }

    return;

    error:
    throw std::runtime_error(message + mpp_err_string(result));
}


void
video_pipe::_M_stop_pipe()
{
    int result;
    std::string message;

    result = HI_MPI_VI_StopPipe(m_pipe_id);
    if (result != HI_SUCCESS) {
        message = "HI_MPI_VI_StopPipe: ";
        goto error;
    }

    result = HI_MPI_VI_DestroyPipe(m_pipe_id);
    if (result != HI_SUCCESS) {
        message = "HI_MPI_VI_DestroyPipe: ";
        goto error;
    }

    return;

    error:
    throw std::runtime_error(message + mpp_err_string(result));
}
