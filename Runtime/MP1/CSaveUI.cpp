#include "CSaveUI.hpp"
#include "CSimplePool.hpp"
#include "GameGlobalObjects.hpp"
#include "CMemoryCardSys.hpp"
#include "GuiSys/CGuiFrame.hpp"
#include "GuiSys/CGuiTableGroup.hpp"
#include "GuiSys/CGuiTextPane.hpp"

namespace urde
{
namespace MP1
{

using EState = CMemoryCardDriver::EState;
using EError = CMemoryCardDriver::EError;

void CSaveUI::ResetCardDriver()
{
    x92_ = false;
    x6c_cardDriver.reset();
    bool importState = (x0_instIdx == 0 && !x90_needsDriverReset);
    x6c_cardDriver = ConstructCardDriver(importState);
    x6c_cardDriver->StartCardProbe();
    x10_uiType = UIType::Zero;
    FinishedLoading();
}

CIOWin::EMessageReturn CSaveUI::Update(float dt)
{
    if (PumpLoad())
        return CIOWin::EMessageReturn::Normal;

    x50_loadedFrame->Update(dt);
    x6c_cardDriver->Update();

    if (x6c_cardDriver->x10_state == EState::DriverClosed)
    {
        if (x90_needsDriverReset)
        {
            ResetCardDriver();
            x90_needsDriverReset = false;
        }
        else
            x80_iowRet = CIOWin::EMessageReturn::Exit;
    }
    else if (x6c_cardDriver->x10_state == EState::CardCheckDone && x10_uiType != UIType::Fourteen)
    {
        if (x6c_cardDriver->x28_cardSerial && x8_serial)
        {
            if (x93_secondaryInst)
            {
                x10_uiType = UIType::Fourteen;
                x91_ = true;
            }
            else
            {
                x8_serial = x6c_cardDriver->x28_cardSerial;
                x6c_cardDriver->IndexFiles();
            }
        }
    }
    else if (x6c_cardDriver->x10_state == EState::Ready)
    {
        if (x90_needsDriverReset)
            x6c_cardDriver->StartFileCreateTransactional();
    }

    if (x80_iowRet != CIOWin::EMessageReturn::Normal)
        return x80_iowRet;

    UIType oldTp = x10_uiType;
    x10_uiType = SelectUIType();
    if (oldTp == x10_uiType || x91_)
        FinishedLoading();

    if (x6c_cardDriver->x10_state == EState::NoCard)
    {
        auto res = CMemoryCardSys::CardProbe(CMemoryCardSys::EMemoryCardPort::SlotA);
        if (res.x0_error == CMemoryCardSys::ECardResult::READY ||
            res.x0_error == CMemoryCardSys::ECardResult::WRONGDEVICE)
            ResetCardDriver();
    }
    else if (x6c_cardDriver->x10_state == EState::CardFormatted)
    {
        ResetCardDriver();
    }
    else if (x6c_cardDriver->x10_state == EState::FileBad &&
             x6c_cardDriver->x14_error == EError::FileMissing)
    {
        x6c_cardDriver->StartFileCreate();
    }

    return CIOWin::EMessageReturn::Normal;
}

bool CSaveUI::PumpLoad()
{
    if (x50_loadedFrame)
        return true;
    if (!x14_txtrSaveBanner.IsLoaded())
        return false;
    if (!x20_txtrSaveIcon0.IsLoaded())
        return false;
    if (!x2c_txtrSaveIcon1.IsLoaded())
        return false;
    if (!x38_strgMemoryCard.IsLoaded())
        return false;
    for (TLockedToken<CSaveWorld>& savw : x70_saveWorlds)
        if (!savw.IsLoaded())
            return false;
    if (!x44_frmeGenericMenu.IsLoaded())
        return false;

    x50_loadedFrame = x44_frmeGenericMenu.GetObj();
    x54_textpane_message = static_cast<CGuiTextPane*>(x50_loadedFrame->FindWidget("textpane_message"));
    x58_tablegroup_choices = static_cast<CGuiTableGroup*>(x50_loadedFrame->FindWidget("tablegroup_choices"));
    x5c_textpane_choice0 = static_cast<CGuiTextPane*>(x50_loadedFrame->FindWidget("textpane_choice0"));
    x60_textpane_choice1 = static_cast<CGuiTextPane*>(x50_loadedFrame->FindWidget("textpane_choice1"));
    x64_textpane_choice2 = static_cast<CGuiTextPane*>(x50_loadedFrame->FindWidget("textpane_choice2"));
    x68_textpane_choice3 = static_cast<CGuiTextPane*>(x50_loadedFrame->FindWidget("textpane_choice3"));

    x58_tablegroup_choices->SetMenuAdvanceCallback(
        std::bind(&CSaveUI::DoAdvance, this, std::placeholders::_1));
    x58_tablegroup_choices->SetMenuSelectionChangeCallback(
        std::bind(&CSaveUI::DoSelectionChange, this, std::placeholders::_1));

    if (x0_instIdx == 1)
        x6c_cardDriver->StartCardProbe();

    x10_uiType = SelectUIType();
    FinishedLoading();
    return true;
}

CSaveUI::UIType CSaveUI::SelectUIType() const
{
    if (x6c_cardDriver->x10_state == EState::NoCard)
        return UIType::Three;

    switch (x10_uiType)
    {
    case UIType::Thirteen:
    case UIType::Fourteen:
    case UIType::Fifteen:
        return x10_uiType;
    default: break;
    }

    if (CMemoryCardDriver::IsCardBusy(x6c_cardDriver->x10_state))
    {
        if (!CMemoryCardDriver::IsOperationDestructive(x6c_cardDriver->x10_state))
            return UIType::Two;
        return UIType::One;
    }

    if (x6c_cardDriver->x10_state == EState::Ready)
    {
        if (x6c_cardDriver->x14_error == CMemoryCardDriver::EError::CardStillFull)
            return UIType::Twelve;
        return UIType::Sixteen;
    }

    if (x6c_cardDriver->x14_error == CMemoryCardDriver::EError::CardBroken)
        return UIType::Four;

    if (x6c_cardDriver->x14_error == CMemoryCardDriver::EError::CardWrongCharacterSet)
        return UIType::Five;

    if (x6c_cardDriver->x14_error == CMemoryCardDriver::EError::CardWrongDevice)
        return UIType::Seven;

    if (x6c_cardDriver->x14_error == CMemoryCardDriver::EError::CardFull)
    {
        if (x6c_cardDriver->x10_state == EState::CardCheckFailed)
            return UIType::Eight;
        return UIType::Nine;
    }

    if (x6c_cardDriver->x14_error == CMemoryCardDriver::EError::CardNon8KSectors)
        return UIType::Ten;

    if (x6c_cardDriver->x14_error == CMemoryCardDriver::EError::FileCorrupted)
        return UIType::Eleven;

    if (x6c_cardDriver->x14_error == CMemoryCardDriver::EError::CardIOError)
        return UIType::Six;

    return UIType::Zero;
}

void CSaveUI::FinishedLoading()
{

}

void CSaveUI::Draw() const
{

}

void CSaveUI::DoAdvance(CGuiTableGroup* caller)
{

}

void CSaveUI::DoSelectionChange(CGuiTableGroup* caller)
{

}

void CSaveUI::ProcessUserInput(const CFinalInput& input)
{

}

void CSaveUI::StartGame(int idx)
{
}

void CSaveUI::EraseGame(int idx)
{

}

void* CSaveUI::GetGameData(int idx) const
{
    return nullptr;
}

CSaveUI::CSaveUI(u32 instIdx, u64 serial)
: x0_instIdx(instIdx), x8_serial(serial)
{
    x14_txtrSaveBanner = g_SimplePool->GetObj("TXTR_SaveBanner");
    x20_txtrSaveIcon0 = g_SimplePool->GetObj("TXTR_SaveIcon0");
    x2c_txtrSaveIcon1 = g_SimplePool->GetObj("TXTR_SaveIcon1");
    x38_strgMemoryCard = g_SimplePool->GetObj("STRG_MemoryCard");
    x44_frmeGenericMenu = g_SimplePool->GetObj("FRME_GenericMenu");

    x6c_cardDriver = ConstructCardDriver(x0_instIdx);

    if (instIdx == 1)
    {
        x84_navConfirmSfx = 1432;
        x88_navMoveSfx = 1436;
        x8c_navBackSfx = 1431;
    }
    x93_secondaryInst = instIdx;

    x70_saveWorlds.reserve(g_MemoryCardSys->GetMemoryWorlds().size());
    for (const std::pair<ResId, CSaveWorldMemory>& wld : g_MemoryCardSys->GetMemoryWorlds())
    {
        x70_saveWorlds.push_back(
            g_SimplePool->GetObj(SObjectTag{FOURCC('SAVW'), wld.second.GetSaveWorldAssetId()}));
    }
}

std::unique_ptr<CMemoryCardDriver> CSaveUI::ConstructCardDriver(bool importState)
{
    return std::make_unique<CMemoryCardDriver>(CMemoryCardSys::EMemoryCardPort::SlotA,
        g_ResFactory->GetResourceIdByName("TXTR_SaveBanner")->id,
        g_ResFactory->GetResourceIdByName("TXTR_SaveIcon0")->id,
        g_ResFactory->GetResourceIdByName("TXTR_SaveIcon1")->id, importState);
}

}
}
