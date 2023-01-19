#include <menu/TitleTaskState.h>
#include <cstring>
#include <utils/InputUtils.h>
#include <utils/LanguageUtils.h>
#include <savemng.h>

#include <coreinit/time.h>

static int cursorPos = 0;
static int entrycount;

void TitleOptionsState::render() {
    this->isWiiUTitle = this->title.highID != (0x00050000 | 0x00050002);
    entrycount = 3;
    consolePrintPos(M_OFF, 2, "[%08X-%08X] %s", this->title.highID, this->title.lowID,
                    this->title.shortName);

    if (this->task == copytoOtherDevice) {
        consolePrintPos(M_OFF, 4, LanguageUtils::gettext("Destination:"));
        consolePrintPos(M_OFF, 5, "    (%s)", this->title.isTitleOnUSB ? "NAND" : "USB");
    } else if (this->task > 2) {
        entrycount = 2;
        consolePrintPos(M_OFF, 4, LanguageUtils::gettext("Select %s:"), LanguageUtils::gettext("version"));
        consolePrintPos(M_OFF, 5, "   < v%u >", versionList != nullptr ? versionList[slot] : 0);
    } else if (task == wipe) {
        consolePrintPos(M_OFF, 4, LanguageUtils::gettext("Delete from:"));
        consolePrintPos(M_OFF, 5, "    (%s)", this->title.isTitleOnUSB ? "USB" : "NAND");
    } else {
        consolePrintPos(M_OFF, 4, LanguageUtils::gettext("Select %s:"), LanguageUtils::gettext("slot"));

        if (((this->title.highID & 0xFFFFFFF0) == 0x00010000) && (slot == 255))
            consolePrintPos(M_OFF, 5, "   < SaveGame Manager GX > (%s)",
                            isSlotEmpty(this->title.highID, this->title.lowID, slot) ? LanguageUtils::gettext("Empty")
                                                                                        : LanguageUtils::gettext("Used"));
        else
            consolePrintPos(M_OFF, 5, "   < %03u > (%s)", slot,
                            isSlotEmpty(this->title.highID, this->title.lowID, slot) ? LanguageUtils::gettext("Empty")
                                                                                        : LanguageUtils::gettext("Used"));
    }

    if (this->isWiiUTitle) {
        if (task == restore) {
            if (!isSlotEmpty(this->title.highID, this->title.lowID, slot)) {
                entrycount++;
                consolePrintPos(M_OFF, 7, LanguageUtils::gettext("Select SD user to copy from:"));
                if (sdusers == -1)
                    consolePrintPos(M_OFF, 8, "   < %s >", LanguageUtils::gettext("all users"));
                else
                    consolePrintPos(M_OFF, 8, "   < %s > (%s)", sdacc[sdusers].persistentID,
                                    hasAccountSave(&this->title, true, false, sdacc[sdusers].pID,
                                                    slot, 0)
                                            ? LanguageUtils::gettext("Has Save")
                                            : LanguageUtils::gettext("Empty"));
            }
        }

        if (task == wipe) {
            consolePrintPos(M_OFF, 7, LanguageUtils::gettext("Select Wii U user to delete from:"));
            if (allusers == -1)
                consolePrintPos(M_OFF, 8, "   < %s >", LanguageUtils::gettext("all users"));
            else
                consolePrintPos(M_OFF, 8, "   < %s (%s) > (%s)", wiiuacc[allusers].miiName,
                                wiiuacc[allusers].persistentID,
                                hasAccountSave(&this->title, false, false, wiiuacc[allusers].pID,
                                                slot, 0)
                                        ? LanguageUtils::gettext("Has Save")
                                        : LanguageUtils::gettext("Empty"));
        }

        if ((task == backup) || (task == restore) || (task == copytoOtherDevice)) {
            if ((task == restore) && isSlotEmpty(this->title.highID, this->title.lowID, slot))
                entrycount--;
            else {
                consolePrintPos(M_OFF, (task == restore) ? 10 : 7, LanguageUtils::gettext("Select Wii U user%s:"),
                                (task == copytoOtherDevice) ? LanguageUtils::gettext(" to copy from") : ((task == restore) ? LanguageUtils::gettext(" to copy to") : ""));
                if (allusers == -1)
                    consolePrintPos(M_OFF, (task == restore) ? 11 : 8, "   < %s >", LanguageUtils::gettext("all users"));
                else
                    consolePrintPos(M_OFF, (task == restore) ? 11 : 8, "   < %s (%s) > (%s)",
                                    wiiuacc[allusers].miiName, wiiuacc[allusers].persistentID,
                                    hasAccountSave(&this->title,
                                                    (!((task == backup) || (task == restore) || (task == copytoOtherDevice))),
                                                    (!((task < 3) || (task == copytoOtherDevice))),
                                                    wiiuacc[allusers].pID, slot,
                                                    versionList != nullptr ? versionList[slot] : 0)
                                            ? LanguageUtils::gettext("Has Save")
                                            : LanguageUtils::gettext("Empty"));
            }
        }
        if ((task == backup) || (task == restore))
            if (!isSlotEmpty(this->title.highID, this->title.lowID, slot)) {
                Date *dateObj = new Date(this->titles.highID, this->title.lowID, slot);
                consolePrintPos(M_OFF, 15, LanguageUtils::gettext("Date: %s"),
                                dateObj->get().c_str());
                delete dateObj;
            }

        if (task == copytoOtherDevice) {
            entrycount++;
            consolePrintPos(M_OFF, 10, LanguageUtils::gettext("Select Wii U user%s:"), (task == copytoOtherDevice) ? LanguageUtils::gettext(" to copy to") : "");
            if (allusers_d == -1)
                consolePrintPos(M_OFF, 11, "   < %s >", LanguageUtils::gettext("all users"));
            else
                consolePrintPos(M_OFF, 11, "   < %s (%s) > (%s)", wiiuacc[allusers_d].miiName,
                                wiiuacc[allusers_d].persistentID,
                                hasAccountSave(&titles[this->title.dupeID], false, false,
                                                wiiuacc[allusers_d].pID, 0, 0)
                                        ? LanguageUtils::gettext("Has Save")
                                        : LanguageUtils::gettext("Empty"));
        }

        if ((task != importLoadiine) && (task != exportLoadiine)) {
            if (allusers > -1) {
                if (hasCommonSave(&this->title,
                                    (!((task == backup) || (task == wipe) || (task == copytoOtherDevice))),
                                    (!((task < 3) || (task == copytoOtherDevice))), slot,
                                    versionList != nullptr ? versionList[slot] : 0)) {
                    consolePrintPos(M_OFF, (task == restore) || (task == copytoOtherDevice) ? 13 : 10,
                                    LanguageUtils::gettext("Include 'common' save?"));
                    consolePrintPos(M_OFF, (task == restore) || (task == copytoOtherDevice) ? 14 : 11, "   < %s >",
                                    common ? LanguageUtils::gettext("yes") : LanguageUtils::gettext("no "));
                } else {
                    common = false;
                    consolePrintPos(M_OFF, (task == restore) || (task == copytoOtherDevice) ? 13 : 10,
                                    LanguageUtils::gettext("No 'common' save found."));
                    entrycount--;
                }
            } else {
                common = false;
                entrycount--;
            }
        } else {
            if (hasCommonSave(&this->title, true, true, slot, versionList != nullptr ? versionList[slot] : 0)) {
                consolePrintPos(M_OFF, 7, LanguageUtils::gettext("Include 'common' save?"));
                consolePrintPos(M_OFF, 8, "   < %s >", common ? LanguageUtils::gettext("yes") : LanguageUtils::gettext("no "));
            } else {
                common = false;
                consolePrintPos(M_OFF, 7, LanguageUtils::gettext("No 'common' save found."));
                entrycount--;
            }
        }

        consolePrintPos(M_OFF, 5 + cursor * 3, "\u2192");
        if (this->title.iconBuf != nullptr)
            DrawUtils::drawTGA(660, 100, 1, this->title.iconBuf);
    } else {
        entrycount = 1;
        if (this->title.iconBuf != nullptr)
            DrawUtils::drawRGB5A3(650, 100, 1, this->title.iconBuf);
        if (!isSlotEmpty(this->title.highID, this->title.lowID, slot)) {
            Date *dateObj = new Date(this->title.highID, this->title.lowID, slot);
            consolePrintPos(M_OFF, 15, LanguageUtils::gettext("Date: %s"),
                            dateObj->get().c_str());
            delete dateObj;
        }
    }

    switch (task) {
        case backup:
            consolePrintPosAligned(17, 4, 2, LanguageUtils::gettext("\ue000: Backup  \ue001: Back"));
            break;
        case restore:
            consolePrintPosAligned(17, 4, 2, LanguageUtils::gettext("\ue000: Restore  \ue001: Back"));
            break;
        case wipe:
            consolePrintPosAligned(17, 4, 2, LanguageUtils::gettext("\ue000: Wipe  \ue001: Back"));
            break;
        case importLoadiine:
            consolePrintPosAligned(17, 4, 2, LanguageUtils::gettext("\ue000: Import  \ue001: Back"));
            break;
        case exportLoadiine:
            consolePrintPosAligned(17, 4, 2, LanguageUtils::gettext("\ue000: Export  \ue001: Back"));
            break;
        case copytoOtherDevice:
            consolePrintPosAligned(17, 4, 2, LanguageUtils::gettext("\ue000: Copy  \ue001: Back"));
            break;
    }
}

ApplicationState::eSubState TitleOptionsState::update(Input *input) {
    if(input->get(TRIGGER, PAD_BUTTON_B))
        return SUBSTATE_RETURN;
    
    return SUBSTATE_RUNNING;
}