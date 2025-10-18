///////////////////////////////////////////////////////////////////////////////
// Name:        startpage.cpp
// Author:      Blake Madden
// Copyright:   (c) 2005-2023 Blake Madden
// License:     3-Clause BSD license
// SPDX-License-Identifier: BSD-3-Clause
///////////////////////////////////////////////////////////////////////////////

#include "startpage.h"
#include <wx/dcbuffer.h>
#include <wx/stdpaths.h>
#include <algorithm>
#include <utility>

wxDEFINE_EVENT(wxEVT_STARTPAGE_CLICKED, wxCommandEvent);

//-------------------------------------------
wxStartPage::wxStartPage(wxWindow* parent, wxWindowID id /*= wxID_ANY*/,
                         const wxArrayString& mruFiles /*= wxArrayString{}*/,
                         const wxBitmapBundle& logo /*= wxBitmapBundle{}*/,
                         wxString productDescription /*= wxString{}*/)
        : wxWindow(parent, id, wxDefaultPosition, wxDefaultSize,
                   wxFULL_REPAINT_ON_RESIZE, L"wxStartPage"),
          m_logoFont(wxFontInfo(
              wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT).
              GetFractionalPointSize() * 1.5)),
          m_logo(logo),
          m_productDescription(std::move(productDescription))
    {
    // Size of an icon scaled to 32x32, with label padding above and below it.
    // Note that Realise will adjust this later more intelligently.
    m_buttonHeight = GetButtonSize().GetHeight() + (2 * GetLabelPaddingHeight());
    m_buttonWidth = FromDIP(wxSize(200, 200)).GetWidth();

    wxWindow::SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    SetMRUList(mruFiles);

    if (wxSystemSettings::GetAppearance().IsDark())
        {
        SetButtonAreaBackgroundColor(wxColour(46, 46, 46));
        SetMRUBackgroundColor(wxColour(31, 31, 31));
        }

    Bind(wxEVT_PAINT, &wxStartPage::OnPaintWindow, this);
    Bind(wxEVT_MOTION, &wxStartPage::OnMouseChange, this);
    Bind(wxEVT_LEFT_DOWN, &wxStartPage::OnMouseClick, this);
    Bind(wxEVT_LEAVE_WINDOW, &wxStartPage::OnMouseLeave, this);
    Bind(wxEVT_SIZE, &wxStartPage::OnResize, this);
    }

//---------------------------------------------------
void wxStartPage::DrawHighlight(wxDC& dc, const wxRect rect,
                                const wxColour& color) const
    {
    const wxDCPenChanger pc{ dc, wxColour{ 211, 211, 211 } };
    if (m_style == wxStartPageStyle::wxStartPage3D)
        {
        // fill with the color
        dc.GradientFillLinear(rect, color, color.ChangeLightness(140), wxSOUTH);
        // create a shiny overlay
        dc.GradientFillLinear(wxRect(rect.GetX(), rect.GetY(),
                              rect.GetWidth(), rect.GetHeight() * 0.25),
                              wxColour(255, 255, 255, 25),
                              wxColour(255, 255, 255, 125), wxSOUTH);
        dc.DrawLine(rect.GetLeftTop(), rect.GetRightTop());
        dc.DrawLine(rect.GetRightTop(), rect.GetRightBottom());
        dc.DrawLine(rect.GetRightBottom(), rect.GetLeftBottom());
        dc.DrawLine(rect.GetLeftBottom(), rect.GetLeftTop());
        }
    else
        {
        const wxDCBrushChanger bc(dc, color);
        dc.DrawRectangle(rect);
        }
    }

//---------------------------------------------------
void wxStartPage::SetMRUList(const wxArrayString& mruFiles)
    {
    m_fileButtons.clear();
    m_fileButtons.resize(mruFiles.GetCount() + 1);
    if (mruFiles.GetCount() == 0)
        { return; }

    const auto simplifyFilePath = [](auto path)
        {
        path = wxFileName(path).GetPath();
        // shorten standard user paths
        path.Replace(
            wxStandardPaths::Get().GetUserDir(wxStandardPathsBase::Dir::Dir_Documents),
            _(L"Documents"));
        path.Replace(
            wxStandardPaths::Get().GetUserDir(wxStandardPathsBase::Dir::Dir_Desktop),
            _(L"Desktop"));
        path.Replace(
            wxStandardPaths::Get().GetUserDir(wxStandardPathsBase::Dir::Dir_Pictures),
            _(L"Pictures"));
        path.Replace(
            wxStandardPaths::Get().GetUserDir(wxStandardPathsBase::Dir::Dir_Videos),
            _(L"Videos"));
        path.Replace(
            wxStandardPaths::Get().GetUserDir(wxStandardPathsBase::Dir::Dir_Music),
            _(L"Music"));
        path.Replace(
            wxStandardPaths::Get().GetUserDir(wxStandardPathsBase::Dir::Dir_Downloads),
            _(L"Downloads"));
        // replace slashes with guillemets (makes it look fancier)
        path.Replace(wxFileName::GetPathSeparator(), L" \u00BB ", true);
        return path;
        };

    // load files that can be found
    size_t buttonCount{ 0 };
    std::vector<wxString> files;
    for (const auto& file: mruFiles)
        {
        if (wxFileName::FileExists(file))
            { files.push_back(file); }
        // no more than 9 items here, not enough real estate
        if (buttonCount == MAX_FILE_BUTTONS)
            { break; }
        }
    // no files, so no need for file buttons or the clear all button
    if (files.empty())
        {
        return;
        }

    std::sort(files.begin(), files.end(),
        [](const auto& lhv, const auto& rhv)
        {
        const wxFileName fn1(lhv), fn2(rhv);
        wxDateTime accessTime1, modTime1, createTime1,
                   accessTime2, modTime2, createTime2;
        fn1.GetTimes(&accessTime1, &modTime1, &createTime1);
        fn2.GetTimes(&accessTime2, &modTime2, &createTime2);
        // going in reverse, most recently modified goes to the front
        return modTime1 > modTime2;
        });

    // connect the file paths to the buttons in the MRU list
    for (buttonCount = 0; buttonCount < files.size(); ++buttonCount)
        {
        m_fileButtons[buttonCount].m_id = ID_FILE_ID_START + buttonCount;
        m_fileButtons[buttonCount].m_fullFilePath = files[buttonCount];
        m_fileButtons[buttonCount].m_label = simplifyFilePath(files[buttonCount]);
        }

    m_fileButtons[buttonCount].m_id = START_PAGE_FILE_LIST_CLEAR;
    m_fileButtons[buttonCount++].m_label = GetClearFileListLabel();
    m_fileButtons.resize(buttonCount);
    }

//---------------------------------------------------
wxString wxStartPage::FormatGreeting() const
    {
    if (m_greetingStyle == wxStartPageGreetingStyle::wxNoGreeting)
        { return wxString{}; }
    if (m_greetingStyle == wxStartPageGreetingStyle::wxCustomGreeting)
        { return m_customGreeting; }
    if (m_greetingStyle == wxStartPageGreetingStyle::wxDynamicGreetingWithUserName &&
        !m_userName.empty())
        {
        const auto currentHour{ wxDateTime::Now().GetHour() };
        return currentHour < 12 ?
            wxString::Format(_(L"Good morning, %s"), m_userName) :
            currentHour < 17 ?
            wxString::Format(_(L"Good afternoon, %s"), m_userName) :
            wxString::Format(_(L"Good evening, %s"), m_userName);
        }

    const auto currentHour{ wxDateTime::Now().GetHour() };
    return currentHour < 12 ? _(L"Good morning") :
           currentHour < 17 ? _(L"Good afternoon") :
                              _(L"Good evening");
    }

//---------------------------------------------------
void wxStartPage::CalcMRUColumnHeaderHeight(wxDC& dc)
    {
    const wxDCFontChanger fc(dc, dc.GetFont().Larger().Larger().Bold());
        m_fileColumnHeaderHeight = dc.GetTextExtent(GetRecentLabel()).GetHeight() +
            (2 * GetLabelPaddingHeight());

    m_fileColumnHeaderHeight = dc.GetTextExtent(GetRecentLabel()).GetHeight() +
            (2 * GetLabelPaddingHeight());

    const auto greeting{ FormatGreeting() };
    if (!greeting.empty())
        {
        m_fileColumnHeaderHeight += dc.GetTextExtent(greeting).GetHeight() +
            (2 * GetLabelPaddingHeight());
        }
    }

//---------------------------------------------------
void wxStartPage::CalcButtonStart(wxDC& dc)
    {
    wxCoord appNameWidth{ 0 }, appNameHeight{ 0 },
            appDescWidth{ 0 }, appDescHeight{ 0 };
        {
        const wxDCFontChanger fc(dc, m_logoFont);
        dc.GetTextExtent(wxTheApp->GetAppName(), &appNameWidth, &appNameHeight);
        }

    if (m_appHeaderStyle == wxStartPageAppHeaderStyle::wxStartPageNoHeader)
        { m_buttonsStart = GetTopBorder(); }
    else
        {
        m_buttonsStart =
            (m_appHeaderStyle == wxStartPageAppHeaderStyle::wxStartPageAppNameAndLogo) &&
             m_logo.IsOk() ?
            GetTopBorder() + (2 * GetLabelPaddingHeight()) +
            std::max(appNameHeight, GetAppLogoSize().GetHeight()) :
            GetTopBorder() + (2 * GetLabelPaddingHeight()) + appNameHeight;
        if (!m_productDescription.empty())
            {
            dc.GetTextExtent(m_productDescription, &appDescWidth, &appDescHeight);
            m_buttonsStart += appDescHeight + (2 * GetLabelPaddingHeight());
            }
        }
    }

//---------------------------------------------------
void wxStartPage::OnResize(wxSizeEvent& WXUNUSED(event))
    {
    wxClientDC dc(this);

    wxCoord appNameWidth{ 0 }, appNameHeight{ 0 },
            appDescWidth{ 0 }, appDescHeight{ 0 };
        {
        const wxDCFontChanger fc(dc, m_logoFont);
        dc.GetTextExtent(wxTheApp->GetAppName(), &appNameWidth, &appNameHeight);
        }

    CalcButtonStart(dc);
    CalcMRUColumnHeaderHeight(dc);

    if (!m_productDescription.empty())
        {
        dc.GetTextExtent(m_productDescription, &appDescWidth, &appDescHeight);
        m_buttonsStart += appDescHeight + (2 * GetLabelPaddingHeight());
        }
    // calculate how wide the buttons/top label need to be fit their content
    const auto buttonIconSize = GetButtonSize();
    m_buttonHeight = buttonIconSize.GetHeight() + (2 * GetLabelPaddingHeight());
        {
        m_buttonWidth = std::max(m_buttonWidth,
            (m_logo.IsOk() ?
             (appNameWidth + GetAppLogoSize().GetWidth()+(2*GetLabelPaddingWidth())) :
              appNameWidth + (2*GetLabelPaddingWidth())));

        const wxDCFontChanger fc(dc, m_buttons.size() > MAX_BUTTONS_SMALL_SIZE ?
                               dc.GetFont() :
                               dc.GetFont().Larger());
        wxCoord textWidth{ 0 }, textHeight{ 0 };

        for (const auto& button : m_buttons)
            {
            dc.GetTextExtent(button.m_label, &textWidth, &textHeight);
            m_buttonWidth =
                std::max(m_buttonWidth,
                         textWidth +
                         (4 * GetLabelPaddingWidth()) + buttonIconSize.GetWidth());
            m_buttonHeight = std::max(m_buttonHeight,
                                      textHeight + (2 * GetLabelPaddingWidth()));
            if (button.m_icon.IsOk())
                {
                m_buttonHeight =
                    std::max(m_buttonHeight,
                             buttonIconSize.GetHeight() +
                                (2 * GetLabelPaddingHeight()));
                }
            }
        }
    // initialize the buttons
    for (size_t i = 0; i < m_buttons.size(); ++i)
        {
        m_buttons[i].m_id = ID_BUTTON_ID_START + i;
        m_buttons[i].m_rect = wxRect(GetLeftBorder(),
                                     m_buttonsStart + (i * m_buttonHeight),
                                     m_buttonWidth,
                                     m_buttonHeight);
        }

    // calculate MRU info
        {
        const wxDCFontChanger fc(dc, dc.GetFont().Larger());
        if (!m_fileButtons.empty())
            {
            const auto line2TextSz = dc.GetTextExtent(m_fileButtons[0].m_label);
            const wxDCFontChanger fc2(dc, wxFont(dc.GetFont()).MakeLarger());
            const auto line1TextSz = dc.GetTextExtent(m_fileButtons[0].m_label);

            // enough space for the text (label and path) height
            // (or icon, whichever is larger) and some padding around it
            m_mruButtonHeight =
                std::max(line1TextSz.GetHeight() + line2TextSz.GetHeight(),
                         buttonIconSize.GetHeight()) +
                    (2*GetLabelPaddingHeight()) +
                    // line space between file name and path
                    (GetLabelPaddingHeight()/2);
            }
        }
    }

//---------------------------------------------------
void wxStartPage::OnPaintWindow(wxPaintEvent& WXUNUSED(event))
    {
    wxAutoBufferedPaintDC adc(this);
    adc.Clear();
    wxGCDC dc(adc);

    CalcButtonStart(dc);
    CalcMRUColumnHeaderHeight(dc);

    const wxColour buttonAreaFontColor =
        BlackOrWhiteContrast(GetButtonAreaBackgroundColor());
    const wxColour mruFontColor =
        BlackOrWhiteContrast(GetMRUBackgroundColor());
    const wxColour buttonAreaHoverColor =
        ShadeOrTint(GetButtonAreaBackgroundColor());
    const wxColour mruHoverColor =
        ShadeOrTint(GetMRUBackgroundColor());
    const wxColour buttonAreaHoverFontColor =
        BlackOrWhiteContrast(buttonAreaHoverColor);
    const wxColour mruFontHoverColor =
        BlackOrWhiteContrast(mruHoverColor);
    const wxColour mruSeparatorLineColor =
        ShadeOrTint(GetMRUBackgroundColor());

    // calculate the positions of the buttons in the files area
    const wxRect filesArea = wxRect(m_buttonWidth + (GetLeftBorder() * 2),
        0,
        GetClientSize().GetWidth() - (m_buttonWidth + (GetLeftBorder() * 2)),
        GetClientSize().GetHeight());
    const wxRect buttonsArea =
        wxRect(wxSize(GetClientSize().GetWidth() - filesArea.GetWidth(),
                      GetClientSize().GetHeight()));

    const wxRect fileColumnHeader =
        wxRect(filesArea.GetLeft(), 0,
               filesArea.GetWidth(), m_fileColumnHeaderHeight);
    wxRect greetingRect{ fileColumnHeader }, recentRect{ fileColumnHeader };

    const auto greeting{ FormatGreeting() };
    if (!greeting.empty())
        {
        greetingRect.SetHeight(fileColumnHeader.GetHeight()/2);
        recentRect.SetTop(greetingRect.GetBottom());
        recentRect.SetHeight(fileColumnHeader.GetHeight()/2);
        }
    else
        { greetingRect.SetSize(wxSize(0, 0)); }

    if (GetMRUFileCount() > 0)
        {
        // update the rects for the file buttons
        for (size_t i = 0; i < GetMRUFileCount(); ++i)
            {
            m_fileButtons[i].m_rect =
                wxRect(filesArea.GetLeft() + FromDIP(1),
                       m_fileColumnHeaderHeight + (i * GetMRUButtonHeight()),
                       filesArea.GetWidth() - FromDIP(2),
                       GetMRUButtonHeight());
            }
        // the "clear file list" button
            {
            const wxDCFontChanger fc(dc, wxFont(dc.GetFont()).MakeLarger());
            const auto clearButtonSize = dc.GetTextExtent(GetClearFileListLabel());
            m_fileButtons[GetMRUFileAndClearButtonCount() - 1].m_rect =
                wxRect(filesArea.GetLeft() + FromDIP(1),
                    m_fileColumnHeaderHeight +
                        ((GetMRUFileAndClearButtonCount() - 1) * GetMRUButtonHeight()),
                    clearButtonSize.GetWidth() + (GetLabelPaddingHeight() * 2),
                    clearButtonSize.GetHeight() + (GetLabelPaddingHeight() * 2));
            }
        }

    // update the custom buttons' rects
    for (size_t i = 0; i < m_buttons.size(); ++i)
        {
        m_buttons[i].m_rect = wxRect(GetLeftBorder(),
                                     m_buttonsStart + (i * m_buttonHeight),
                                     m_buttonWidth,
                                     m_buttonHeight);
        }

    // fill the background
    dc.SetBackground(GetButtonAreaBackgroundColor());
    dc.Clear();

    // draw the program logo
    if (m_appHeaderStyle != wxStartPageAppHeaderStyle::wxStartPageNoHeader)
        {
        wxCoord appDescWidth{ 0 }, appDescHeight{ 0 };
        if (!m_productDescription.empty())
            {
            dc.GetTextExtent(m_productDescription, &appDescWidth, &appDescHeight);
            appDescHeight += (2 * GetLabelPaddingHeight());
            }
        const wxDCTextColourChanger cc(dc, buttonAreaFontColor);
        const wxDCPenChanger pc(dc, buttonAreaFontColor);
        wxCoord textWidth{ 0 }, textHeight{ 0 };
        wxBitmap appLogo = m_logo.GetBitmap(ScaleToContentSize(GetAppLogoSize()));
        appLogo.SetScaleFactor(GetContentScaleFactor());
        if (m_appHeaderStyle == wxStartPageAppHeaderStyle::wxStartPageAppNameAndLogo &&
            appLogo.IsOk())
            {
            dc.DrawBitmap(appLogo, GetLeftBorder(), GetTopBorder());
            // draw with larger font
                {
                const wxDCFontChanger fc(dc, m_logoFont);
                dc.GetTextExtent(wxTheApp->GetAppName(), &textWidth, &textHeight);
                dc.DrawText(wxTheApp->GetAppName(),
                            GetLeftBorder()+appLogo.GetScaledWidth()+GetLabelPaddingWidth(),
                            GetTopBorder()+((appLogo.GetScaledHeight()/2)-(textHeight/2)));
                }
            if (!m_productDescription.empty())
                {
                dc.DrawText(m_productDescription,
                            (GetLeftBorder()) + ((m_buttonWidth/2) - (appDescWidth/2)),
                            GetTopBorder() + std::max<wxCoord>(appLogo.GetScaledHeight(),textHeight) +
                            GetLabelPaddingHeight());
                }
            const auto lineY = GetTopBorder() +
                            std::max<wxCoord>(appLogo.GetScaledHeight(), textHeight) +
                            appDescHeight + GetLabelPaddingWidth();
            dc.DrawLine(wxPoint((2*GetLeftBorder()),lineY),
                        wxPoint(m_buttonWidth, lineY));
            }
        else
            {
            // draw with larger font
                {
                const wxDCFontChanger fc(dc, m_logoFont);
                dc.GetTextExtent(wxTheApp->GetAppName(), &textWidth, &textHeight);
                // centering looks better when there is no logo
                dc.DrawText(wxTheApp->GetAppName(),
                            (buttonsArea.GetWidth() - textWidth) * 0.5,
                            GetTopBorder() + GetLabelPaddingHeight());
                }
            if (!m_productDescription.empty())
                {
                dc.DrawText(m_productDescription,
                           (GetLeftBorder())+((m_buttonWidth/2) - (appDescWidth/2)),
                           GetTopBorder() + textHeight + GetLabelPaddingHeight());
                }
            dc.DrawLine(wxPoint((2*GetLeftBorder()),
                                GetTopBorder() + textHeight +
                                GetLabelPaddingHeight() + appDescHeight),
                        wxPoint(m_buttonWidth,
                                GetTopBorder() + textHeight +
                                GetLabelPaddingHeight() + appDescHeight));
            }
        }

    // draw the MRU files area
        {
        const wxDCPenChanger pc(dc, wxColour{ 0, 0, 0, 0 });
        const wxDCBrushChanger bc(dc, GetMRUBackgroundColor());
        dc.DrawRectangle(filesArea);
        // if areas have the same color, then draw a contrasting line between them
        if (GetMRUBackgroundColor() == GetButtonAreaBackgroundColor())
            {
            const wxDCPenChanger pc2(dc, ShadeOrTint(GetMRUBackgroundColor()));
            dc.DrawLine(filesArea.GetTopLeft(), filesArea.GetBottomLeft());
            }
        }
    // draw the greeting
        {
        const wxDCFontChanger fc(dc, dc.GetFont().Larger().Larger().Bold());
        const wxDCTextColourChanger tcc(dc, mruFontColor);
        const wxDCPenChanger pc(dc, mruSeparatorLineColor);
        dc.SetClippingRegion(greetingRect);
        dc.DrawLabel(greeting,
            wxRect(greetingRect).Deflate(GetLabelPaddingWidth()),
            wxALIGN_LEFT);
        dc.DestroyClippingRegion();
        dc.DrawLine(greetingRect.GetLeftBottom(),
                    greetingRect.GetRightBottom());
        }
    // draw MRU column header
        {
        const wxDCFontChanger fc(dc, dc.GetFont().Larger().Larger());
        const wxDCTextColourChanger tcc(dc, mruFontColor);
        const wxDCPenChanger pc(dc,
            wxPen(wxPenInfo(mruSeparatorLineColor,
                            FromDIP(2)).Cap(wxPenCap::wxCAP_BUTT)));
        dc.SetClippingRegion(recentRect);
        dc.DrawLabel(GetRecentLabel(),
            wxRect(recentRect).Deflate(GetLabelPaddingWidth()),
            wxALIGN_CENTRE);
        dc.DestroyClippingRegion();
        auto midPoint = recentRect.GetLeftBottom();
        midPoint.x += (recentRect.GetRightBottom().x -
                       recentRect.GetLeftBottom().x) / 2;
        const wxSize recentTextSz{ dc.GetTextExtent(GetRecentLabel()) };
        dc.DrawLine(midPoint -
                        wxSize((recentTextSz.GetWidth()/2), 0),
                    midPoint +
                        wxSize((recentTextSz.GetWidth()/2), 0));
        }

    [[maybe_unused]] const wxString currentToolTip = m_toolTip;
    // highlight the active MRU file or custom button
    if (m_activeButton != wxNOT_FOUND)
        {
        wxRect buttonBorderRect;
        ActiveButtonType activeButton{ ActiveButtonType::CustomButton };
        for (size_t i = 0; i < GetMRUFileAndClearButtonCount(); ++i)
            {
            if (m_fileButtons[i].IsOk() &&
                m_activeButton == m_fileButtons[i].m_id)
                {
                // show either full path, the label, or nothing
                // (if a button under the MRU list)
                m_toolTip = (i == (GetMRUFileAndClearButtonCount() -1 )) ?
                    wxString{} :
                    !m_fileButtons[i].m_fullFilePath.empty() ?
                    m_fileButtons[i].m_fullFilePath : m_fileButtons[i].m_label;
                buttonBorderRect = m_fileButtons[i].m_rect;
                activeButton = (i == (GetMRUFileAndClearButtonCount() - 1)) ?
                    ActiveButtonType::FileActionButton :
                    ActiveButtonType::FileButton;
                break;
                }
            }
        for (const auto& button : m_buttons)
            {
            if (m_activeButton == button.m_id)
                {
                buttonBorderRect = button.m_rect;
                m_toolTip.clear();
                activeButton = ActiveButtonType::CustomButton;
                break;
                }
            }
        if (!buttonBorderRect.IsEmpty())
            {
            if (activeButton == ActiveButtonType::FileActionButton)
                {
                // highlight just the border so that it looks like a UI button
                const wxDCBrushChanger bdc(dc, wxColour{ 0, 0, 0, 0 });
                const wxDCPenChanger pdc(dc,
                    wxPen(ShadeOrTint(GetMRUBackgroundColor(), 0.4), FromDIP(2)));
                dc.DrawRectangle(buttonBorderRect);
                }
            else
                {
                DrawHighlight(dc, buttonBorderRect,
                    (activeButton == ActiveButtonType::CustomButton) ?
                    buttonAreaHoverColor : mruHoverColor);
                }
            }
        }
    else
        { m_toolTip.clear(); }

// don't use tooltips with GTK, they only appear randomly and cause painting issues
#ifndef __WXGTK__
    if (currentToolTip != m_toolTip)
        { SetToolTip(m_toolTip); }
#endif

    const auto formatFileDateTime = [](const auto& dt)
        {
        wxString dateStr;
        const auto currentTime{ wxDateTime::Now() };
        const wxTimeSpan timeDiff = wxDateTime::Now().Subtract(dt);
        if (timeDiff.GetHours() < 1)
            {
            if (timeDiff.GetMinutes() < 10)
                { dateStr = _(L"Just now"); }
            else
                {
                dateStr = wxString::Format(_(L"%d minutes ago"),
                    timeDiff.GetMinutes());
                }
            }
        else if (timeDiff.GetMinutes() < 90)
            { dateStr = _(L"1 hour ago"); }
        else if (timeDiff.GetHours() <= 8)
            {
            dateStr = wxString::Format(_(L"%d hours ago"),
                timeDiff.GetHours() +
                // round the minutes
                ((timeDiff.GetMinutes() - (60 * timeDiff.GetHours())) < 30 ? 0 : 1));
            }
        // named day if modified sometime this week
        else if (currentTime.GetYear() == dt.GetYear() &&
            currentTime.GetMonth() == dt.GetMonth() &&
            currentTime.GetWeekOfMonth() == dt.GetWeekOfMonth())
            {
            // include time, in the local clock format
            wxString am, pm;
            wxDateTime::GetAmPmStrings(&am, &pm);
            wxString timeStr;
            if (!am.empty() && !pm.empty())
                { timeStr = dt.Format(L"%I:%M %p").MakeUpper(); }
            else
                { timeStr = dt.Format(L"%H:%M"); }

            if (currentTime.GetDay() == dt.GetDay())
                { dateStr = wxString::Format(_(L"Today at %s"), timeStr); }
            else if (currentTime.GetDay() - 1 == dt.GetDay())
                { dateStr = wxString::Format(_(L"Yesterday at %s"), timeStr); }
            else
                {
                dateStr = wxString::Format(// TRANSLATORS: DAY at TIME OF DAY
                                           _(L"%s at %s"),
                    wxDateTime::GetWeekDayName(dt.GetWeekDay(),
                                               wxDateTime::NameFlags::Name_Abbr),
                    timeStr);
                }
            }
        else
            {
            // only show year if modified last year
            const wxString dateFormatStr =
                (wxDateTime::Now().GetYear() == dt.GetYear()) ?
                    L"%B %d " :
                    L"%B %d, %Y";
            dateStr = dt.Format(dateFormatStr);
            }

        return dateStr;
        };

    // file labels
        {
        // Get the widest file modified time label so that we can draw 
        // them ragged right. Also, get the longest file path to make
        // sure the time and path don't overlap.
        decltype(wxSize::x) filePathLabelWidth{ 0 };
        decltype(wxSize::x) timeLabelWidth{ 0 };
            {
            const wxDCFontChanger fc(dc, wxFont(dc.GetFont()));
            for (size_t i = 0; i < GetMRUFileCount(); ++i)
                {
                if (m_fileButtons[i].IsOk())
                    {
                    const wxFileName fn(m_fileButtons[i].m_fullFilePath);
                    wxDateTime accessTime, modTime, createTime;
                    if (fn.FileExists() && fn.GetTimes(&accessTime, &modTime, &createTime))
                        {
                        const wxString modTimeStr =
                            formatFileDateTime(modTime);
                        const wxSize timeStringSize =
                            dc.GetTextExtent(modTimeStr);
                        timeLabelWidth = std::max(timeLabelWidth, timeStringSize.GetWidth());
                        }
                    const wxSize filePathStringSize =
                        dc.GetTextExtent(fn.GetPath());
                    filePathLabelWidth = std::max(filePathLabelWidth, filePathStringSize.GetWidth());
                    }
                }
            }

        // begin drawing them
        wxBitmap fileIcon = m_logo.GetBitmap(ScaleToContentSize(wxSize{ 32, 32 }));
        fileIcon.SetScaleFactor(GetContentScaleFactor());
        for (size_t i = 0; i < GetMRUFileAndClearButtonCount(); ++i)
            {
            if (m_fileButtons[i].IsOk())
                {
                const wxDCTextColourChanger tcc(dc,
                    m_activeButton == m_fileButtons[i].m_id ?
                    mruFontHoverColor : mruFontColor);
                const wxRect fileLabelRect =
                    wxRect{ m_fileButtons[i].m_rect }.Deflate(GetLabelPaddingHeight());
                dc.SetClippingRegion(m_fileButtons[i].m_rect);
                // if the "clear file list" button
                if (i == GetMRUFileAndClearButtonCount() - 1)
                    {
                    const wxDCFontChanger fc(dc, wxFont(dc.GetFont()).MakeLarger());
                    dc.DrawLabel(m_fileButtons[i].m_label, fileLabelRect,
                                 wxALIGN_LEFT|wxALIGN_CENTRE_VERTICAL);
                    }
                else
                    {
                    // show the files
                    const wxFileName fn(m_fileButtons[i].m_fullFilePath);
                    if (fileIcon.IsOk())
                        {
                        dc.DrawBitmap(fileIcon,
                            wxPoint(fileLabelRect.GetLeft(),
                                    fileLabelRect.GetTop() +
                                        ((fileLabelRect.GetHeight() - fileIcon.GetLogicalHeight()) / 2)
                                    ));
                        int nameHeight{ 0 };
                        // draw the filename
                            {
                            const wxDCFontChanger fc(dc, wxFont(dc.GetFont()).MakeLarger());
                            nameHeight =
                                dc.GetTextExtent(fn.GetFullName()).GetHeight();
                            dc.DrawText(fn.GetFullName(),
                                wxPoint(fileLabelRect.GetLeft() +
                                        GetLabelPaddingWidth() + fileIcon.GetLogicalWidth(),
                                        fileLabelRect.GetTop()));
                            }
                        // draw the filepath
                            {
                            const wxDCTextColourChanger cc(dc, mruFontColor);
                            dc.DrawText(
                                // truncate the path is necessary
                                (m_fileButtons[i].m_label.length() <= 75) ?
                                m_fileButtons[i].m_label :
                                // TRANSLATORS: "..." implies a truncated file path.
                                (m_fileButtons[i].m_label.substr(0, 75) + _(L"...")),
                                wxPoint(fileLabelRect.GetLeft() +
                                        GetLabelPaddingWidth() +
                                        fileIcon.GetLogicalWidth(),
                                        fileLabelRect.GetTop() + nameHeight +
                                        (GetLabelPaddingHeight()/2)));
                            }
                        // draw the modified time off to the side
                        if ((fileIcon.GetLogicalWidth() +
                             GetLabelPaddingWidth() +
                             filePathLabelWidth + timeLabelWidth) <
                            fileLabelRect.GetWidth())
                            {
                            wxDateTime accessTime, modTime, createTime;
                            if (fn.FileExists() && fn.GetTimes(&accessTime, &modTime, &createTime))
                                {
                                const wxString modTimeStr =
                                    formatFileDateTime(modTime);
                                const wxSize timeStringSize =
                                    dc.GetTextExtent(modTimeStr);
                                dc.DrawText(modTimeStr,
                                    fileLabelRect.GetRight() -
                                        (timeLabelWidth+GetLabelPaddingHeight()),
                                    fileLabelRect.GetTop() +
                                        ((fileLabelRect.GetHeight()/2) -
                                          (timeStringSize.GetHeight()/2)));
                                }
                            }
                        }
                    // if not using an icon, then just keep it
                    // simple and draw the filename
                    else
                        {
                        dc.DrawLabel(fn.GetFullName(), fileLabelRect,
                                     wxALIGN_LEFT|wxALIGN_CENTRE_VERTICAL);
                        }
                    // draw separator line, unless this button is highlighted
                    if (m_activeButton != m_fileButtons[i].m_id)
                        {
                        const wxDCPenChanger pc(dc, mruSeparatorLineColor);
                        dc.DrawLine(m_fileButtons[i].m_rect.GetLeftBottom(),
                            m_fileButtons[i].m_rect.GetRightBottom());
                        }
                    }
                dc.DestroyClippingRegion();
                }
            }
        }

    // draw the custom button labels
        {
        const auto buttonIconSize = GetButtonSize();
        m_buttonHeight = buttonIconSize.GetHeight() + (2 * GetLabelPaddingHeight());
        const wxDCFontChanger fc(dc, m_buttons.size() > MAX_BUTTONS_SMALL_SIZE ?
                               dc.GetFont() :
                               dc.GetFont().Larger());
        for (const auto& button : m_buttons)
            {
            if (button.IsOk())
                {
                const wxDCTextColourChanger cc(dc,
                    m_activeButton == button.m_id ?
                    buttonAreaHoverFontColor : buttonAreaFontColor);

                // draw it
                dc.SetClippingRegion(button.m_rect);
                wxBitmap bmp = button.m_icon.GetBitmap(ScaleToContentSize(buttonIconSize));
                bmp.SetScaleFactor(GetContentScaleFactor());
                dc.DrawLabel(button.m_label, bmp,
                    wxRect(button.m_rect).Deflate(GetLabelPaddingWidth()));
                dc.DestroyClippingRegion();
                }
            }
        }
    }

//---------------------------------------------------
void wxStartPage::OnMouseChange(const wxMouseEvent& event)
    {
    // see which (if any) button was previously highlighted
    wxRect previousRect, currentRect;
    if (m_activeButton != wxNOT_FOUND)
        {
        for (const auto& button : m_buttons)
            {
            if (m_activeButton == button.m_id)
                {
                previousRect = button.m_rect;
                break;
                }
            }
        for (size_t i = 0; i < GetMRUFileAndClearButtonCount(); ++i)
            {
            if (m_activeButton == m_fileButtons[i].m_id)
                {
                previousRect = m_fileButtons[i].m_rect;
                break;
                }
            }
        }

    const auto previouslyActiveButton{ m_activeButton };
    m_activeButton = wxNOT_FOUND;
    for (const auto& button : m_buttons)
        {
        if (button.IsOk() &&
            button.m_rect.Contains(event.GetX(), event.GetY()) )
            {
            m_activeButton = button.m_id;
            // if it's the same active button from before, then don't bother refreshing
            if (previouslyActiveButton == m_activeButton)
                { return; }
            // ...otherwise, just refresh the current and previous (if applicable) highlighted areas
            currentRect = button.m_rect;
            wxRect refreshRect = previousRect.IsEmpty() ?
                currentRect :
                previousRect.Union(currentRect);
            refreshRect.Inflate(GetLabelPaddingHeight());
            Refresh(true, &refreshRect);
            Update();
            return;
            }
        }
    for (size_t i = 0; i < GetMRUFileAndClearButtonCount(); ++i)
        {
        if (m_fileButtons[i].IsOk() &&
            m_fileButtons[i].m_rect.Contains(event.GetX(), event.GetY()) )
            {
            m_activeButton = m_fileButtons[i].m_id;
            if (previouslyActiveButton == m_activeButton)
                { return; }
            currentRect = m_fileButtons[i].m_rect;
            wxRect refreshRect = previousRect.IsEmpty() ?
                currentRect :
                previousRect.Union(currentRect);
            refreshRect.Inflate(GetLabelPaddingHeight());
            Refresh(true, &refreshRect);
            Update();
            return;
            }
        }

    if (previousRect.IsEmpty())
        { previousRect = GetClientRect(); }
    else
        { previousRect.Inflate(GetLabelPaddingHeight()); }
    Refresh(true, &previousRect);
    Update();
    }

//---------------------------------------------------
void wxStartPage::OnMouseLeave(wxMouseEvent& WXUNUSED(event))
    {
    // see which (if any) button was previously highlighted
    wxRect refreshRect;
    if (m_activeButton != wxNOT_FOUND)
        {
        for (const auto& button : m_buttons)
            {
            if (m_activeButton == button.m_id)
                {
                refreshRect = button.m_rect;
                break;
                }
            }
        for (size_t i = 0; i < GetMRUFileAndClearButtonCount(); ++i)
            {
            if (m_activeButton == m_fileButtons[i].m_id)
                {
                refreshRect = m_fileButtons[i].m_rect;
                break;
                }
            }
        }
    else
        { return; }

    m_activeButton = wxNOT_FOUND;

    refreshRect.Inflate(GetLabelPaddingHeight());
    Refresh(true, &refreshRect);
    Update();
    }

//---------------------------------------------------
void wxStartPage::OnMouseClick(wxMouseEvent& event)
    {
    for (const auto& button : m_buttons)
        {
        if (button.IsOk() &&
            button.m_rect.Contains(event.GetX(), event.GetY()) )
            {
            wxCommandEvent cevent(wxEVT_STARTPAGE_CLICKED, GetId());
            cevent.SetId(button.m_id);
            cevent.SetInt(button.m_id);
            cevent.SetEventObject(this);
            GetEventHandler()->ProcessEvent(cevent);
            break;
            }
        }
    for (size_t i = 0; i < GetMRUFileAndClearButtonCount(); ++i)
        {
        if (m_fileButtons[i].IsOk() &&
            m_fileButtons[i].m_rect.Contains(event.GetX(), event.GetY()) )
            {
            // if clicking the clear all button
            if (i == GetMRUFileAndClearButtonCount() - 1)
                {
                if (wxMessageBox(
                        _(L"Do you wish to clear the list of recent files?"),
                        _(L"Clear File List"), wxYES_NO | wxICON_QUESTION) == wxYES)
                    {
                    SetMRUList(wxArrayString{});
                    Refresh();
                    Update();
                    // give the caller a change to clear the file history
                    // from their doc manager and whatnot
                    wxCommandEvent cevent(wxEVT_STARTPAGE_CLICKED, GetId());
                    cevent.SetId(START_PAGE_FILE_LIST_CLEAR);
                    cevent.SetInt(START_PAGE_FILE_LIST_CLEAR);
                    cevent.SetEventObject(this);
                    GetEventHandler()->ProcessEvent(cevent);
                    break;
                    }
                }
            else
                {
                wxCommandEvent cevent(wxEVT_STARTPAGE_CLICKED, GetId());
                // client calls IsFileId() on this ID to see
                // if a file button was clicked
                cevent.SetId(m_fileButtons[i].m_id);
                cevent.SetInt(m_fileButtons[i].m_id);
                // selected file path
                cevent.SetString(m_fileButtons[i].m_fullFilePath);
                cevent.SetEventObject(this);
                GetEventHandler()->ProcessEvent(cevent);
                break;
                }
            }
        }
    }
