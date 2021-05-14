/**
 * Created by TekuConcept on May 5, 2021
 */

#ifndef HISILICON_VIDEO_PIPE_H
#define HISILICON_VIDEO_PIPE_H

#include "hi_comm_vi.h"
#include "video_input/video_device.h"

namespace hisilicon {

    class video_pipe {
    public:
        video_pipe& operator=(const video_pipe&) = delete;
        video_pipe(const video_pipe&) = delete;
        video_pipe(video_device* sensor);
        ~video_pipe();

        inline VI_PIPE id() const
        { return m_pipe_id; }
        inline video_device* sensor() const
        { return m_video_device; }
        inline const VI_PIPE_ATTR_S& pipe_info() const
        { return m_pipe_info; }

    private:
        video_device*  m_video_device;
        VI_PIPE        m_pipe_id;
        VI_PIPE_ATTR_S m_pipe_info;

        void _M_set_pipe_info();
        void _M_bind_to_device();
        void _M_start_pipe();
        void _M_stop_pipe();
    };

} /* namespace hisilicon */

#endif /* HISILICON_VIDEO_PIPE_H */
