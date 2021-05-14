/**
 * Created by TekuConcept on May 4, 2021
 */

#ifndef HISILICON_VIDEO_DEVICE_H
#define HISILICON_VIDEO_DEVICE_H

#include "hi_mipi.h"
#include "hi_comm_vi.h"

#include "media_system.h"

namespace hisilicon {

    class video_device {
    public:
        video_device& operator=(const video_device&) = delete;
        video_device(const video_device&) = delete;
        video_device(media_system* buffer);
        ~video_device();

        inline VI_DEV id() const
        { return m_device_id; }
        inline const combo_dev_attr_t& mipi_info() const
        { return m_mipi_info; }
        inline const VI_DEV_ATTR_S& device_info() const
        { return m_device_info; }

    private:
        media_system*      m_media_system; // representive relation only
        VI_DEV             m_device_id;
        lane_divide_mode_t m_lane_divide_mode;
        combo_dev_attr_t   m_mipi_info;
        VI_DEV_ATTR_S      m_device_info;

        void _M_set_mipi_info();
        void _M_set_device_info();
        void _M_configure_as_offline();
        void _M_start_mipi_sensor();
        void _M_stop_mipi_sensor();
        void _M_start_device();
        void _M_stop_device();
    };

} /* namespace hisilicon */

#endif /* HISILICON_VIDEO_DEVICE_H */
