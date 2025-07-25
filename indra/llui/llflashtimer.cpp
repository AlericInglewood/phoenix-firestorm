/**
 * @file llflashtimer.cpp
 * @brief LLFlashTimer class implementation
 *
 * $LicenseInfo:firstyear=2002&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2012, Linden Research, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
 * $/LicenseInfo$
 */

#include "linden_common.h" // <MG:Aleric> Must be included first.

#include "llflashtimer.h"
#include "lleventtimer.h"
#include "llui.h"

LLFlashTimer::LLFlashTimer(callback_t cb, S32 count, F32 period)
:   LLEventTimer(period),
    mCallback(cb),
    mCurrentTickCount(0),
    mIsFlashingInProgress(false),
    mIsCurrentlyHighlighted(false),
    mUnset(false)
{
    mEventTimer.stop();

    // By default use settings from settings.xml to be able change them via Debug settings. See EXT-5973.
    // Due to Timer is implemented as derived class from EventTimer it is impossible to change period
    // in runtime. So, both settings are made as required restart.
    // <FS:CR>
    //mFlashCount = 2 * ((count > 0) ? count : LLUI::getInstance()->mSettingGroups["config"]->getS32("FlashCount"));
    static LLCachedControl<S32> flash_count(*LLUI::getInstance()->mSettingGroups["config"], "FlashCount");
    mFlashCount = 2 * ((count > 0) ? count : flash_count);
    // </FS:CR>
    if (mPeriod <= 0)
    {
        //mPeriod = LLUI::getInstance()->mSettingGroups["config"]->getF32("FlashPeriod");
        // <FS:CR>
        static LLCachedControl<F32> flash_period(*LLUI::getInstance()->mSettingGroups["config"], "FlashPeriod");
        mPeriod = flash_period;
        // </FS:CR>
    }

    // <FS:Ansariel> Configurable at runtime
    LLUI::getInstance()->mSettingGroups["config"]->getControl("FlashCount")->getSignal()->connect(boost::bind(&LLFlashTimer::onUpdateFlashSettings, this));
    LLUI::getInstance()->mSettingGroups["config"]->getControl("FlashPeriod")->getSignal()->connect(boost::bind(&LLFlashTimer::onUpdateFlashSettings, this));
    // </FS:Ansariel>
}

// <FS:Ansariel> Configurable at runtime
void LLFlashTimer::onUpdateFlashSettings()
{
    stopFlashing();
    mFlashCount = 2 * llmax(LLUI::getInstance()->mSettingGroups["config"]->getS32("FlashCount"), 0);
    mPeriod = llmax(LLUI::getInstance()->mSettingGroups["config"]->getF32("FlashPeriod"), 0.f);
}
// </FS:Ansariel>

void LLFlashTimer::unset()
{
    mUnset = true;
    mCallback = NULL;
}

bool LLFlashTimer::tick()
{
    mIsCurrentlyHighlighted = !mIsCurrentlyHighlighted;

    if (mCallback)
    {
        mCallback(mIsCurrentlyHighlighted);
    }

    if (++mCurrentTickCount >= mFlashCount)
    {
        stopFlashing();
    }

    return mUnset;
}

void LLFlashTimer::startFlashing()
{
    mIsFlashingInProgress = true;
    mIsCurrentlyHighlighted = true;
    mEventTimer.start();
}

void LLFlashTimer::stopFlashing()
{
    mEventTimer.stop();
    mIsFlashingInProgress = false;
    mIsCurrentlyHighlighted = false;
    mCurrentTickCount = 0;
}

bool LLFlashTimer::isFlashingInProgress()
{
    return mIsFlashingInProgress;
}

bool LLFlashTimer::isCurrentlyHighlighted()
{
    return mIsCurrentlyHighlighted;
}


