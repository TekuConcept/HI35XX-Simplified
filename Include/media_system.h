/**
 * Created by TekuConcept on May 4, 2021
 */

#ifndef HISILICON_VIDEO_BUFFER_H
#define HISILICON_VIDEO_BUFFER_H

#include <vector>
#include "mpi_vb.h"
#include "mpp_types.h"

namespace hisilicon {

    class media_system {
    public:
        typedef struct pool_t {
            unsigned int width;
            unsigned int height;
            unsigned int block_count;
            pixel_format_t pixel_format;
            buffer_iface_t buffer_iface;
            payload_type_t payload_type;
            bit_width_t data_bit_width;

            pool_t();
        } pool_t;

        media_system& operator=(const media_system&) = delete;
        media_system(const media_system&) = delete;
        media_system(const std::vector<pool_t>& pools);
        ~media_system();

    private:
        VB_CONFIG_S m_vb_config;

        void _M_set_video_buffer_data(const std::vector<pool_t>&);
        void _M_start_video_buffer();
        void _M_stop_video_buffer();
        void _M_start_system();
        void _M_stop_system();
    };

} /* namespace hisilicon */

#endif /* HISILICON_VIDEO_BUFFER_H */
