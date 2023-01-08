/** @date 2015-2023
    @copyright Blake Madden
    @author Blake Madden
    @details This program is free software; you can redistribute it and/or modify
     it under the terms of the 3-Clause BSD License.

     SPDX-License-Identifier: BSD-3-Clause
@{*/

#ifndef __WXSTART_PAGE_H__
#define __WXSTART_PAGE_H__

#include <wx/wx.h>
#include <wx/window.h>
#include <wx/filename.h>
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>
#include <wx/artprov.h>
#include <vector>

wxDECLARE_EVENT(wxEVT_STARTPAGE_CLICKED, wxCommandEvent);

#define EVT_STARTPAGE_CLICKED(winid, fn) \
    wx__DECLARE_EVT1(wxEVT_STARTPAGE_CLICKED, winid, wxCommandEventHandler(fn))

/** @brief A wxWidgets landing page for an application.

    It displays an MRU list on the right side (up to 10 files) and a list of
    customizable buttons on the left.

    The events of a user clicking on a file or button can be handled via
    an @c EVT_STARTPAGE_CLICKED message map or bound to @c wxEVT_STARTPAGE_CLICKED.
    (This should be bound to a function accepting a @c wxCommandEvent object.)
    Calling the `wxCommandEvent`'s @c GetId() method in your handler will return the
    ID of the button that was clicked. This ID can be checked by:
    - Calling IsCustomButtonId() to see if a custom button was clicked.
      If so, then call GetButtonID() to see which button it was. For example,
      If you have 5 buttons, then you can call `GetButtonID(0)`-`GetButtonID(4)`
      and compare the event's ID against that to see which button it was.
    - Calling IsFileId() to see if a file button was clicked.
      If @c true, then you can get the selected file path from the event's
      string value.
    - Calling IsFileListClearId() to see if the "Clear File List" button was clicked.
*/
class wxStartPage final : public wxWindow
    {
public:
    /// @brief The appearance of the buttons on the start page.
    enum class wxStartPageStyle
        {
        wxStartPageFlat, /*!<Flat button appearance.*/
        wxStartPage3D    /*!<3D button appearance.*/
        };
    /** @brief Constructor.
        @param parent The parent window.
        @param id The start page's ID.
        @param mruFiles A list of file paths to show in the
            most-recently-used file list.
        @param logo A logo image of the program to show on the left side,
            as well as the files in the MRU list.
        @param productDescription An optional description of the application to
            show on the left side (under the application's name).*/
    explicit wxStartPage(wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxArrayString& mruFiles = wxArrayString{},
        const wxBitmapBundle& logo = wxBitmapBundle{},
        const wxString productDescription = wxString{});
    /// @private
    wxStartPage() = delete;
    /// @private
    wxStartPage(const wxStartPage&) = delete;
    /// @private
    wxStartPage(wxStartPage&&) = delete;
    /// @private
    wxStartPage& operator=(const wxStartPage&) = delete;
    /// @private
    wxStartPage& operator=(wxStartPage&&) = delete;
    /// @brief Adds a feature button on the left side.
    /// @details A feature button can be something like
    ///     "Read the Help" or "Create a New Project."
    /// @param bmp The image for the button.
    /// @param label The label on the button.
    void AddButton(const wxBitmapBundle& bmp, const wxString& label)
        { m_buttons.push_back(wxStartPageButton(bmp, label)); }
    /// @returns The ID of the given index into the custom button list,
    ///     or returns @c wxNOT_FOUND if an invalid index is given.
    /// @param buttonIndex The index into the custom button list.
    wxNODISCARD const int GetButtonID(const size_t buttonIndex) const noexcept
        {
        return buttonIndex > m_buttons.size() ?
            wxNOT_FOUND :
            m_buttons[buttonIndex].m_id;
        }
    /// @brief Sets the list of files to be shown in the
    ///     "most-recently-used" list on the right side.
    /// @param mruFiles The list of file names.
    /// @note Files that can't be found will be filtered out,
    ///     although they will remain in the parent application's MRU list.\n
    ///     That way, if a user is disconnected from their network,
    ///     then any network files won't appear, but may appear next time
    ///     if they are then connected to the network.
    void SetMRUList(const wxArrayString& mruFiles);
    /// @returns @c true if @c Id is an ID within the MRU list.
    /// @param Id The ID from an @c EVT_STARTPAGE_CLICKED event after a
    ///     user clicks a button on the start page.
    wxNODISCARD constexpr bool IsFileId(const wxWindowID Id) const noexcept
        { return (Id >= ID_FILE_ID_START && Id < START_PAGE_FILE_LIST_CLEAR); }
    /// @returns @c true if @c Id is the "Clear File List" button.
    /// @param Id The ID from an @c EVT_STARTPAGE_CLICKED event after a
    ///     user clicks a button on the start page.
    wxNODISCARD constexpr bool IsFileListClearId(const wxWindowID Id) const noexcept
        { return (Id == START_PAGE_FILE_LIST_CLEAR); }
    /// @returns @c true if @c Id is and ID for one of the custom buttons on the left.
    /// @param Id The ID from an @c EVT_STARTPAGE_CLICKED event after a
    ///     user clicks a button on the start page.
    wxNODISCARD bool IsCustomButtonId(const wxWindowID Id) const noexcept
        {
        return (Id >= ID_BUTTON_ID_START &&
                static_cast<size_t>(Id) < ID_BUTTON_ID_START + m_buttons.size());
        }
    /// @brief Sets the appearance of the start page.
    /// @param style The style for the start page.
    void SetStyle(const wxStartPageStyle style) noexcept
        { m_style = style; }
    /// @returns The color of the left side of the start page.
    wxNODISCARD wxColour GetBackstageBackgroundColor() const noexcept
        { return m_backstageBackgroundColor; }
    /// @brief Sets the color of the left side of the start page.
    /// @param color The color to use.
    void SetBackstageBackgroundColor(const wxColour& color) noexcept
        { m_backstageBackgroundColor = color; }
    /// @returns The font color of the left side of the start page.
    wxNODISCARD wxColour GetBackstageFontColor() const noexcept
        { return m_backstageFontColor; }
    /// @brief Sets the font color of the left side of the start page.
    /// @param color The color to use.
    void SetBackstageFontColor(const wxColour& color) noexcept
        { m_backstageFontColor = color; }
    /// @returns The color of the right side of the start page.
    wxNODISCARD wxColour GetDetailBackgroundColor() const noexcept
        { return m_detailBackgroundColor; }
    /// @brief Sets the color of the right side of the start page.
    /// @param color The color to use.
    void SetDetailBackgroundColor(const wxColour& color) noexcept
        { m_detailBackgroundColor = color; }
    /// @returns The font color of the right side of the start page.
    wxNODISCARD wxColour GetDetailFontColor() const noexcept
        { return m_detailFontColor; }
    /// @brief Sets the font color of the right side of the start page.
    /// @param color The color to use.
    void SetDetailFontColor(const wxColour& color) noexcept
        { m_detailFontColor = color; }
    /// @returns The color of buttons when they are hovered over.
    wxNODISCARD wxColour GetHoverColor() const noexcept
        { return m_hoverColor; }
    /// @brief Sets the color of buttons when they are hovered over.
    /// @param color The color to use.
    void SetHoverColor(const wxColour& color) noexcept
        { m_hoverColor = color; }
    /// @returns The font color of buttons when they are hovered over.
    wxNODISCARD wxColour GetHoverFontColor() const noexcept
        { return m_hoverFontColor; }
    /// @brief Sets the font color of buttons when they are hovered over.
    /// @param color The color to use.
    void SetHoverFontColor(const wxColour& color) noexcept
        { m_hoverFontColor = color; }
private:
    struct wxStartPageButton
        {
        wxStartPageButton(const wxBitmapBundle& icon, const wxString& label) :
            m_icon(icon), m_label(label)
            {}
        wxStartPageButton() = default;
        wxNODISCARD bool IsOk() const
            { return m_label.length() > 0; }
        wxRect m_rect;
        wxBitmapBundle m_icon;
        wxString m_label;
        wxWindowID m_id{ wxNOT_FOUND };
        };

    /// @returns The number of items in the MRU list.
    /// @note This is the number of files in the list,
    ///     not including the "clear all" button.
    wxNODISCARD const size_t GetMRUFileCount() const noexcept
        {
        // the last item is the "clear list" button, so don't count that
        return m_fileButtons.size() > 0 ?
            m_fileButtons.size() - 1 :
            0;
        }

    void OnResize(wxSizeEvent& WXUNUSED(event));
    void OnPaintWindow(wxPaintEvent& WXUNUSED(event));
    void OnMouseChange(wxMouseEvent& event);
    void OnMouseClick(wxMouseEvent& event);
    void OnMouseLeave(wxMouseEvent& WXUNUSED(event));
    static constexpr int MAX_BUTTONS_SMALL_SIZE = 8;
    static constexpr int MAX_FILE_BUTTONS = 10;
    // supports 10 MRU file buttons
    static constexpr int ID_FILE_ID_START = wxID_HIGHEST;
    static constexpr int ID_BUTTON_ID_START =
        wxID_HIGHEST + MAX_FILE_BUTTONS + 1;
    /// @brief ID returned when the "Clear file list" button is clicked.
    /// @details Client code can check for this in their @c wxEVT_STARTPAGE_CLICKED
    ///     code and clear the application's file history.
    static constexpr int START_PAGE_FILE_LIST_CLEAR =
        wxID_HIGHEST + MAX_FILE_BUTTONS;
    /// @returns The number of items in the MRU list
    ///     (including the "clear list" button).
    wxNODISCARD const size_t GetMRUFileAndClearButtonCount() const noexcept
        { return m_fileButtons.size(); }
    /// @returns The padding height around the labels.
    wxNODISCARD const wxCoord GetLabelPaddingHeight() const
        { return wxSizerFlags::GetDefaultBorder(); }
    /// @returns The padding width around the labels.
    wxNODISCARD const wxCoord GetLabelPaddingWidth() const
        { return wxSizerFlags::GetDefaultBorder(); }
    /// @returns The padding at the top of the control.
    wxNODISCARD const wxCoord GetTopBorder() const
        { return wxSizerFlags::GetDefaultBorder() * 4; }
    /// @returns The left border around the icons/labels.
    wxNODISCARD const wxCoord GetLeftBorder() const
        { return wxSizerFlags::GetDefaultBorder() * 4; }
    /// @returns The size for the app logo
    wxNODISCARD const wxSize GetAppLogoSize() const
        { return FromDIP(wxSize(64, 64)); }
    /// @returns The button size, which will be smaller if there are numerous buttons.
    wxNODISCARD const wxSize GetButtonSize() const
        {
        return FromDIP(m_buttons.size() > MAX_BUTTONS_SMALL_SIZE ?
                       wxSize(16, 16) : wxSize(32, 32));
        }
    /// @returns The size of an icon scaled to 16x16,
    ///     with label padding above and below it.
    wxNODISCARD const wxCoord GetMRUButtonHeight() const noexcept
        { return m_mruButtonHeight; }
    void DrawHighlight(wxDC& dc, const wxRect rect, const wxColour color) const;

    wxCoord m_buttonWidth{ 0 };
    wxCoord m_buttonHeight{ 0 };
    wxCoord m_buttonsStart{ 0 };
    wxCoord m_fileColumnHeight{ 0 };
    wxCoord m_mruButtonHeight{ 0 };
    wxWindowID m_activeButton{ wxNOT_FOUND };
    wxStartPageStyle m_style{ wxStartPageStyle::wxStartPageFlat };
    wxFont m_logoFont;
    std::vector<wxStartPageButton> m_fileButtons;
    std::vector<wxStartPageButton> m_buttons;
    wxBitmapBundle m_logo;
    wxString m_toolTip;
    wxString m_productDescription;
    wxColour m_backstageBackgroundColor{ 145, 168, 208 };
    wxColour m_backstageFontColor{ *wxWHITE };
    wxColour m_detailBackgroundColor{ *wxWHITE };
    wxColour m_detailFontColor{ *wxBLACK };
    wxColour m_hoverColor{ 100, 140, 250 };
    wxColour m_hoverFontColor{ *wxBLACK  };
    };

/** @}*/

#endif //__WXSTART_PAGE_H__
