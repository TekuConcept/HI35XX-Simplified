/**
 * Created by TekuConcept on May 10, 2021
 */

#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include <string>
#include <stdexcept>

#include "hi_defines.h"
#include "acodec.h"
#include "mpi_audio.h"

#include "audio.h"
#include "mpp_utils.h"

using namespace hisilicon;


audio_io::audio_io(media_system* __media_system)
: m_media_system(__media_system),
  m_device_id(0),
  m_channel_id(0)
{
    _M_set_info();
    _M_init_codec();
    _M_start_output();
    _M_start_input();
}


audio_io::~audio_io()
{
    try { _M_stop_input(); } catch (...) { }
    try { _M_stop_output(); } catch (...) { }
}


void
audio_io::_M_set_info()
{
    memset(&m_info, 0, sizeof(AIO_ATTR_S));

    m_info.enSamplerate   = AUDIO_SAMPLE_RATE_48000;
    m_info.enBitwidth     = AUDIO_BIT_WIDTH_16;
    m_info.enWorkmode     = AIO_MODE_I2S_MASTER;
    m_info.enSoundmode    = AUDIO_SOUND_MODE_MONO;
    m_info.u32EXFlag      = 0;    /* no extensions */
    m_info.u32FrmNum      = 30;   /* buffer size */
    m_info.u32PtNumPerFrm = 1024; /* number of samples per frame */
    m_info.u32ChnCnt      = 1;    /* mono */
    m_info.u32ClkSel      = 0;    /* ai and ao share clock */
    m_info.enI2sType      = AIO_I2STYPE_INNERCODEC;
}


void
audio_io::_M_init_codec()
{
    int result, fd;
    std::string message;
    ACODEC_FS_E i2s_fs_sel;

    HI_S32 volume;
    HI_U32 muted;
    HI_U32 power_down;
    ACODEC_VOL_CTRL volume_control;

    fd = open("/dev/acodec", O_RDWR);
    if (fd < 0) throw std::runtime_error("failed to open /dev/acodec");

    result = ioctl(fd, ACODEC_SOFT_RESET_CTRL);
    if (result != HI_SUCCESS) {
        message = "ACODEC_SOFT_RESET_CTRL: ";
        goto error;
    }

    switch (m_info.enSamplerate) {
    case AUDIO_SAMPLE_RATE_8000:  i2s_fs_sel = ACODEC_FS_8000;  break;
    case AUDIO_SAMPLE_RATE_16000: i2s_fs_sel = ACODEC_FS_16000; break;
    case AUDIO_SAMPLE_RATE_32000: i2s_fs_sel = ACODEC_FS_32000; break;
    case AUDIO_SAMPLE_RATE_11025: i2s_fs_sel = ACODEC_FS_11025; break;
    case AUDIO_SAMPLE_RATE_22050: i2s_fs_sel = ACODEC_FS_22050; break;
    case AUDIO_SAMPLE_RATE_44100: i2s_fs_sel = ACODEC_FS_44100; break;
    case AUDIO_SAMPLE_RATE_12000: i2s_fs_sel = ACODEC_FS_12000; break;
    case AUDIO_SAMPLE_RATE_24000: i2s_fs_sel = ACODEC_FS_24000; break;
    case AUDIO_SAMPLE_RATE_48000: i2s_fs_sel = ACODEC_FS_48000; break;
    case AUDIO_SAMPLE_RATE_64000: i2s_fs_sel = ACODEC_FS_64000; break;
    case AUDIO_SAMPLE_RATE_96000: i2s_fs_sel = ACODEC_FS_96000; break;
    default:
        message = std::to_string(m_info.enSamplerate);
        message += std::string(" sample rate not supported");
        result = HI_ERR_AO_NOT_SUPPORT;
        goto error;
    }

    result = ioctl(fd, ACODEC_SET_I2S1_FS, &i2s_fs_sel);
    if (result != HI_SUCCESS) {
        message = "ACODEC_SET_I2S1_FS: ";
        goto error;
    }

    volume = 6/*db*/;
    result = ioctl(fd, ACODEC_SET_OUTPUT_VOL, &volume);
    if (result != HI_SUCCESS) {
        message = "ACODEC_SET_OUTPUT_VOL: ";
        goto error;
    }

    // volume_control.vol_ctrl      = 0x00; // max volume
    // volume_control.vol_ctrl_mute = 0;    // not muted
    // result = ioctl(fd, ACODEC_SET_DACL_VOL, &volume_control);
    // if (result != HI_SUCCESS) {
    //     message = "ACODEC_SET_DACL_VOL: ";
    //     goto error;
    // }
    // result = ioctl(fd, ACODEC_SET_DACR_VOL, &volume_control);
    // if (result != HI_SUCCESS) {
    //     message = "ACODEC_SET_DACR_VOL: ";
    //     goto error;
    // }

    // power_down = 0;
    // result = ioctl(fd, ACODEC_SET_PD_DACL, &power_down);
    // if (result != HI_SUCCESS) {
    //     message = "ACODEC_SET_PD_DACL: ";
    //     goto error;
    // }
    // result = ioctl(fd, ACODEC_SET_PD_DACR, &power_down);
    // if (result != HI_SUCCESS) {
    //     message = "ACODEC_SET_PD_DACR: ";
    //     goto error;
    // }

    return;
    error:
    throw std::runtime_error(message + mpp_err_string(result));
}


void
audio_io::_M_start_input()
{
    int result;
    std::string message;

    result = HI_MPI_AI_SetPubAttr(m_device_id, &m_info);
    if (result != HI_SUCCESS) {
        message = "HI_MPI_AI_SetPubAttr: ";
        goto error;
    }

    result = HI_MPI_AI_Enable(m_device_id);
    if (result != HI_SUCCESS) {
        message = "HI_MPI_AI_Enable: ";
        goto error;
    }

    result = HI_MPI_AI_EnableChn(m_device_id, m_channel_id);
    if (result != HI_SUCCESS) {
        message = "HI_MPI_AI_EnableChn: ";
        goto error;
    }

    return;
    error:
    throw std::runtime_error(message + mpp_err_string(result));
}


void
audio_io::_M_stop_input()
{
    int result;
    std::string message;

    result = HI_MPI_AI_DisableChn(m_device_id, m_channel_id);
    if (result != HI_SUCCESS) {
        message = "HI_MPI_AI_DisableChn: ";
        goto error;
    }

    result = HI_MPI_AI_Disable(m_device_id);
    if (result != HI_SUCCESS) {
        message = "HI_MPI_AI_Disable: ";
        goto error;
    }

    return;
    error:
    throw std::runtime_error(message + mpp_err_string(result));
}


void
audio_io::_M_start_output()
{
    int result;
    std::string message;

    result = HI_MPI_AO_SetPubAttr(m_device_id, &m_info);
    if (result != HI_SUCCESS) {
        message = "HI_MPI_AO_SetPubAttr: ";
        goto error;
    }

    result = HI_MPI_AO_Enable(m_device_id);
    if (result != HI_SUCCESS) {
        message = "HI_MPI_AO_Enable: ";
        goto error;
    }

    result = HI_MPI_AO_EnableChn(m_device_id, m_channel_id);
    if (result != HI_SUCCESS) {
        message = "HI_MPI_AO_EnableChn: ";
        goto error;
    }

    result = HI_MPI_AO_EnableChn(m_device_id, AO_SYSCHN_CHNID);
    if (result != HI_SUCCESS) {
        message = "HI_MPI_AO_EnableChn: ";
        goto error;
    }

    return;
    error:
    throw std::runtime_error(message + mpp_err_string(result));
}


void
audio_io::_M_stop_output()
{
    int result;
    std::string message;

    result = HI_MPI_AO_DisableChn(m_device_id, AO_SYSCHN_CHNID);
    if (result != HI_SUCCESS) {
        message = "HI_MPI_AO_DisableChn: ";
        goto error;
    }

    result = HI_MPI_AO_DisableChn(m_device_id, m_channel_id);
    if (result != HI_SUCCESS) {
        message = "HI_MPI_AO_DisableChn: ";
        goto error;
    }

    result = HI_MPI_AO_Disable(m_device_id);
    if (result != HI_SUCCESS) {
        message = "HI_MPI_AO_Disable: ";
        goto error;
    }

    return;
    error:
    throw std::runtime_error(message + mpp_err_string(result));
}
