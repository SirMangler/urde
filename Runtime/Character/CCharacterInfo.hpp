#ifndef __URDE_CCHARACTERINFO_HPP__
#define __URDE_CCHARACTERINFO_HPP__

#include "IOStreams.hpp"
#include "CPASDatabase.hpp"
#include "zeus/CAABox.hpp"
#include "CEffectComponent.hpp"

namespace urde
{

class CCharacterInfo
{
public:
    struct CParticleResData
    {
        std::vector<ResId> x0_part;
        std::vector<ResId> x10_swhc;
        std::vector<ResId> x20_;
        std::vector<ResId> x30_elsc;
        CParticleResData(CInputStream& in, u16 tableCount);
    };
private:
    u16 x0_tableCount;
    std::string x4_name;
    ResId x14_cmdl;
    ResId x18_cskr;
    ResId x1c_cinf;
    std::vector<std::pair<u32, std::pair<std::string, std::string>>> x20_animInfo;
    CPASDatabase x30_pasDatabase;
    CParticleResData x44_partRes;
    u32 x84_unk;
    std::vector<std::pair<std::string, zeus::CAABox>> x88_aabbs;
    std::vector<std::pair<std::string, std::vector<CEffectComponent>>> x98_effects;

    ResId xa8_cmdlOverlay = 0;
    ResId xac_cskrOverlay = 0;

    std::vector<u32> xb0_animIdxs;

public:
    CCharacterInfo(CInputStream& in);

    ResId GetCharLayoutInfoId() const {return x1c_cinf;}
};

}

#endif // __URDE_CCHARACTERINFO_HPP__
