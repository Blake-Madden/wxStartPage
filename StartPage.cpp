/* copyright (c) Oleander Software, Ltd.
   author: Blake Madden
   This program is free software; you can redistribute it and/or modify
   it under the terms of the BSD License.
*/

#include "StartPage.h"

DEFINE_EVENT_TYPE(wxEVT_STARTPAGE_CLICKED)

wxStartPage::wxStartPage(wxWindow* parent, wxWindowID id /*= wxID_ANY*/, const wxArrayString& mruFiles /*= wxArrayString()*/, const wxBitmap& logo /*= wxNullBitmap*/, const wxBitmap& fileImage /*= wxNullBitmap*/, const wxString productDescription /*= wxEmptyString*/) 
        : wxWindow(parent, id, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE, wxT("wxStartPage")),
          m_buttonsStart(0), m_fileColumnHeight(),
          m_activeButton(-1), m_logo(logo),
          m_logoFont(wxFontInfo(static_cast<double>(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT).GetPointSize())*2)),
          m_buttonFont(wxFontInfo(static_cast<double>(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT).GetPointSize())*1.5)),
          m_style(wxStartPageStyle::wxStartPageFlat), m_productDescription(productDescription),
          m_backstageBackgroundColor(wxColour(145,168,208)),
          m_backstageFontColor(*wxWHITE),
          m_detailBackgroundColor(*wxWHITE),
          m_detailFontColor(*wxBLACK),
          m_hoverColor(wxColour(100,140,250)),
          m_hoverFontColor(*wxBLACK)
    {
    // Size of an icon scaled to 16x16, with label padding above and below it.
    // Note that Realise will adjust this later more intelligently.
    m_buttonHeight = FromDIP(wxSize(16+10,16+10)).GetHeight();
    m_buttonWidth = FromDIP(wxSize(200,200)).GetWidth();

    const wxSize scaledSize = FromDIP(wxSize(16,16));
    m_fileImage = fileImage.ConvertToImage().Scale(scaledSize.GetWidth(), scaledSize.GetHeight(), wxIMAGE_QUALITY_HIGH);

    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
    SetMRUList(mruFiles);

    Bind(wxEVT_PAINT, &wxStartPage::OnPaint, this);
    Bind(wxEVT_MOTION, &wxStartPage::OnMouseChange, this);
    Bind(wxEVT_LEFT_DOWN, &wxStartPage::OnMouseClick, this);
    }

void wxStartPage::DrawHighlight(wxDC& dc, const wxRect rect, const wxColour color) const
    {
    wxDCPenChanger pc(dc, *wxLIGHT_GREY_PEN);
    if (m_style == wxStartPageStyle::wxStartPage3D)
        {
        // fill with the color
        dc.GradientFillLinear(rect, color, color.ChangeLightness(140), wxSOUTH);
        // create a shiny overlay
        dc.GradientFillLinear(wxRect(rect.GetX(), rect.GetY(),
                                rect.GetWidth(), rect.GetHeight()*.25),
                                wxColour(255,255,255,25), wxColour(255,255,255,125), wxSOUTH);
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

void wxStartPage::SetMRUList(const wxArrayString& mruFiles)
    {
    m_fileButtons.clear();
    m_fileButtons.resize(mruFiles.GetCount()+1);
    if (mruFiles.GetCount() == 0)
        { return; }
    size_t i{ 0 };
    for (; i < mruFiles.GetCount(); ++i)
        {
        m_fileButtons[i].m_id = ID_FILE_ID_START+i;
        m_fileButtons[i].m_label = mruFiles[i];
        }
    m_fileButtons[i].m_id = wxSTART_PAGE_FILE_LIST_CLEAR;
    m_fileButtons[i].m_label = _("Clear file list...");
    }

void wxStartPage::Realise()
    {
    wxClientDC dc(this);

    wxCoord appNameWidth(0), appNameHeight(0), appDescWidth(0), appDescHeight(0);
        {
        wxDCFontChanger fc(dc, m_logoFont);
        dc.GetTextExtent(wxTheApp->GetAppName(), &appNameWidth, &appNameHeight);
        }
    m_buttonsStart = m_logo.IsOk() ? GetTopBorder()+(2*GetLabelPaddingHeight())+wxMax(appNameHeight,m_logo.GetHeight()) : GetTopBorder()+(2*GetLabelPaddingHeight())+appNameHeight;
    if (m_productDescription.length())
        {
        dc.GetTextExtent(m_productDescription, &appDescWidth, &appDescHeight);
        m_buttonsStart += appDescHeight+(2*GetLabelPaddingHeight());
        }
    // calculate how wide the buttons/top label need to be fit their content
        {
        m_buttonWidth = wxMax(m_buttonWidth, (m_logo.IsOk() ? (appNameWidth+m_logo.GetWidth()+(2*GetLabelPaddingWidth())) : appNameWidth+(2*GetLabelPaddingWidth())));

        wxDCFontChanger fc(dc, m_buttonFont);
        wxCoord textWidth(0), textHeight(0);
        for (size_t i = 0; i < m_buttons.size(); ++i)
            {
            dc.GetTextExtent(m_buttons[i].m_label, &textWidth, &textHeight);
            m_buttonWidth = wxMax(m_buttonWidth, textWidth+(4*GetLabelPaddingWidth())+m_buttons[i].m_icon.GetWidth());
            m_buttonHeight = wxMax(m_buttonHeight, textHeight+(2*GetLabelPaddingWidth()));
            if (m_buttons[i].m_icon.IsOk())
                { m_buttonHeight = wxMax(m_buttonHeight, m_buttons[i].m_icon.GetHeight()+(2*GetLabelPaddingHeight())); }
            }
        }
    // initialize the buttons
    for (size_t i = 0; i < m_buttons.size(); ++i)
        {
        m_buttons[i].m_id = ID_BUTTON_ID_START+i;
        m_buttons[i].m_rect = wxRect(GetLeftBorder(),m_buttonsStart+(i*m_buttonHeight),m_buttonWidth,m_buttonHeight);
        }

    // calculate MRU info
        {
        wxDCFontChanger fc(dc, m_buttonFont);
        m_fileColumnHeight = dc.GetTextExtent(_("Recent")).GetHeight()+(2*GetLabelPaddingHeight());
        if (m_fileButtons.size())
            {
            wxCoord textWidth(0), textHeight(0);
            dc.GetTextExtent(m_fileButtons[0].m_label, &textWidth, &textHeight);
            // enough space for the text height (or icon, whichever is larger) and some padding around it
            m_mruButtonHeight = wxMax(textHeight, FromDIP(wxSize(16,16)).GetHeight()) +
                                (2*GetLabelPaddingHeight());
            }
        }
    }

void wxStartPage::OnPaint([[maybe_unused]] wxPaintEvent& event)
    {
    wxAutoBufferedPaintDC adc(this);
    adc.Clear();
    wxGCDC dc(adc);

    wxRect fileColumnHeader;
    wxRect childArea;
    // calculate the positions of the buttons in the child area
        {
        childArea = wxRect(m_buttonWidth+(GetLeftBorder()*2), 0, GetClientSize().GetWidth()-(m_buttonWidth+(GetLeftBorder()*2)), GetClientSize().GetHeight());

        fileColumnHeader = wxRect(childArea.GetLeft(), 0, childArea.GetWidth(), m_fileColumnHeight);
        fileColumnHeader.Deflate(1);

        for (size_t i = 0; i < GetMRUFileAndClearButtonCount(); ++i)
            {
            m_fileButtons[i].m_rect = wxRect(childArea.GetLeft()+1,
                                             m_fileColumnHeight+(i*GetMruButtonHeight()),
                                             childArea.GetWidth()-2,
                                             GetMruButtonHeight());
            }
        }

    //fill the background
    dc.SetBackground(GetBackstageBackgroundColor());
    dc.Clear();

    // draw the program logo
        {
        wxCoord appDescWidth(0), appDescHeight(0);
        if (m_productDescription.length())
            {
            dc.GetTextExtent(m_productDescription, &appDescWidth, &appDescHeight);
            appDescHeight += (2*GetLabelPaddingHeight());
            }
        wxDCTextColourChanger cc(dc, GetBackstageFontColor());
        wxDCPenChanger pc(dc, GetBackstageFontColor());
        wxCoord textWidth(0), textHeight(0);
        if (m_logo.IsOk())
            {
            dc.DrawBitmap(m_logo, GetLeftBorder(), GetTopBorder());
            //draw with larger font
                {
                wxDCFontChanger fc(dc, m_logoFont);
                dc.GetTextExtent(wxTheApp->GetAppName(), &textWidth, &textHeight);
                dc.DrawText(wxTheApp->GetAppName(), GetLeftBorder()+m_logo.GetWidth()+GetLabelPaddingWidth(), GetTopBorder()+((m_logo.GetHeight()/2)-(textHeight/2)));
                }
            if (m_productDescription.length())
                {
                dc.DrawText(m_productDescription, (GetLeftBorder()) +((m_buttonWidth/2) - (appDescWidth/2)),
                    GetTopBorder()+wxMax(m_logo.GetHeight(),textHeight)+GetLabelPaddingHeight());
                }
            dc.DrawLine(wxPoint((2*GetLeftBorder()), GetTopBorder()+wxMax(m_logo.GetHeight(),textHeight)+appDescHeight),
                        wxPoint(m_buttonWidth, GetTopBorder()+wxMax(m_logo.GetHeight(),textHeight)+appDescHeight));
            }
        else
            {
            //draw with larger font
                {
                wxDCFontChanger fc(dc, m_logoFont);
                dc.GetTextExtent(wxTheApp->GetAppName(), &textWidth, &textHeight);
                dc.DrawText(wxTheApp->GetAppName(), GetLeftBorder(), GetTopBorder()+GetLabelPaddingHeight());
                }
            if (m_productDescription.length())
                {
                dc.DrawText(m_productDescription, (GetLeftBorder())+((m_buttonWidth/2) - (appDescWidth/2)),
                    GetTopBorder()+textHeight+GetLabelPaddingHeight());
                }
            dc.DrawLine(wxPoint((2*GetLeftBorder()), GetTopBorder()+textHeight+GetLabelPaddingHeight()+appDescHeight),
                        wxPoint(m_buttonWidth, GetTopBorder()+textHeight+GetLabelPaddingHeight()+appDescHeight));
            }
        }

    // draw the child area
        {
        wxDCPenChanger pc(dc, *wxTRANSPARENT_PEN);
        wxDCBrushChanger bc(dc, GetDetailBackgroundColor());
        dc.DrawRectangle(childArea);
        }
    // draw MRU column header
        {
        wxDCFontChanger fc(dc, m_buttonFont);
        wxDCTextColourChanger tcc(dc, GetDetailFontColor());
        wxDCPenChanger pc(dc, GetDetailFontColor());
        dc.SetClippingRegion(fileColumnHeader);
        dc.DrawLabel(_("Recent"), wxRect(fileColumnHeader).Deflate(GetLabelPaddingWidth()), wxALIGN_CENTRE);
        dc.DestroyClippingRegion();
        dc.DrawLine(fileColumnHeader.GetLeftBottom()+wxSize((fileColumnHeader.GetWidth()/10),0), fileColumnHeader.GetRightBottom()-wxSize((fileColumnHeader.GetWidth()/10),0));
        }

    const wxString currentToolTip = m_toolTip;
    // highlight the active MRU file or backstage button
    if (m_activeButton != -1)
        {
        wxRect buttonBorderRect;
        for (size_t i = 0; i < GetMRUFileAndClearButtonCount(); ++i)
            {
            if (m_fileButtons[i].IsOk() && m_activeButton == m_fileButtons[i].m_id)
                {
                m_toolTip = m_fileButtons[i].m_label;
                buttonBorderRect = m_fileButtons[i].m_rect;
                break;
                }
            }
        for (size_t i = 0; i < m_buttons.size(); ++i)
            {
            if (m_activeButton == m_buttons[i].m_id)
                {
                buttonBorderRect = m_buttons[i].m_rect;
                m_toolTip.clear();
                break;
                }
            }
        if (!buttonBorderRect.IsEmpty())
            {
            DrawHighlight(dc, wxRect(buttonBorderRect.GetLeftTop().x, buttonBorderRect.GetLeftTop().y,
                          buttonBorderRect.GetWidth(), buttonBorderRect.GetHeight()), GetHoverColor());
            }
        }
    else
        { m_toolTip.clear(); }
    if (currentToolTip != m_toolTip)
        { SetToolTip(m_toolTip); }

    // draw the file labels
        {
        for (size_t i = 0; i < GetMRUFileAndClearButtonCount(); ++i)
            {
            if (m_fileButtons[i].IsOk())
                {
                wxDCTextColourChanger tcc(dc,
                    m_activeButton == m_fileButtons[i].m_id ?
                    GetHoverFontColor() : GetDetailFontColor());
                wxRect fileLabelRect = wxRect(m_fileButtons[i].m_rect).Deflate(GetLabelPaddingHeight());
                dc.SetClippingRegion(m_fileButtons[i].m_rect);
                // if the "clear all" button
                if (i == GetMRUFileAndClearButtonCount()-1)
                    {
                    wxDCPenChanger pc(dc, *wxLIGHT_GREY_PEN);
                    dc.DrawLine(m_fileButtons[i].m_rect.GetLeftTop(), m_fileButtons[i].m_rect.GetRightTop());
                    dc.DrawLabel(m_fileButtons[i].m_label, fileLabelRect, wxALIGN_LEFT|wxALIGN_CENTRE_VERTICAL);
                    }
                else
                    {
                    // just show the file name on the screen
                    const wxFileName fn(m_fileButtons[i].m_label);
                    if (m_fileImage.IsOk())
                        { dc.DrawLabel(fn.GetName(), m_fileImage, fileLabelRect); }
                    else
                        { dc.DrawLabel(fn.GetName(), fileLabelRect, wxALIGN_LEFT|wxALIGN_CENTRE_VERTICAL); }
                    }
                dc.DestroyClippingRegion();
                }
            }
        }

    // draw the backstage button labels
        {
        wxDCFontChanger fc(dc, m_buttonFont);
        for (size_t i = 0; i < m_buttons.size(); ++i)
            {
            if (m_buttons[i].IsOk())
                {
                wxDCTextColourChanger cc(dc,
                    m_activeButton == m_buttons[i].m_id ?
                    GetHoverFontColor() : GetBackstageFontColor());
                wxRect buttonLabelRect = wxRect(m_buttons[i].m_rect).Deflate(GetLabelPaddingWidth());
                buttonLabelRect.SetWidth(buttonLabelRect.GetWidth()-GetLabelPaddingWidth()-m_buttons[i].m_icon.GetWidth());
                //draw it
                dc.SetClippingRegion(wxRect(m_buttons[i].m_rect).Deflate(GetLabelPaddingWidth()));
                dc.DrawLabel(m_buttons[i].m_label, m_buttons[i].m_icon, wxRect(m_buttons[i].m_rect).Deflate(GetLabelPaddingWidth()));
                dc.DestroyClippingRegion();
                }
            }
        }
    }

void wxStartPage::OnMouseChange(wxMouseEvent& event)
    {
    m_activeButton = -1;
    for (size_t i = 0; i < m_buttons.size(); ++i)
        {
        if (m_buttons[i].IsOk() && m_buttons[i].m_rect.Contains(event.GetX(), event.GetY()) )
            {
            m_activeButton = m_buttons[i].m_id;
            Refresh();
            Update();
            return;
            }
        }
    for (size_t i = 0; i < GetMRUFileAndClearButtonCount(); ++i)
        {
        if (m_fileButtons[i].IsOk() && m_fileButtons[i].m_rect.Contains(event.GetX(), event.GetY()) )
            {
            m_activeButton = m_fileButtons[i].m_id;
            Refresh();
            Update();
            return;
            }
        }
    Refresh();
    Update();
    }

void wxStartPage::OnMouseClick(wxMouseEvent& event)
    {
    for (size_t i = 0; i < m_buttons.size(); ++i)
        {
        if (m_buttons[i].IsOk() && m_buttons[i].m_rect.Contains(event.GetX(), event.GetY()) )
            {
            wxCommandEvent cevent(wxEVT_STARTPAGE_CLICKED, GetId());
            cevent.SetInt(m_buttons[i].m_id);
            cevent.SetEventObject(this);
            GetEventHandler()->ProcessEvent(cevent);
            break;
            }
        }
    for (size_t i = 0; i < GetMRUFileAndClearButtonCount(); ++i)
        {
        if (m_fileButtons[i].IsOk() && m_fileButtons[i].m_rect.Contains(event.GetX(), event.GetY()) )
            {
            // if clicking the clear all button
            if (i == GetMRUFileAndClearButtonCount()-1)
                {
                if (wxMessageBox(_("Do you wish to clear the list of recent files?"),
                        _("Clear File List"), wxYES_NO|wxICON_QUESTION) == wxYES)
                    {
                    SetMRUList(wxArrayString());
                    Refresh();
                    Update();
                    // give the caller a change to clear the file history from their doc manager
                    // and whatnot
                    wxCommandEvent cevent(wxEVT_STARTPAGE_CLICKED, GetId());
                    cevent.SetInt(wxSTART_PAGE_FILE_LIST_CLEAR);
                    cevent.SetEventObject(this);
                    GetEventHandler()->ProcessEvent(cevent);
                    break;
                    }
                }
            else
                {
                wxCommandEvent cevent(wxEVT_STARTPAGE_CLICKED, GetId());
                cevent.SetInt(m_fileButtons[i].m_id);
                cevent.SetEventObject(this);
                GetEventHandler()->ProcessEvent(cevent);
                break;
                }
            }
        }
    }
