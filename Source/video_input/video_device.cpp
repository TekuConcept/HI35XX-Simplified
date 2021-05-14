/**
 * Created by TekuConcept on May 4, 2021
 */

#include <cstdio>
#include <cstring>
#include <stdexcept>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "hi_mipi.h"
#include "hi_common.h"
#include "mpi_sys.h"
#include "mpi_vi.h"

#include "video_input/video_device.h"
#include "mpp_utils.h"

using namespace hisilicon;


video_device::video_device(media_system* __buffer)
: m_media_system(__buffer),
  m_device_id(0),
  m_lane_divide_mode(LANE_DIVIDE_MODE_0)
{
    if (m_media_system == nullptr)
        throw std::runtime_error("video_buffer is NULL");

    try { _M_stop_device();      } catch (...) { }
    try { _M_stop_mipi_sensor(); } catch (...) { }

    _M_set_mipi_info();
    _M_set_device_info();
    _M_start_mipi_sensor();
    _M_configure_as_offline();
    try { _M_start_device(); }
    catch (...) { _M_stop_mipi_sensor(); throw; }
}


video_device::~video_device()
{
    try { _M_stop_device();      } catch (...) { }
    try { _M_stop_mipi_sensor(); } catch (...) { }
}


void
video_device::_M_set_mipi_info()
{
    memset(&m_mipi_info, 0, sizeof(combo_dev_attr_t));

    // -- mipi info for the gc2053 sensor --
    m_mipi_info.devno                     = 0;
    m_mipi_info.input_mode                = INPUT_MODE_MIPI;
    m_mipi_info.data_rate                 = MIPI_DATA_RATE_X1;
    m_mipi_info.img_rect.x                = 0;
    m_mipi_info.img_rect.y                = 0;
    m_mipi_info.img_rect.width            = 1920;
    m_mipi_info.img_rect.height           = 1080;
    m_mipi_info.mipi_attr.input_data_type = DATA_TYPE_RAW_10BIT;
    m_mipi_info.mipi_attr.wdr_mode        = HI_MIPI_WDR_MODE_NONE;
    m_mipi_info.mipi_attr.lane_id[0]      = 0;
    m_mipi_info.mipi_attr.lane_id[1]      = 1;
    m_mipi_info.mipi_attr.lane_id[2]      = -1;
    m_mipi_info.mipi_attr.lane_id[3]      = -1;
}


void
video_device::_M_set_device_info()
{
    memset(&m_device_info, 0, sizeof(VI_DEV_ATTR_S));

    m_device_info.enIntfMode                                = VI_MODE_MIPI; // interface mode
    m_device_info.enWorkMode                                = VI_WORK_MODE_1Multiplex;
    m_device_info.au32ComponentMask[0]                      = 0xFFC00000u;
    m_device_info.au32ComponentMask[1]                      = 0x00000000u;
    m_device_info.enScanMode                                = VI_SCAN_PROGRESSIVE;
    m_device_info.as32AdChnId[0]                            = -1;
    m_device_info.as32AdChnId[1]                            = -1;
    m_device_info.as32AdChnId[2]                            = -1;
    m_device_info.as32AdChnId[3]                            = -1;
    m_device_info.enDataSeq                                 = VI_DATA_SEQ_YUYV;
    m_device_info.stSynCfg.enVsync                          = VI_VSYNC_PULSE;
    m_device_info.stSynCfg.enVsyncNeg                       = VI_VSYNC_NEG_LOW;
    m_device_info.stSynCfg.enHsync                          = VI_HSYNC_VALID_SINGNAL;
    m_device_info.stSynCfg.enHsyncNeg                       = VI_HSYNC_NEG_HIGH;
    m_device_info.stSynCfg.enVsyncValid                     = VI_VSYNC_VALID_SINGAL;
    m_device_info.stSynCfg.enVsyncValidNeg                  = VI_VSYNC_VALID_NEG_HIGH;
    m_device_info.stSynCfg.stTimingBlank.u32HsyncHfb        = 0;
    m_device_info.stSynCfg.stTimingBlank.u32HsyncAct        = 1280;
    m_device_info.stSynCfg.stTimingBlank.u32HsyncHbb        = 0;
    m_device_info.stSynCfg.stTimingBlank.u32VsyncVfb        = 0;
    m_device_info.stSynCfg.stTimingBlank.u32VsyncVact       = 720;
    m_device_info.stSynCfg.stTimingBlank.u32VsyncVbb        = 0;
    m_device_info.stSynCfg.stTimingBlank.u32VsyncVbfb       = 0;
    m_device_info.stSynCfg.stTimingBlank.u32VsyncVbact      = 0;
    m_device_info.stSynCfg.stTimingBlank.u32VsyncVbbb       = 0;
    m_device_info.enInputDataType                           = VI_DATA_TYPE_RGB;
    m_device_info.bDataReverse                              = HI_FALSE;
    m_device_info.stSize.u32Width                           = m_mipi_info.img_rect.width;
    m_device_info.stSize.u32Height                          = m_mipi_info.img_rect.height;
    m_device_info.stBasAttr.stSacleAttr.stBasSize.u32Width  = m_mipi_info.img_rect.width;
    m_device_info.stBasAttr.stSacleAttr.stBasSize.u32Height = m_mipi_info.img_rect.height;
    m_device_info.stBasAttr.stRephaseAttr.enHRephaseMode    = VI_REPHASE_MODE_NONE;
    m_device_info.stBasAttr.stRephaseAttr.enVRephaseMode    = VI_REPHASE_MODE_NONE;
    m_device_info.stWDRAttr.enWDRMode                       = WDR_MODE_NONE;
    m_device_info.stWDRAttr.u32CacheLine                    = m_mipi_info.img_rect.height;
    m_device_info.enDataRate                                = DATA_RATE_X1;
}


void
video_device::_M_configure_as_offline()
{
    int result;
    std::string message;
    VI_VPSS_MODE_S vi2vpss_mode;

    result = HI_MPI_SYS_GetVIVPSSMode(&vi2vpss_mode);
    if (result != HI_SUCCESS) {
        message = "HI_MPI_SYS_GetVIVPSSMode: ";
        goto error;
    }

    // for (int i = 0; i < VI_MAX_PIPE_NUM; i++)
    vi2vpss_mode.aenMode[0] = VI_OFFLINE_VPSS_OFFLINE;

    result = HI_MPI_SYS_SetVIVPSSMode(&vi2vpss_mode);
    if (result != HI_SUCCESS) {
        message = "HI_MPI_SYS_SetVIVPSSMode: ";
        goto error;
    }

    return;

    error:
    throw std::runtime_error(message + mpp_err_string(result));
}


void
video_device::_M_start_mipi_sensor()
{
    int result;
    int mipi_fd;
    std::string message;

    mipi_fd = open("/dev/hi_mipi", O_RDWR);
    if (mipi_fd < 0)
        throw std::runtime_error("failed to open /dev/hi_mipi");

    result = ioctl(mipi_fd, HI_MIPI_SET_HS_MODE, &m_lane_divide_mode);
    if (result != HI_SUCCESS) {
        message = "HI_MIPI_SET_HS_MODE: ";
        goto error;
    }

    result = ioctl(mipi_fd, HI_MIPI_ENABLE_MIPI_CLOCK, &m_mipi_info.devno);
    if (result != HI_SUCCESS) {
        message = "HI_MIPI_ENABLE_MIPI_CLOCK: ";
        goto error;
    }

    result = ioctl(mipi_fd, HI_MIPI_RESET_MIPI, &m_mipi_info.devno);
    if (result != HI_SUCCESS) {
        message = "HI_MIPI_RESET_MIPI: ";
        goto error;
    }

    for (int clock = 0; clock < SNS_MAX_CLK_SOURCE_NUM; clock++) {
        result = ioctl(mipi_fd, HI_MIPI_ENABLE_SENSOR_CLOCK, &clock);
        if (result != HI_SUCCESS) {
            message = "HI_MIPI_ENABLE_SENSOR_CLOCK(";
            message += std::to_string(clock) + std::string("): ");
            goto error;
        }
    }

    for (int clock = 0; clock < SNS_MAX_RST_SOURCE_NUM; clock++) {
        result = ioctl(mipi_fd, HI_MIPI_RESET_SENSOR, &clock);
        if (result != HI_SUCCESS) {
            message = "HI_MIPI_RESET_SENSOR(";
            message += std::to_string(clock) + std::string("): ");
            goto error;
        }
    }

    result = ioctl(mipi_fd, HI_MIPI_SET_DEV_ATTR, &m_mipi_info);
    if (result != HI_SUCCESS) {
        message = "HI_MIPI_SET_DEV_ATTR: ";
        goto error;
    }

    result = ioctl(mipi_fd, HI_MIPI_UNRESET_MIPI, &m_mipi_info.devno);
    if (result != HI_SUCCESS) {
        message = "HI_MIPI_UNRESET_MIPI: ";
        goto error;
    }

    for (int clock = 0; clock < SNS_MAX_RST_SOURCE_NUM; clock++) {
        result = ioctl(mipi_fd, HI_MIPI_UNRESET_SENSOR, &clock);
        if (result != HI_SUCCESS) {
            message = "HI_MIPI_UNRESET_SENSOR(";
            message += std::to_string(clock) + std::string("): ");
            goto error;
        }
    }

    close(mipi_fd);
    return;

    error:
    if (mipi_fd >= 0) close(mipi_fd);
    throw std::runtime_error(message + mpp_err_string(result));
}


void
video_device::_M_stop_mipi_sensor()
{
    int result;
    int mipi_fd;
    std::string message;

    mipi_fd = open("/dev/hi_mipi", O_RDWR);
    if (mipi_fd < 0) return;

    for (int clock = 0; clock < SNS_MAX_RST_SOURCE_NUM; clock++) {
        result = ioctl(mipi_fd, HI_MIPI_RESET_SENSOR, &clock);
        if (result != HI_SUCCESS) {
            message = "HI_MIPI_RESET_SENSOR: ";
            goto error;
        }
    }

    for (int clock = 0; clock < SNS_MAX_CLK_SOURCE_NUM; clock++) {
        result = ioctl(mipi_fd, HI_MIPI_DISABLE_SENSOR_CLOCK, &clock);
        if (result != HI_SUCCESS) {
            message = "HI_MIPI_DISABLE_SENSOR_CLOCK: ";
            goto error;
        }
    }

    result = ioctl(mipi_fd, HI_MIPI_RESET_MIPI, &m_mipi_info.devno);
    if (result != HI_SUCCESS) {
        message = "HI_MIPI_RESET_MIPI: ";
        goto error;
    }

    result = ioctl(mipi_fd, HI_MIPI_DISABLE_MIPI_CLOCK, &m_mipi_info.devno);
    if (result != HI_SUCCESS) {
        message = "HI_MIPI_DISABLE_MIPI_CLOCK: ";
        goto error;
    }

    close(mipi_fd);
    return;

    error:
    if (mipi_fd >= 0) close(mipi_fd);
    throw std::runtime_error(message + mpp_err_string(result));
}


void
video_device::_M_start_device()
{
    int result;
    std::string message;

    result = HI_MPI_VI_SetDevAttr(m_device_id, &m_device_info);
    if (result != HI_SUCCESS) {
        message = "HI_MPI_VI_SetDevAttr: ";
        goto error;
    }

    result = HI_MPI_VI_EnableDev(m_device_id);
    if (result != HI_SUCCESS) {
        message = "HI_MPI_VI_EnableDev: ";
        goto error;
    }

    return;

    error:
    throw std::runtime_error(message + mpp_err_string(result));
}


void
video_device::_M_stop_device()
{
    int result = HI_MPI_VI_DisableDev(m_device_id);
    if (result != HI_SUCCESS)
        throw std::runtime_error(
            std::string("HI_MPI_VI_DisableDev: ") +
            mpp_err_string(result));
}
