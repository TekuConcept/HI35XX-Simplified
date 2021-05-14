/**
 * Created by TekuConcept on May 4, 2021
 */

#include <cstring>
#include <iostream>
#include <stdexcept>
#include "hi_buffer.h"
#include "mpi_sys.h"
#include "media_system.h"
#include "mpp_utils.h"

using namespace hisilicon;


media_system::pool_t::pool_t()
: width(1920), height(1080), block_count(10),
  pixel_format(pixel_format_t::YVU_SEMIPLANAR_420),
  buffer_iface(buffer_iface_t::YUV_FRAME),
  payload_type(payload_type_t::H264),
  data_bit_width(bit_width_t::B8)
{ }


media_system::media_system(const std::vector<pool_t>& __pools)
{
    int result;

    try { _M_stop_system();       } catch (...) { }
    try { _M_stop_video_buffer(); } catch (...) { }

    _M_set_video_buffer_data(__pools);
    _M_start_video_buffer();
    try { _M_start_system(); }
    catch (...) { _M_stop_video_buffer(); throw; }
}


media_system::~media_system()
{
    try { _M_stop_system();       } catch (...) { }
    try { _M_stop_video_buffer(); } catch (...) { }
}


void
media_system::_M_set_video_buffer_data(const std::vector<pool_t>& __pools)
{
    if (__pools.size() > VB_MAX_COMM_POOLS)
        throw std::runtime_error(
            std::string("number of pools exceeds maximum: ") +
            std::to_string(VB_MAX_COMM_POOLS));

    memset(&m_vb_config, 0, sizeof(VB_CONFIG_S));
    m_vb_config.u32MaxPoolCnt = __pools.size();

    for (size_t i = 0; i < __pools.size(); i++) {
        m_vb_config.astCommPool[i].u32BlkCnt = __pools[i].block_count;
        switch (__pools[i].buffer_iface) {
        case buffer_iface_t::YUV_FRAME: {
            auto u32BlkSize = COMMON_GetPicBufferSize(
                __pools[i].width,
                __pools[i].height,
                (PIXEL_FORMAT_E)__pools[i].pixel_format,
                (DATA_BITWIDTH_E)__pools[i].data_bit_width,
                COMPRESS_MODE_NONE,
                DEFAULT_ALIGN);
            m_vb_config.astCommPool[i].u64BlkSize  = u32BlkSize;
        } break;
        case buffer_iface_t::RAW_FRAME: {
            m_vb_config.astCommPool[i].u64BlkSize = VI_GetRawBufferSize(
                __pools[i].width,
                __pools[i].height,
                (PIXEL_FORMAT_E)__pools[i].pixel_format,
                COMPRESS_MODE_NONE,
                DEFAULT_ALIGN);
        } break;
        case buffer_iface_t::ENCODED_FRAME: {
            m_vb_config.astCommPool[i].u64BlkSize = VENC_GetRefBufferSize(
                (PAYLOAD_TYPE_E)__pools[i].payload_type,
                __pools[i].width,
                __pools[i].height,
                (DATA_BITWIDTH_E)__pools[i].data_bit_width,
                DEFAULT_ALIGN);
        } break;
        }
    }
}


void
media_system::_M_start_video_buffer()
{
    int result;
    std::string message;

    HI_MPI_VB_Exit();

    result = HI_MPI_VB_SetConfig(&m_vb_config);
    if (result != HI_SUCCESS) {
        message = "HI_MPI_VB_SetConfig: ";
        goto error;
    }

    result = HI_MPI_VB_Init();
    if (result != HI_SUCCESS) {
        message = "HI_MPI_VB_Init: ";
        goto error;
    }

    return;

    error:
    throw std::runtime_error(message + mpp_err_string(result));
}


void
media_system::_M_stop_video_buffer()
{
    int result = HI_MPI_VB_Exit();
    if (result != HI_SUCCESS)
        throw std::runtime_error(
            std::string("HI_MPI_VB_Exit: ") +
            mpp_err_string(result));
}


void
media_system::_M_start_system()
{
    int result = HI_MPI_SYS_Init();
    if (result != HI_SUCCESS)
        throw std::runtime_error(
            std::string("HI_MPI_SYS_Init: ") +
            mpp_err_string(result));
}


void
media_system::_M_stop_system()
{
    int result = HI_MPI_SYS_Exit();
    if (result != HI_SUCCESS)
        throw std::runtime_error(
            std::string("HI_MPI_SYS_Exit: ") +
            mpp_err_string(result));
}
