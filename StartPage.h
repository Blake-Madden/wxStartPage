/** @date 2015-2021
    @copyright Oleander Software, Ltd.
    @author Blake Madden
    @details This program is free software; you can redistribute it and/or modify
    it under the terms of the BSD License.
* @{*/

#ifndef __WXSTART_PAGE_H__
#define __WXSTART_PAGE_H__

#include <wx/wx.h>
#include <wx/window.h>
#include <wx/filename.h>
#include <wx/dcbuffer.h>
#include <wx/dcgraph.h>
#include <wx/artprov.h>
#include <vector>

DECLARE_EVENT_TYPE(wxEVT_STARTPAGE_CLICKED, -1)

#define EVT_STARTPAGE_CLICKED(winid, fn) \
    wx__DECLARE_EVT1(wxEVT_STARTPAGE_CLICKED, winid, wxCommandEventHandler(fn))

/** @brief A wxWidgets landing page for an application.

    It displays an MRU list on the right side and a list of customizable list of buttons on the left.

    The events of a user clicking on a file or button can be handled via
    an `EVT_STARTPAGE_CLICKED` message map or bound to `wxEVT_STARTPAGE_CLICKED`.
    (This should be bound to a function accepting a wxCommandEvent object.)
    Calling the wxCommandEvent's GetInt() method in your handler will return the ID
    of the button that was clicked. This ID can be checked against:
    - Calling wxStartPage::IsFileId() to see if a file button was clicked.
      If true, then you can get the selected file path from the event's string value.
    - GetButtonID(), if a feature button on the left was clicked. If you have 5 buttons,
      then you can call GetButtonID(0)-GetButtonID(4) and compare the event's ID against the
      returned IDs to see which button was clicked.
    - or it will be wxStartPage::wxSTART_PAGE_FILE_LIST_CLEAR if the "Clear all..." button was clicked.
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
        @param mruFiles A list of file paths to show in the most-recently-used file list.
        @param logo A logo image of the program to show on the left side.
        @param fileImage Icon to use for the files in the MRU list. This would usually be the application's logo.
        @param productDescription An option description of the application to show on the left side (under the application's name).*/
    explicit wxStartPage(wxWindow* parent, wxWindowID id = wxID_ANY, const wxArrayString& mruFiles = wxArrayString(),
        const wxBitmap& logo = wxNullBitmap, const wxBitmap& fileImage = wxNullBitmap,
        const wxString productDescription = wxEmptyString);
    wxStartPage() = delete;
    wxStartPage(const wxStartPage&) = delete;
    wxStartPage(wxStartPage&&) = delete;
    wxStartPage& operator=(const wxStartPage&) = delete;
    wxStartPage& operator=(wxStartPage&&) = delete;
    /// Adds a feature button on the left side.
    /// A feature button can be something like "Read the Help" or "Create a New Project."
    /// @param bmp The image for the button.
    /// @param label The label on the button.
    /// @sa Realise().
    void AddButton(const wxBitmap& bmp, const wxString& label)
        { m_buttons.push_back(wxStartPageButton(bmp, label)); }
    /// @returns The ID of the given index into the custom button list, or returns wxNOT_FOUND if an invalid index is given.
    /// @param buttonIndex The index into the custom button list.
    [[nodiscard]] const int GetButtonID(const size_t buttonIndex) const
        { return buttonIndex > m_buttons.size() ? wxNOT_FOUND : m_buttons[buttonIndex].m_id; }
    /// Sets the list of files to be shown in the "most-recently-used" list.
    /// @param mruFiles The list of file names.
    /// @sa Realise().
    /// @note Files that can't be found will be filtered out, although they will remain in the parent application's MRU list.
    /// That way, if a user is disconnected from their network, then any network files won't appear, but may appear next time
    /// if they are then connected to the network.
    void SetMRUList(const wxArrayString& mruFiles);
    /// @returns The number of items in the MRU list.
    /// @note This is the number of files in the list, not including the "clear all" button.
    [[nodiscard]] const size_t GetMRUFileCount() const noexcept
        {
        // the last item is the "clear list" button, so don't count that
        return m_fileButtons.size() > 0 ? m_fileButtons.size()-1 : 0;
        }
    /// @returns True if @c Id is an ID within the MRU list.
    /// @param Id The ID from an EVT_STARTPAGE_CLICKED event after a user clicks a button on the start page.
    [[nodiscard]] static constexpr bool IsFileId(const int Id) noexcept
        { return (Id >= ID_FILE_ID_START && Id < wxSTART_PAGE_FILE_LIST_CLEAR); }
    /// Recalculate the sizes of the controls and their layout.
    /// @note This should be called after adding the feature buttons and MRU list.
    void Realise();
    /// Sets the appearance of the start page.
    /// @param style The style for the start page.
    void SetStyle(const wxStartPageStyle style) noexcept
        { m_style = style; }
    /// @returns The color of the left side of the start page.
    [[nodiscard]] wxColour GetBackstageBackgroundColor() const noexcept
        { return m_backstageBackgroundColor; }
    /// Sets the color of the left side of the start page.
    /// @param color The color to use.
    void SetBackstageBackgroundColor(const wxColour& color) noexcept
        { m_backstageBackgroundColor = color; }
    /// @returns The font color of the left side of the start page.
    [[nodiscard]] wxColour GetBackstageFontColor() const noexcept
        { return m_backstageFontColor; }
    /// Sets the font color of the left side of the start page.
    /// @param color The color to use.
    void SetBackstageFontColor(const wxColour& color) noexcept
        { m_backstageFontColor = color; }
    /// @returns The color of the right side of the start page.
    [[nodiscard]] wxColour GetDetailBackgroundColor() const noexcept
        { return m_detailBackgroundColor; }
    /// Sets the color of the right side of the start page.
    /// @param color The color to use.
    void SetDetailBackgroundColor(const wxColour& color) noexcept
        { m_detailBackgroundColor = color; }
    /// @returns The font color of the right side of the start page.
    [[nodiscard]] wxColour GetDetailFontColor() const noexcept
        { return m_detailFontColor; }
    /// Sets the font color of the right side of the start page.
    /// @param color The color to use.
    void SetDetailFontColor(const wxColour& color) noexcept
        { m_detailFontColor = color; }
    /// @returns The color of buttons when they are hovered over.
    [[nodiscard]] wxColour GetHoverColor() const noexcept
        { return m_hoverColor; }
    /// Sets the color of buttons when they are hovered over.
    /// @param color The color to use.
    void SetHoverColor(const wxColour& color) noexcept
        { m_hoverColor = color; }
    /// @returns The font color of buttons when they are hovered over.
    [[nodiscard]] wxColour GetHoverFontColor() const noexcept
        { return m_hoverFontColor; }
    /// Sets the font color of buttons when they are hovered over.
    /// @param color The color to use.
    void SetHoverFontColor(const wxColour& color) noexcept
        { m_hoverFontColor = color; }
private:
    struct wxStartPageButton
        {
        wxStartPageButton(const wxBitmap& icon, const wxString& label) : m_icon(icon), m_label(label), m_id(-1)
            {}
        wxStartPageButton(): m_id(-1) {}
        bool IsOk() const
            { return m_label.length() > 0; }
        wxRect m_rect;
        wxBitmap m_icon;
        wxString m_label;
        int m_id{ -1 };
        };
    void OnPaintWindow([[maybe_unused]] wxPaintEvent& event);
    void OnMouseChange(wxMouseEvent& event);
    void OnMouseClick(wxMouseEvent& event);
    // supports 50 file buttons (realistically, there would be ~15)
    static constexpr int ID_FILE_ID_START = wxID_HIGHEST+1;
    static constexpr int ID_BUTTON_ID_START = wxID_HIGHEST+51;
public:
    /// ID returned when the "Clear file list" button is clicked.
    /// Client code can check for this in their `wxEVT_STARTPAGE_CLICKED`
    /// code and clear the application's file history.
    static constexpr int wxSTART_PAGE_FILE_LIST_CLEAR = wxID_HIGHEST+50;
private:
    /// @returns The number of items in the MRU list (including the "clear list" button).
    [[nodiscard]] const size_t GetMRUFileAndClearButtonCount() const noexcept
        { return m_fileButtons.size(); }
    /// @returns The padding height around the labels.
    [[nodiscard]] const wxCoord GetLabelPaddingHeight() const
        { return FromDIP(wxSize(wxSizerFlags::GetDefaultBorder(), wxSizerFlags::GetDefaultBorder())).GetHeight(); }
    /// @returns The padding width around the labels.
    [[nodiscard]] const wxCoord GetLabelPaddingWidth() const
        { return FromDIP(wxSize(wxSizerFlags::GetDefaultBorder(), wxSizerFlags::GetDefaultBorder())).GetWidth(); }
    /// @returns The padding at the top of the control.
    [[nodiscard]] const wxCoord GetTopBorder() const
        { return FromDIP(wxSize(wxSizerFlags::GetDefaultBorder()*4, wxSizerFlags::GetDefaultBorder()*4)).GetHeight(); }
    /// @returns The left border around the icons/labels.
    [[nodiscard]] const wxCoord GetLeftBorder() const
        { return FromDIP(wxSize(wxSizerFlags::GetDefaultBorder()*4, wxSizerFlags::GetDefaultBorder()*4)).GetWidth(); }
    /// @returns Size of an icon scaled to 16x16, with label padding above and below it
    [[nodiscard]] const wxCoord GetMruButtonHeight() const noexcept
        { return m_mruButtonHeight; }
    void DrawHighlight(wxDC& dc, const wxRect rect, const wxColour color) const;
    wxCoord m_buttonWidth{ 0 };
    wxCoord m_buttonHeight{ 0 };
    wxCoord m_buttonsStart{ 0 };
    wxCoord m_fileColumnHeight{ 0 };
    wxCoord m_mruButtonHeight{ 0 };
    wxFont m_logoFont;
    wxFont m_buttonFont;
    int m_activeButton{ -1 };
    std::vector<wxStartPageButton> m_fileButtons;
    std::vector<wxStartPageButton> m_buttons;
    wxBitmap m_logo;
    wxBitmap m_fileImage;
    wxString m_toolTip;
    wxString m_productDescription;
    wxStartPageStyle m_style{ wxStartPageStyle::wxStartPageFlat };
    wxColour m_backstageBackgroundColor{ 145, 168, 208 };
    wxColour m_backstageFontColor{ *wxWHITE };
    wxColour m_detailBackgroundColor{ *wxWHITE };
    wxColour m_detailFontColor{ *wxBLACK };
    wxColour m_hoverColor{ 100, 140, 250 };
    wxColour m_hoverFontColor{ *wxBLACK  };
    };

/** @}*/

#endif //__WXSTART_PAGE_H__
