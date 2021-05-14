/**
 * Created by TekuConcept on May 6, 2021
 */

#ifndef HISILICON_VIDEO_ISP_H
#define HISILICON_VIDEO_ISP_H

#include <thread>
#include "hi_comm_isp.h"
#include "hi_comm_3a.h"
#include "video_input/video_device.h"
#include "video_input/video_pipe.h"
#include "video_input/video_channel.h"

namespace hisilicon {

    class video_isp {
    public:
        video_isp& operator=(const video_isp&) = delete;
        video_isp(const video_isp&) = delete;
        video_isp(
            video_device*  device,
            video_pipe*    pipe,
            video_channel* channel);
        ~video_isp();

        inline ISP_DEV id() const
        { return m_isp_id; }

    private:
        video_device*  m_device;
        video_pipe*    m_pipeline;
        video_channel* m_channel;
        HI_S32         m_bus_id;
        ISP_DEV        m_isp_id;
        ISP_PUB_ATTR_S m_isp_info;
        ISP_SNS_TYPE_E m_bus_type;
        ALG_LIB_S      m_ae_lib; // ae    - auto exposure
        ALG_LIB_S      m_awb_lib;// awb   - auto white balance
                                 // af/ai - auto focus / auto iris
        ISP_SNS_COMMBUS_U m_bus_info;
        std::thread    m_isp_thread;

        void _M_set_isp_info();
        void _M_start_isp();
        void _M_stop_isp();
    };

} /* namespace hisilicon */

#endif /* HISILICON_VIDEO_ISP_H */
