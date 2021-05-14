/**
 * Created by TekuConcept on May 5, 2021
 */

#ifndef HISILICON_VIDEO_CHANNEL_H
#define HISILICON_VIDEO_CHANNEL_H

#include "hi_comm_vi.h"
#include "video_input/video_device.h"
#include "video_input/video_pipe.h"

namespace hisilicon {

    class video_channel {
    public:
        video_channel& operator=(const video_channel&) = delete;
        video_channel(const video_channel&) = delete;
        video_channel(video_pipe* pipeline);
        ~video_channel();

        inline VI_CHN id() const
        { return m_channel_id; }
        inline video_device* sensor() const
        { return m_device; }
        inline video_pipe* pipeline() const
        { return m_pipe; }
        inline const VI_CHN_ATTR_S& channel_info() const
        { return m_channel_info; }

    private:
        video_device* m_device;
        video_pipe*   m_pipe;
        VI_CHN        m_channel_id;
        VI_CHN_ATTR_S m_channel_info;

        void _M_set_channel_info();
        void _M_start_channel();
        void _M_stop_channel();
    };

} /* namespace hisilicon */

#endif /* HISILICON_VIDEO_CHANNEL_H */
