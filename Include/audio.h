/**
 * Created by TekuConcept on May 10, 2021
 */

#ifndef HISILICON_AUDIO_IO_H
#define HISILICON_AUDIO_IO_H

#include "hi_comm_aio.h"
#include "media_system.h"

namespace hisilicon {

    class audio_io {
    public:
        audio_io& operator=(const audio_io&) = delete;
        audio_io(const audio_io&) = delete;
        audio_io(media_system* system);
        ~audio_io();

        inline AUDIO_DEV device_id() const
        { return m_device_id; }
        inline AO_CHN channel_id() const
        { return AO_SYSCHN_CHNID; }
        inline AO_CHN mono_channel_id() const
        { return m_channel_id; }
        inline const AIO_ATTR_S& info() const
        { return m_info; }

    private:
        media_system* m_media_system; // representive relation only
        AUDIO_DEV     m_device_id;
        AO_CHN        m_channel_id;
        AIO_ATTR_S    m_info;

        void _M_set_info();
        void _M_init_codec();
        void _M_start_input();
        void _M_stop_input();
        void _M_start_output();
        void _M_stop_output();
    };

} /* namespace hisilicon */

#endif /* HISILICON_AUDIO_IO_H */
