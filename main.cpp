/**
 * Created by TekuConcept on May 5, 2021
 */

#include <cstring>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <stdexcept>

#include <poll.h>
#include <sys/select.h>

#include "video_input/video_device.h"
#include "video_input/video_pipe.h"
#include "video_input/video_channel.h"
#include "video_input/video_isp.h"
#include "video_codec/video_encoder.h"
#include "audio.h"
#include "media_system.h"
#include "mpp_utils.h"

#include "hi_defines.h"
#include "mpi_vi.h"
#include "mpi_venc.h"
#include "mpi_audio.h"
#include "mpi_sys.h"

using namespace hisilicon;

#define V(x) std::cout << x << std::endl;

std::vector<media_system::pool_t>
get_pools() {
    std::vector<media_system::pool_t> pools(1);

    for (auto& pool : pools) {
        pool.width          = 1920;
        pool.height         = 1080;
        pool.block_count    = 18;
        pool.pixel_format   = pixel_format_t::YVU_SEMIPLANAR_420;
        pool.buffer_iface   = buffer_iface_t::YUV_FRAME;
        pool.data_bit_width = bit_width_t::B8;
    }

    return pools;
}

void set_log_level()
{
    LOG_LEVEL_CONF_S log_conf;
    int result;

    memset(&log_conf, 0, sizeof(LOG_LEVEL_CONF_S));

    log_conf.cModName[0] = 'a';
    log_conf.cModName[1] = 'l';
    log_conf.cModName[2] = 'l';
    log_conf.enModId     = HI_ID_BUTT;
    log_conf.s32Level    = HI_DBG_ERR;
    result = HI_MPI_LOG_SetLevelConf(&log_conf);
    if (result != HI_SUCCESS)
        throw std::runtime_error(
            std::string("HI_MPI_LOG_SetLevelConf: ") +
            mpp_err_string(result));
}

int raw_video()
{
    int result;
    auto pools = get_pools();

    set_log_level();

    try {
        media_system  media(pools);
        video_device  sensor(&media);
        video_pipe    pipeline(&sensor);
        video_channel channel(&pipeline);
        video_isp     isp(&sensor, &pipeline, &channel);

        VIDEO_FRAME_INFO_S frame_info;
        do {
            result = HI_MPI_VI_GetChnFrame(
                pipeline.id(), channel.id(), &frame_info, -1);
            if (result != HI_SUCCESS) {
                std::cout << "HI_MPI_VI_GetChnFrame: ";
                std::cout << hisilicon::mpp_err_string(result) << std::endl;
                break;
            }
            std::cout << frame_info.stVFrame.u32Width << "x" << frame_info.stVFrame.u32Height << std::endl;
            HI_MPI_VI_ReleaseChnFrame(pipeline.id(), channel.id(), &frame_info);
        } while (true);
    }
    catch (std::runtime_error& err) {
        std::cerr << "Error: " << err.what() << std::endl;
        std::string log(1024u, '\0');
        result = HI_MPI_LOG_Read(&log[0], log.size());
        if (result >= 0)
            std::cerr << "Log: " << log << std::endl;
        else std::cerr << "Failed to get log ("
            << mpp_err_string(result) << ")" << std::endl;
    }
}

int encoded_video()
{
    int result;
    std::string message;
    auto pools = get_pools();
    struct pollfd pfds[1];

    set_log_level();

    try {
        media_system  media(pools);
        video_device  sensor(&media);
        video_pipe    pipeline(&sensor);
        video_channel channel(&pipeline);
        video_isp     isp(&sensor, &pipeline, &channel);
        video_encoder encoder(
            channel.channel_info().stSize.u32Width,
            channel.channel_info().stSize.u32Height);

        VIDEO_FRAME_INFO_S frame_info;
        VENC_STREAM_S stream;
        VENC_CHN_STATUS_S codec_status;

        pfds[0].fd = HI_MPI_VENC_GetFd(encoder.id());
        pfds[0].events = POLLIN;

        do {
            { // -- raw frames --
                // get raw frame
                result = HI_MPI_VI_GetChnFrame(
                    pipeline.id(), channel.id(), &frame_info, 10);
                if (result != HI_SUCCESS) {
                    std::cout << "HI_MPI_VI_GetChnFrame: ";
                    std::cout << hisilicon::mpp_err_string(result) << std::endl;
                    break;
                }

                // do something with the raw frame

                result = HI_MPI_VENC_SendFrame(encoder.id(), &frame_info, /*block*/-1);
                if (result != HI_SUCCESS) {
                    HI_MPI_VI_ReleaseChnFrame(pipeline.id(), channel.id(), &frame_info);
                    message = "HI_MPI_VENC_SendFrame: ";
                    goto error;
                }

                HI_MPI_VI_ReleaseChnFrame(pipeline.id(), channel.id(), &frame_info);
            }

            { // -- encoded frames --
                // check if encoded frames are available
                result = poll(pfds, 1, 0);
                if (result == -1) {
                    std::cout << "poll error: " << strerror(errno) << std::endl;
                    break;
                }
                else if (!(pfds[0].revents & POLLIN)) continue;

                memset(&stream, 0, sizeof(stream));

                result = HI_MPI_VENC_QueryStatus(encoder.id(), &codec_status);
                if (result != HI_SUCCESS) {
                    message = "HI_MPI_VENC_QueryStatus: ";
                    goto error;
                }
                else if (codec_status.u32CurPacks == 0) continue;

                std::vector<VENC_PACK_S> packets(codec_status.u32CurPacks);
                stream.pstPack = &packets[0];
                stream.u32PackCount = packets.size();

                result = HI_MPI_VENC_GetStream(encoder.id(), &stream, 1/*millisecond*/);
                if (result != HI_SUCCESS) {
                    message = "HI_MPI_VENC_GetStream: ";
                    goto error;
                }

                // for (const auto& packet : packets) {
                //     auto ptr = packet.pu8Addr + packet.u32Offset;
                //     auto size = packet.u32Len - packet.u32Offset;
                //     write(ptr, size);
                // }

                result = HI_MPI_VENC_ReleaseStream(encoder.id(), &stream);
                if (result != HI_SUCCESS) {
                    message = "HI_MPI_VENC_ReleaseStream: ";
                    goto error;
                }
            }
        } while (true);

        return 0;
        error:
        throw std::runtime_error(message + hisilicon::mpp_err_string(result));
    }
    catch (std::runtime_error& err) {
        std::cerr << "Error: " << err.what() << std::endl;
        std::string log(1024u, '\0');
        result = HI_MPI_LOG_Read(&log[0], log.size());
        if (result >= 0)
            std::cerr << "Log: " << log << std::endl;
        else std::cerr << "Failed to get log ("
            << mpp_err_string(result) << ")" << std::endl;
        return -1;
    }
}

int play_audio()
{
    int result, m, size;
    std::string message;

    AUDIO_FADE_S audio_fade;
    HI_BOOL muted;
    HI_S32 volume_db;

    set_log_level();

    try {
        media_system media({ });
        audio_io aio(&media);

        result = HI_MPI_AO_SetVolume(aio.device_id(), 6/*dB*/);
        if (result != HI_SUCCESS) {
            message = "HI_MPI_AO_SetVolume: ";
            goto error;
        }

        AUDIO_FRAME_S frame;
        size = aio.info().u32PtNumPerFrm;
        memset(&frame, 0, sizeof(AUDIO_FRAME_S));
        frame.enBitwidth  = aio.info().enBitwidth;
        frame.enSoundmode = aio.info().enSoundmode;
        frame.u32Len      = size << 1;
        frame.u32Seq      = 0;

        result = HI_MPI_SYS_MmzAlloc(
            &frame.u64PhyAddr[0],
            (void **)&frame.u64VirAddr[0],
            NULL, NULL,
            frame.u32Len);
        if (result != HI_SUCCESS) {
            message = "HI_MPI_SYS_MmzAlloc: ";
            goto error;
        }

        do {
            HI_U8* data = frame.u64VirAddr[0];
            for (int i = 0; i < frame.u32Len; i++)
                data[i] = (HI_U8)rand();

            result = HI_MPI_AO_SendFrame(
                aio.device_id(), aio.channel_id(), &frame, -1);
            if (result != HI_SUCCESS) {
                std::cout << "HI_MPI_AO_SendFrame: ";
                std::cout << hisilicon::mpp_err_string(result) << std::endl;
                break;
            }

            frame.u32Seq++;
        } while (true);

        result = HI_MPI_SYS_MmzFree(frame.u64PhyAddr[0], frame.u64VirAddr[0]);
        if (result != HI_SUCCESS) {
            message = "HI_MPI_SYS_MmzAlloc: ";
            goto error;
        }

        return 0;
        error:
        throw std::runtime_error(message + hisilicon::mpp_err_string(result));
    }
    catch (std::runtime_error& err) {
        std::cerr << "Error: " << err.what() << std::endl;
        std::string log(1024u, '\0');
        result = HI_MPI_LOG_Read(&log[0], log.size());
        if (result >= 0)
            std::cerr << "Log: " << log << std::endl;
        else std::cerr << "Failed to get log ("
            << mpp_err_string(result) << ")" << std::endl;
        return -1;
    }
}

int record_audio()
{
    int result, m, size;
    std::string message;
    struct pollfd pfds[1];

    AUDIO_FRAME_S frame;
    AEC_FRAME_S   aec_frame;
    AUDIO_FADE_S audio_fade;
    HI_BOOL muted;
    HI_S32 volume_db;

    set_log_level();

    try {
        media_system media({ });
        audio_io aio(&media);

        pfds[0].fd = HI_MPI_AI_GetFd(aio.device_id(), aio.mono_channel_id());
        pfds[0].events = POLLIN;

        do {
            result = poll(pfds, 1, 1000);
            if (result == 0) continue;
            else if (result == -1) {
                std::cout << "poll error: " << strerror(errno) << std::endl;
                break;
            }

            if (!(pfds[0].revents & POLLIN)) continue;

            result = HI_MPI_AI_GetFrame(
                aio.device_id(),
                aio.mono_channel_id(),
                &frame, &aec_frame, HI_FALSE);
            if (result != HI_SUCCESS) {
                message = "HI_MPI_AI_GetFrame: ";
                goto error;
            }

            // write((const char*)frame.u64VirAddr[0], frame.u32Len);

            result = HI_MPI_AI_ReleaseFrame(
                aio.device_id(),
                aio.mono_channel_id(),
                &frame, &aec_frame);
            if (result != HI_SUCCESS) {
                message = "HI_MPI_AI_ReleaseFrame: ";
                goto error;
            }
        } while (true);

        return 0;
        error:
        throw std::runtime_error(message + hisilicon::mpp_err_string(result));
    }
    catch (std::runtime_error& err) {
        std::cerr << "Error: " << err.what() << std::endl;
        std::string log(1024u, '\0');
        result = HI_MPI_LOG_Read(&log[0], log.size());
        if (result >= 0)
            std::cerr << "Log: " << log << std::endl;
        else std::cerr << "Failed to get log ("
            << mpp_err_string(result) << ")" << std::endl;
        return -1;
    }
}

void print_usage(char* name)
{
    std::cout << "Usage: " << name << " [0-3]\n";
    std::cout << "\t0: raw video\n";
    std::cout << "\t1: encoded video\n";
    std::cout << "\t2: play audio\n";
    std::cout << "\t3: record audio\n";
}

int main(int argc, char** argv) {
    if (argc != 2) {
        print_usage(argv[0]);
        return 1;
    }

    int option;
    try { option = std::stoi(argv[1]); }
    catch (...) {
        print_usage();
        return 1;
    }

    switch (option) {
    case 0: return raw_video();
    case 1: return encoded_video();
    case 2: return play_audio();
    case 3: return record_audio();
    default: print_usage();
    }

    return 0;
}
