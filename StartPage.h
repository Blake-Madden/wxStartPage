/** @date 2005-2021
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
#include <wx/vector.h>
#include <wx/artprov.h>

DECLARE_EVENT_TYPE(wxEVT_STARTPAGE_CLICKED, -1)

#define EVT_STARTPAGE_CLICKED(winid, fn) \
    wx__DECLARE_EVT1(wxEVT_STARTPAGE_CLICKED, winid, wxCommandEventHandler(fn))

/** @brief A landing page for an application.

    It displays an MRU list on the right side and a list of customizable list of buttons on the left.

    The events of a user clicking on a file or button can be handled via
    an `EVT_STARTPAGE_CLICKED` message map or bound to `wxEVT_STARTPAGE_CLICKED`.
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
        @param productDescription A name or description of the application to show on the left side.*/
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
    void AddButton(const wxBitmap& bmp, const wxString& label)
        { m_buttons.push_back(wxStartPageButton(bmp, label)); }
    /// @returns The ID of the given index into the custom button list, or returns wxNOT_FOUND if an invalid index is given.
    /// @param buttonIndex The index into the custom button list.
    [[nodiscard]] const int GetButtonID(const size_t buttonIndex) const
        { return buttonIndex > m_buttons.size() ? wxNOT_FOUND : m_buttons[buttonIndex].m_id; }
    /// Sets the list of files to be shown in the "most-recently-used" list.
    /// @param mruFiles The list of file names.
    void SetMRUList(const wxArrayString& mruFiles);
    /// @returns The number of items in the MRU list.
    /// @note This is the number of files in the list, not including the "clear all" button.
    [[nodiscard]] const size_t GetMRUFileCount() const
        {
        // the last item is the "clear list" button, so don't count that
        return m_fileButtons.size() > 0 ? m_fileButtons.size()-1 : 0;
        }
    /// @returns The ID of the given index into the MRU file list, or returns wxNOT_FOUND if an invalid index is given.
    /// @param buttonIndex The index into the list of files.
    [[nodiscard]] const int GetMRUButtonID(const size_t buttonIndex) const
        { return buttonIndex > GetMRUFileCount() ? wxNOT_FOUND : m_fileButtons[buttonIndex].m_id; }
    /// Recalculate the sizes of the controls and their layout.
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
    void OnPaint([[maybe_unused]] wxPaintEvent& event);
    void OnMouseChange(wxMouseEvent& event);
    void OnMouseClick(wxMouseEvent& event);
    // supports 100 buttons (usually, you would have 5 or so)
    static constexpr int ID_FILE_ID_START = wxID_HIGHEST+1;
    static constexpr int ID_BUTTON_ID_START = wxID_HIGHEST+101;
public:
    /// ID returned when the "Clear file list" button is clicked.
    /// Client code can check for this in their `wxEVT_STARTPAGE_CLICKED`
    /// code and clear the application's file history.
    static constexpr int wxSTART_PAGE_FILE_LIST_CLEAR = ID_BUTTON_ID_START+1;
private:
    /// @returns The number of items in the MRU list (including the "clear list" button).
    [[nodiscard]] const size_t GetMRUFileAndClearButtonCount() const
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
    wxVector<wxStartPageButton> m_fileButtons;
    wxVector<wxStartPageButton> m_buttons;
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
