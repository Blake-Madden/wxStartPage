///////////////////////////////////////////////////////////////////////////////
// Name:        startpage.cpp
// Author:      Blake Madden
// Copyright:   (c) 2005-2023 Blake Madden
// Licence:     3-Clause BSD licence
// SPDX-License-Identifier: BSD-3-Clause
///////////////////////////////////////////////////////////////////////////////

#include "startpage.h"

wxDEFINE_EVENT(wxEVT_STARTPAGE_CLICKED, wxCommandEvent);

//-------------------------------------------
wxStartPage::wxStartPage(wxWindow* parent, wxWindowID id /*= wxID_ANY*/,
                         const wxArrayString& mruFiles /*= wxArrayString{}*/,
                         const wxBitmapBundle& logo /*= wxBitmapBundle{}*/,
                         const wxString productDescription /*= wxString{}*/) 
        : wxWindow(parent, id, wxDefaultPosition, wxDefaultSize,
                   wxFULL_REPAINT_ON_RESIZE, L"wxStartPage"),
          m_logo(logo),
          m_logoFont(wxFontInfo(
              wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT).
              GetFractionalPointSize() * 1.75)),
          m_productDescription(productDescription)
    {
    // Size of an icon scaled to 32x32, with label padding above and below it.
    // Note that Realise will adjust this later more intelligently.
    m_buttonHeight = GetButtonSize().GetHeight() + (2 * GetLabelPaddingHeight());
    m_buttonWidth = FromDIP(wxSize(200, 200)).GetWidth();

    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    SetMRUList(mruFiles);

    Bind(wxEVT_PAINT, &wxStartPage::OnPaintWindow, this);
    Bind(wxEVT_MOTION, &wxStartPage::OnMouseChange, this);
    Bind(wxEVT_LEFT_DOWN, &wxStartPage::OnMouseClick, this);
    Bind(wxEVT_LEAVE_WINDOW, &wxStartPage::OnMouseLeave, this);
    Bind(wxEVT_SIZE, &wxStartPage::OnResize, this);
    }

//---------------------------------------------------
void wxStartPage::DrawHighlight(wxDC& dc, const wxRect rect,
                                const wxColour color) const
    {
    wxDCPenChanger pc(dc, *wxLIGHT_GREY_PEN);
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
        wxDCBrushChanger bc(dc, color);
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
    size_t buttonCount{ 0 };
    for (const auto& file: mruFiles)
        {
        if (wxFileName::FileExists(file))
            {
            m_fileButtons[buttonCount].m_id = ID_FILE_ID_START + buttonCount;
            m_fileButtons[buttonCount++].m_label = file;
            }
        // no more than 10 items here, not enough real estate
        if (buttonCount == MAX_FILE_BUTTONS)
            { break; }
        }
    m_fileButtons[buttonCount].m_id = START_PAGE_FILE_LIST_CLEAR;
    m_fileButtons[buttonCount++].m_label = _("Clear file list...");
    m_fileButtons.resize(buttonCount);
    }

//---------------------------------------------------
void wxStartPage::OnResize(wxSizeEvent& WXUNUSED(event))
    {
    wxClientDC dc(this);

    wxCoord appNameWidth{ 0 }, appNameHeight{ 0 },
            appDescWidth{ 0 }, appDescHeight{ 0 };
        {
        wxDCFontChanger fc(dc, m_logoFont);
        dc.GetTextExtent(wxTheApp->GetAppName(), &appNameWidth, &appNameHeight);
        }
    m_buttonsStart = m_logo.IsOk() ?
        GetTopBorder() + (2*GetLabelPaddingHeight()) +
            std::max(appNameHeight, GetAppLogoSize().GetHeight()) :
        GetTopBorder() + (2*GetLabelPaddingHeight()) + appNameHeight;
    if (m_productDescription.length())
        {
        dc.GetTextExtent(m_productDescription, &appDescWidth, &appDescHeight);
        m_buttonsStart += appDescHeight+(2*GetLabelPaddingHeight());
        }
    // calculate how wide the buttons/top label need to be fit their content
    const auto buttonIconSize = GetButtonSize();
    m_buttonHeight = buttonIconSize.GetHeight() + (2 * GetLabelPaddingHeight());
        {
        m_buttonWidth = std::max(m_buttonWidth,
            (m_logo.IsOk() ?
             (appNameWidth + GetAppLogoSize().GetWidth()+(2*GetLabelPaddingWidth())) :
              appNameWidth + (2*GetLabelPaddingWidth())));

        wxDCFontChanger fc(dc, m_buttons.size() > MAX_BUTTONS_SMALL_SIZE ?
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
        wxDCFontChanger fc(dc, dc.GetFont().Larger());
        m_fileColumnHeight = dc.GetTextExtent(_("Recent")).GetHeight() +
            (2 * GetLabelPaddingHeight());
        if (m_fileButtons.size())
            {
            wxDCFontChanger fc2(dc, wxFont(dc.GetFont()).MakeSmaller());
            wxCoord textWidth{ 0 }, textHeight{ 0 };
            dc.GetTextExtent(m_fileButtons[0].m_label, &textWidth, &textHeight);
            // enough space for the text (label and path) height
            // (or icon, whichever is larger) and some padding around it
            m_mruButtonHeight =
                std::max(textHeight*2,
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

    // calculate the positions of the buttons in the files area
    const wxRect filesArea = wxRect(m_buttonWidth+(GetLeftBorder() * 2),
        0,
        GetClientSize().GetWidth() - (m_buttonWidth+(GetLeftBorder() * 2)),
        GetClientSize().GetHeight());

    wxRect fileColumnHeader =
        wxRect(filesArea.GetLeft(), 0,
               filesArea.GetWidth(), m_fileColumnHeight).Deflate(1);

    for (size_t i = 0; i < GetMRUFileAndClearButtonCount(); ++i)
        {
        m_fileButtons[i].m_rect =
            wxRect(filesArea.GetLeft() + 1,
                   m_fileColumnHeight+(i * GetMRUButtonHeight()),
                   filesArea.GetWidth() - 2,
                   GetMRUButtonHeight());
        }

    // update the buttons' rects
    for (size_t i = 0; i < m_buttons.size(); ++i)
        {
        m_buttons[i].m_rect = wxRect(GetLeftBorder(),
                                     m_buttonsStart + (i * m_buttonHeight),
                                     m_buttonWidth,
                                     m_buttonHeight);
        }

    // fill the background
    dc.SetBackground(GetBackstageBackgroundColor());
    dc.Clear();

    // draw the program logo
        {
        wxCoord appDescWidth{ 0 }, appDescHeight{ 0 };
        if (m_productDescription.length())
            {
            dc.GetTextExtent(m_productDescription, &appDescWidth, &appDescHeight);
            appDescHeight += (2 * GetLabelPaddingHeight());
            }
        wxDCTextColourChanger cc(dc, GetBackstageFontColor());
        wxDCPenChanger pc(dc, GetBackstageFontColor());
        wxCoord textWidth{ 0 }, textHeight{ 0 };
        wxBitmap appLogo = m_logo.GetBitmap(GetAppLogoSize());
        if (appLogo.IsOk())
            {
            dc.DrawBitmap(appLogo, GetLeftBorder(), GetTopBorder());
            // draw with larger font
                {
                wxDCFontChanger fc(dc, m_logoFont);
                dc.GetTextExtent(wxTheApp->GetAppName(), &textWidth, &textHeight);
                dc.DrawText(wxTheApp->GetAppName(),
                            GetLeftBorder()+appLogo.GetWidth()+GetLabelPaddingWidth(),
                            GetTopBorder()+((appLogo.GetHeight()/2)-(textHeight/2)));
                }
            if (m_productDescription.length())
                {
                dc.DrawText(m_productDescription,
                            (GetLeftBorder()) + ((m_buttonWidth/2) - (appDescWidth/2)),
                            GetTopBorder() + std::max(appLogo.GetHeight(),textHeight) +
                            GetLabelPaddingHeight());
                }
            const auto lineY = GetTopBorder() +
                            std::max(appLogo.GetHeight(), textHeight) +
                            appDescHeight + GetLabelPaddingWidth();
            dc.DrawLine(wxPoint((2*GetLeftBorder()),lineY),
                        wxPoint(m_buttonWidth, lineY));
            }
        else
            {
            // draw with larger font
                {
                wxDCFontChanger fc(dc, m_logoFont);
                dc.GetTextExtent(wxTheApp->GetAppName(), &textWidth, &textHeight);
                dc.DrawText(wxTheApp->GetAppName(), GetLeftBorder(),
                            GetTopBorder()+GetLabelPaddingHeight());
                }
            if (m_productDescription.length())
                {
                dc.DrawText(m_productDescription,
                           (GetLeftBorder())+((m_buttonWidth/2) - (appDescWidth/2)),
                           GetTopBorder()+textHeight+GetLabelPaddingHeight());
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
        wxDCPenChanger pc(dc, *wxTRANSPARENT_PEN);
        wxDCBrushChanger bc(dc, GetDetailBackgroundColor());
        dc.DrawRectangle(filesArea);
        }
    // draw MRU column header
        {
        wxDCFontChanger fc(dc, dc.GetFont().Larger());
        wxDCTextColourChanger tcc(dc, GetDetailFontColor());
        wxDCPenChanger pc(dc, GetDetailFontColor());
        dc.SetClippingRegion(fileColumnHeader);
        dc.DrawLabel(_("Recent"),
            wxRect(fileColumnHeader).Deflate(GetLabelPaddingWidth()),
            wxALIGN_CENTRE);
        dc.DestroyClippingRegion();
        dc.DrawLine(fileColumnHeader.GetLeftBottom() +
                        wxSize((fileColumnHeader.GetWidth()/10), 0),
                    fileColumnHeader.GetRightBottom() -
                        wxSize((fileColumnHeader.GetWidth()/10), 0));
        }

    const wxString currentToolTip = m_toolTip;
    // highlight the active MRU file or backstage button
    if (m_activeButton != wxNOT_FOUND)
        {
        wxRect buttonBorderRect;
        for (size_t i = 0; i < GetMRUFileAndClearButtonCount(); ++i)
            {
            if (m_fileButtons[i].IsOk() &&
                m_activeButton == m_fileButtons[i].m_id)
                {
                m_toolTip = m_fileButtons[i].m_label;
                buttonBorderRect = m_fileButtons[i].m_rect;
                break;
                }
            }
        for (const auto& button : m_buttons)
            {
            if (m_activeButton == button.m_id)
                {
                buttonBorderRect = button.m_rect;
                m_toolTip.clear();
                break;
                }
            }
        if (!buttonBorderRect.IsEmpty())
            {
            DrawHighlight(dc, buttonBorderRect, GetHoverColor());
            }
        }
    else
        { m_toolTip.clear(); }

    if (currentToolTip != m_toolTip)
        { SetToolTip(m_toolTip); }

    const auto formatFileDateTime = [](const auto& dt)
        {
        wxString dateStr, timeStr;
        if (wxDateTime::Now().GetYear() == dt.GetYear() &&
            wxDateTime::Now().GetMonth() == dt.GetMonth() &&
            wxDateTime::Now().GetWeekOfMonth() == dt.GetWeekOfMonth())
            {
            // named day if accessed sometime this week
            if (wxDateTime::Now().GetDay() == dt.GetDay())
                { dateStr = _("Today"); }
            else if (wxDateTime::Now().GetDay() - 1 == dt.GetDay())
                { dateStr = _("Yesterday"); }
            else
                { dateStr = wxDateTime::GetWeekDayName(dt.GetWeekDay()); }

            // include time also, in the local clock format
            wxString am, pm;
            wxDateTime::GetAmPmStrings(&am, &pm);
            if (am.length() && pm.length())
                { timeStr = dt.Format(L"  %I:%M %p").MakeUpper(); }
            else
                { timeStr = dt.Format(L"  %H:%M"); }
            }
        else
            {
            // only show year if accessed last year
            const wxString dateFormatStr =
                (wxDateTime::Now().GetYear() == dt.GetYear()) ?
                    L"%B %d " :
                    L"%B %d, %Y";
            dateStr = dt.Format(dateFormatStr);
            }

        return dateStr + timeStr;
        };

    // file labels
        {
        // Get the widest file access time label so that we can draw 
        // them ragged right. Also, get the longest file path to make
        // sure the time and path don't overlap.
        decltype(wxSize::x) filePathLabelWidth{ 0 };
        decltype(wxSize::x) timeLabelWidth{ 0 };
            {
            wxDCFontChanger fc(dc, wxFont(dc.GetFont()).MakeSmaller());
            for (size_t i = 0; i < GetMRUFileCount(); ++i)
                {
                if (m_fileButtons[i].IsOk())
                    {
                    const wxFileName fn(m_fileButtons[i].m_label);
                    wxDateTime accessTime, modTime, createTime;
                    if (fn.GetTimes(&accessTime, &modTime, &createTime))
                        {
                        const wxString accessTimeStr =
                            formatFileDateTime(accessTime);
                        const wxSize timeStringSize =
                            dc.GetTextExtent(accessTimeStr);
                        if (timeLabelWidth < timeStringSize.GetWidth())
                            { timeLabelWidth = timeStringSize.GetWidth(); }
                        }
                    const wxSize filePathStringSize =
                        dc.GetTextExtent(fn.GetFullPath());
                    if (filePathLabelWidth < filePathStringSize.GetWidth())
                        { filePathLabelWidth = filePathStringSize.GetWidth(); }
                    }
                }
            }
        // set file path font color to be slightly ligher/darker
        // than file name color
        const wxColour filePathColor =
            (GetDetailFontColor().GetLuminance() < 0.5f) ?
                GetDetailFontColor().ChangeLightness(160) :
                GetDetailFontColor().ChangeLightness(40);
        // begin drawing them
        wxBitmap fileIcon = m_logo.GetBitmap(FromDIP(wxSize(16, 16)));
        for (size_t i = 0; i < GetMRUFileAndClearButtonCount(); ++i)
            {
            if (m_fileButtons[i].IsOk())
                {
                wxDCTextColourChanger tcc(dc,
                    m_activeButton == m_fileButtons[i].m_id ?
                    GetHoverFontColor() : GetDetailFontColor());
                const wxRect fileLabelRect =
                    wxRect{ m_fileButtons[i].m_rect }.Deflate(GetLabelPaddingHeight());
                dc.SetClippingRegion(m_fileButtons[i].m_rect);
                // if the "clear all" button
                if (i == GetMRUFileAndClearButtonCount() - 1)
                    {
                    wxDCPenChanger pc(dc, *wxLIGHT_GREY_PEN);
                    dc.DrawLine(m_fileButtons[i].m_rect.GetLeftTop(),
                                m_fileButtons[i].m_rect.GetRightTop());
                    dc.DrawLabel(m_fileButtons[i].m_label, fileLabelRect,
                                 wxALIGN_LEFT|wxALIGN_CENTRE_VERTICAL);
                    }
                else
                    {
                    // show the files
                    const wxFileName fn(m_fileButtons[i].m_label);
                    if (fileIcon.IsOk())
                        {
                        dc.DrawBitmap(fileIcon,
                            wxPoint(fileLabelRect.GetLeft(),
                                    fileLabelRect.GetTop() +
                                        (fileLabelRect.GetHeight() -
                                         fileIcon.GetHeight(), 2)));
                        // draw the filename
                        const auto nameHeight =
                            dc.GetTextExtent(fn.GetFullName()).GetHeight();
                        dc.DrawText(fn.GetFullName(),
                            wxPoint(fileLabelRect.GetLeft() +
                                    GetLabelPaddingWidth() + fileIcon.GetWidth(),
                                    fileLabelRect.GetTop()));
                        wxDCFontChanger fc(dc, wxFont(dc.GetFont()).MakeSmaller());
                        // draw the filepath
                            {
                            wxDCTextColourChanger cc(dc, filePathColor);
                            dc.DrawText(
                                // truncate the path is necessary
                                (fn.GetFullPath().length() <= 75) ?
                                fn.GetFullPath() :
                                (fn.GetFullPath().substr(0,75) + L"..."),
                                wxPoint(fileLabelRect.GetLeft() +
                                        GetLabelPaddingWidth() +
                                        fileIcon.GetWidth(),
                                        fileLabelRect.GetTop() + nameHeight +
                                        (GetLabelPaddingHeight()/2)));
                            }
                        // draw the modified time off to the side
                        if ((fileIcon.GetWidth() +
                             GetLabelPaddingWidth() +
                             filePathLabelWidth + timeLabelWidth) <
                            fileLabelRect.GetWidth())
                            {
                            wxDateTime accessTime, modTime, createTime;
                            if (fn.GetTimes(&accessTime, &modTime, &createTime))
                                {
                                const wxString accessTimeStr =
                                    formatFileDateTime(accessTime);
                                const wxSize timeStringSize =
                                    dc.GetTextExtent(accessTimeStr);
                                dc.DrawText(accessTimeStr,
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
                    }
                dc.DestroyClippingRegion();
                }
            }
        }

    // draw the backstage button labels
        {
        const auto buttonIconSize = GetButtonSize();
        m_buttonHeight = buttonIconSize.GetHeight() + (2 * GetLabelPaddingHeight());
        wxDCFontChanger fc(dc, m_buttons.size() > MAX_BUTTONS_SMALL_SIZE ?
                               dc.GetFont() :
                               dc.GetFont().Larger());
        for (const auto& button : m_buttons)
            {
            if (button.IsOk())
                {
                wxDCTextColourChanger cc(dc,
                    m_activeButton == button.m_id ?
                    GetHoverFontColor() : GetBackstageFontColor());

                // draw it
                dc.SetClippingRegion(button.m_rect);
                dc.DrawLabel(button.m_label, button.m_icon.GetBitmap(buttonIconSize),
                    wxRect(button.m_rect).Deflate(GetLabelPaddingWidth()));
                dc.DestroyClippingRegion();
                }
            }
        }
    }

//---------------------------------------------------
void wxStartPage::OnMouseChange(wxMouseEvent& event)
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

    m_activeButton = wxNOT_FOUND;
    for (const auto& button : m_buttons)
        {
        if (button.IsOk() &&
            button.m_rect.Contains(event.GetX(), event.GetY()) )
            {
            m_activeButton = button.m_id;
            // just refresh the current and previous (if applicable) highlighted areas
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
                        _("Do you wish to clear the list of recent files?"),
                        _("Clear File List"), wxYES_NO|wxICON_QUESTION) == wxYES)
                    {
                    SetMRUList(wxArrayString());
                    Refresh();
                    Update();
                    // give the caller a change to clear the file history
                    // from their doc manager and whatnot
                    wxCommandEvent cevent(wxEVT_STARTPAGE_CLICKED, GetId());
                    cevent.SetId(START_PAGE_FILE_LIST_CLEAR);
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
                // selected file path
                cevent.SetString(m_fileButtons[i].m_label);
                cevent.SetEventObject(this);
                GetEventHandler()->ProcessEvent(cevent);
                break;
                }
            }
        }
    }
