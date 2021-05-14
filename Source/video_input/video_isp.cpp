/**
 * Created by TekuConcept on May 6, 2021
 */

#include <stdexcept>

#include "mpi_ae.h"
#include "mpi_awb.h"
#include "mpi_isp.h"
#include "hi_sns_ctrl.h"

#include "mpp_utils.h"
#include "video_input/video_isp.h"

using namespace hisilicon;


video_isp::video_isp(
    video_device*  __device,
    video_pipe*    __pipe,
    video_channel* __channel)
: m_device(__device),
  m_pipeline(__pipe),
  m_channel(__channel),
  m_bus_id(0),
  m_isp_id(0),
  m_bus_type(ISP_SNS_I2C_TYPE)
{
    if (m_device == nullptr || m_pipeline == nullptr || m_channel == nullptr)
        throw std::runtime_error("one or more parameters NULL");

    try { _M_stop_isp();  } catch (...) { }

    _M_set_isp_info();
    _M_start_isp();
}


video_isp::~video_isp()
{ try { _M_stop_isp();  } catch (...) { } }


void
video_isp::_M_set_isp_info()
{
    std::string ae_name  = "hisi_ae_lib";
    std::string awb_name = "hisi_awb_lib";

    memset(&m_isp_info, 0, sizeof(ISP_PUB_ATTR_S));
    memset(&m_bus_info, 0, sizeof(ISP_SNS_COMMBUS_U));

    m_isp_info.stWndRect.s32X      = m_device->mipi_info().img_rect.x;
    m_isp_info.stWndRect.s32Y      = m_device->mipi_info().img_rect.y;
    m_isp_info.stWndRect.u32Width  = m_device->mipi_info().img_rect.width;
    m_isp_info.stWndRect.u32Height = m_device->mipi_info().img_rect.height;
    m_isp_info.stSnsSize.u32Width  = m_device->mipi_info().img_rect.width;;
    m_isp_info.stSnsSize.u32Height = m_device->mipi_info().img_rect.height;
    m_isp_info.f32FrameRate        = 30;
    m_isp_info.enBayer             = BAYER_RGGB;
    m_isp_info.enWDRMode           = m_device->device_info().stWDRAttr.enWDRMode;
    m_isp_info.u8SnsMode           = 0;

    strncpy(m_ae_lib.acLibName,  ae_name.c_str(),  ae_name.size());
    strncpy(m_awb_lib.acLibName, awb_name.c_str(), awb_name.size());
    m_bus_info.s8I2cDev = m_bus_id;
}


void
video_isp::_M_start_isp()
{
    int result;
    std::string message;

    result = stSnsGc2053Obj.pfnRegisterCallback(
        m_isp_id, &m_ae_lib, &m_awb_lib);
    if (result != HI_SUCCESS) {
        message = "pfnRegisterCallback: ";
        goto error;
    }

    result = stSnsGc2053Obj.pfnSetBusInfo(m_isp_id, m_bus_info);
    if (result != HI_SUCCESS) {
        message = "pfnSetBusInfo: ";
        goto error;
    }

    result = HI_MPI_AE_Register(m_isp_id, &m_ae_lib);
    if (result != HI_SUCCESS) {
        message = "HI_MPI_AE_Register: ";
        goto error;
    }

    result = HI_MPI_AWB_Register(m_isp_id, &m_awb_lib);
    if (result != HI_SUCCESS) {
        message = "HI_MPI_AWB_Register: ";
        goto error;
    }

    result = HI_MPI_ISP_MemInit(m_pipeline->id());
    if (result != HI_SUCCESS) {
        message = "HI_MPI_ISP_MemInit: ";
        goto error;
    }

    result = HI_MPI_ISP_SetPubAttr(m_pipeline->id(), &m_isp_info);
    if (result != HI_SUCCESS) {
        message = "HI_MPI_ISP_SetPubAttr: ";
        goto error;
    }

    result = HI_MPI_ISP_Init(m_pipeline->id());
    if (result != HI_SUCCESS) {
        message = "HI_MPI_ISP_Init: ";
        goto error;
    }

    m_isp_thread = std::thread([](ISP_DEV __isp_id){
        int res = HI_MPI_ISP_Run(__isp_id);
        if (res != HI_SUCCESS)
            throw std::runtime_error(
                std::string("HI_MPI_ISP_Run: ") +
                mpp_err_string(res));
    }, m_isp_id);

    return;
    error:
    throw std::runtime_error(message + mpp_err_string(result));
}


void
video_isp::_M_stop_isp()
{
    if (m_isp_thread.joinable()) {
        HI_MPI_ISP_Exit(m_isp_id);
        m_isp_thread.join();
        HI_MPI_AWB_UnRegister(m_isp_id, &m_awb_lib);
        HI_MPI_AE_UnRegister(m_isp_id, &m_ae_lib);
        stSnsGc2053Obj.pfnUnRegisterCallback(
            m_isp_id, &m_ae_lib, &m_awb_lib);
    }
}
