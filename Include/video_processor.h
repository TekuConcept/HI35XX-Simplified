/**
 * Created by TekuConcept on May 4, 2021
 */

#ifndef HISILICON_VIDEO_PROCESSOR_H
#define HISILICON_VIDEO_PROCESSOR_H

#include "video_input.h"

namespace hisilicon {

    class video_processor {
    public:

        void bind(video_input* source);
        void bind(video_processor* source);
    };

} /* namespace hisilicon */

#endif /* HISILICON_VIDEO_PROCESSOR_H */
