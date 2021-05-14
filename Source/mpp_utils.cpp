/**
 * Created by TekuConcept on May 5, 2021
 */

#include <sstream>
#include <iomanip>
#include "hi_errno.h"
#include "hi_common.h"
#include "mpp_utils.h"

namespace hisilicon {

    #define HI_DEF_ERR(module, level, errid) \
    ((HI_S32)((HI_ERR_APPID) | ((module) << 16) | ((level) << 13) | (errid)))

    // MOD_ID_E, ERR_LEVEL_E, EN_ERR_CODE_E

    static std::string
    mpp_id2str(unsigned int id) {
        switch (id) {
        case HI_ID_CMPI:         return "CMPI";
        case HI_ID_VB:           return "VB";
        case HI_ID_SYS:          return "SYS";
        case HI_ID_RGN:          return "RGN";
        case HI_ID_CHNL:         return "CHNL";
        case HI_ID_VDEC:         return "VDEC";
        case HI_ID_AVS:          return "AVS";
        case HI_ID_VPSS:         return "VPSS";
        case HI_ID_VENC:         return "VENC";
        case HI_ID_SVP:          return "SVP";
        case HI_ID_H264E:        return "H264E";
        case HI_ID_JPEGE:        return "JPEGE";
        case HI_ID_MPEG4E:       return "MPEG4E";
        case HI_ID_H265E:        return "H265E";
        case HI_ID_JPEGD:        return "JPEGD";
        case HI_ID_VO:           return "VO";
        case HI_ID_VI:           return "VI";
        case HI_ID_DIS:          return "DIS";
        case HI_ID_VALG:         return "VALG";
        case HI_ID_RC:           return "RC";
        case HI_ID_AIO:          return "AIO";
        case HI_ID_AI:           return "AI";
        case HI_ID_AO:           return "AO";
        case HI_ID_AENC:         return "AENC";
        case HI_ID_ADEC:         return "ADEC";
        case HI_ID_VPU:          return "VPU";
        case HI_ID_PCIV:         return "PCIV";
        case HI_ID_PCIVFMW:      return "PCIVFMW";
        case HI_ID_ISP:          return "ISP";
        case HI_ID_IVE:          return "IVE";
        case HI_ID_USER:         return "USER";
        case HI_ID_DCCM:         return "DCCM";
        case HI_ID_DCCS:         return "DCCS";
        case HI_ID_PROC:         return "PROC";
        case HI_ID_LOG:          return "LOG";
        case HI_ID_VFMW:         return "VFMW";
        case HI_ID_H264D:        return "H264D";
        case HI_ID_GDC:          return "GDC";
        case HI_ID_PHOTO:        return "PHOTO";
        case HI_ID_FB:           return "FB";
        case HI_ID_HDMI:         return "HDMI";
        case HI_ID_VOIE:         return "VOIE";
        case HI_ID_TDE:          return "TDE";
        case HI_ID_HDR:          return "HDR";
        case HI_ID_PRORES:       return "PRORES";
        case HI_ID_VGS:          return "VGS";

        case HI_ID_FD:           return "FD";
        case HI_ID_ODT:          return "ODT";
        case HI_ID_VQA:          return "VQA";
        case HI_ID_LPR:          return "LPR";
        case HI_ID_SVP_NNIE:     return "SVP_NNIE";
        case HI_ID_SVP_DSP:      return "SVP_DSP";
        case HI_ID_DPU_RECT:     return "RECT";
        case HI_ID_DPU_MATCH:    return "MATCH";

        case HI_ID_MOTIONSENSOR: return "MOTIONSENSOR";
        case HI_ID_MOTIONFUSION: return "MOTIONFUSION";

        case HI_ID_GYRODIS:      return "GYRODIS";
        case HI_ID_PM:           return "PM";
        case HI_ID_SVP_ALG:      return "SVP_ALG";

        default: return "UNK";
        }
    }

    static std::string
    mpp_err2str(unsigned int err) {
        switch (err) {
        case EN_ERR_INVALID_DEVID:        return "INVALID_DEVID";
        case EN_ERR_INVALID_CHNID:        return "INVALID_CHNID";
        case EN_ERR_ILLEGAL_PARAM:        return "ILLEGAL_PARAM";
        case EN_ERR_EXIST:                return "EXISTS";
        case EN_ERR_UNEXIST:              return "UNEXISTS";
        case EN_ERR_NULL_PTR:             return "NULL_PTR";
        case EN_ERR_NOT_CONFIG:           return "NOT_CONFIG";
        case EN_ERR_NOT_SUPPORT:          return "NOT_SUPPORT";
        case EN_ERR_NOT_PERM:             return "NOT_PERM";
        case EN_ERR_INVALID_PIPEID:       return "INVALID_PIPEID";
        case EN_ERR_INVALID_STITCHGRPID:  return "INVALID_STITCHGRPID";
        case EN_ERR_NOMEM:                return "NOMEM";
        case EN_ERR_NOBUF:                return "NOBUF";
        case EN_ERR_BUF_EMPTY:            return "BUF_EMPTY";
        case EN_ERR_BUF_FULL:             return "BUF_FULL";
        case EN_ERR_SYS_NOTREADY:         return "SYS_NOTREADY";
        case EN_ERR_BADADDR:              return "BADADDR";
        case EN_ERR_BUSY:                 return "BUSY";
        case EN_ERR_SIZE_NOT_ENOUGH:      return "SIZE_NOT_ENOUGH";
        default:                          return "UNK";
        }
    }

    std::string
    mpp_err_string(int code)
    {
        std::ostringstream os;
        os << "HI_ERR_";
        os << mpp_id2str(((unsigned int)code >> 16) & 0xFF) << "_";
        os << mpp_err2str((unsigned int)code & 0xFF);
        os << std::setfill('0') << std::hex;
        os << "(0x" << std::setw(sizeof(code) << 1) << code << ")";
        return os.str();
    }

} /* namespace hisilicon */
