/**
 * Created by TekuConcept on May 4, 2021
 */

#ifndef HISILICON_VIDEO_ENCODER_H
#define HISILICON_VIDEO_ENCODER_H

#include "hi_comm_venc.h"

namespace hisilicon {

    class video_encoder {
    public:
        video_encoder& operator=(const video_encoder&) = delete;
        video_encoder(const video_encoder&) = delete;
        video_encoder(HI_U32 width, HI_U32 height, HI_U32 gop = 30);
        ~video_encoder();

        inline VENC_CHN id() const
        { return m_channel_id; }
        inline const VENC_CHN_ATTR_S& channel_info() const
        { return m_info; }

    private:
        VENC_CHN m_channel_id;
        VENC_CHN_ATTR_S m_info;
        SIZE_S m_pic_size;
        HI_U32 m_gop;

        void _M_set_encoder_info();
        void _M_set_bitrate_control_info();
        void _M_set_group_of_pics_info();
        void _M_start_encoder();
        void _M_stop_encoder();
    };

} /* namespace hisilicon */

#endif /* HISILICON_VIDEO_ENCODER_H */
