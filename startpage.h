/** @addtogroup Controls
    @date 2015-2023
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
#include <wx/settings.h>
#include <wx/stdpaths.h>
#include <vector>
#include <algorithm>
#include <cassert>

#ifndef wxNODISCARD
    #define wxNODISCARD
#endif

wxDECLARE_EVENT(wxEVT_STARTPAGE_CLICKED, wxCommandEvent);

#define EVT_STARTPAGE_CLICKED(winid, fn) \
    wx__DECLARE_EVT1(wxEVT_STARTPAGE_CLICKED, winid, wxCommandEventHandler(fn))

/// @brief The appearance of the buttons on the start page.
enum class wxStartPageStyle
    {
    wxStartPageFlat, /*!<Flat button appearance. (This is the default.)*/
    wxStartPage3D    /*!<3D button appearance.*/
    };
/// @brief Which type of greeting to show in the start page banner.
enum class wxStartPageGreetingStyle
    {
    wxDynamicGreeting,             /*!<Greeting based on the time of day (e.g., "Good morning").
                                   (This is the default.)*/
    wxDynamicGreetingWithUserName, /* Same as wxDynamicGreeting, but shows the user name also.*/
    wxCustomGreeting,              /*!<User-defined greeting.*/
    wxNoGreeting                   /*!<No greeting.*/
    };
/// @brief How to display in the application header (above the custom buttons)
enum class wxStartPageAppHeaderStyle
    {
    wxStartPageAppNameAndLogo, /*!<The application name & logo. (This is the default.)*/
    wxStartPageAppName,        /*!<The application name.*/
    wxStartPageNoHeader        /*!<No application header.*/
    };

/** @brief A wxWidgets landing page for an application.

    It displays an MRU list on the right side (up to 9 files) and a list of
    customizable buttons on the left.

    Modified dates are shown next to each file in the MRU list. These dates
    are shown in a human readable format
    (e.g., "Just now", "12 minutes ago", "Yesterday", "Tues at 1:07 PM").
    Times are included for dates within the current week;
    otherwise, only the date is shown. The year is only shown if the date
    is from a previous year.

    The application name & logo can be shown above the custom buttons.
    A greeting is also shown above the MRU list (which is customizable).

    The events of a user clicking on a file or button can be handled via
    an @c EVT_STARTPAGE_CLICKED message map or bound to @c wxEVT_STARTPAGE_CLICKED.
    (This should be bound to a function accepting a @c wxCommandEvent object.)
    Calling the `wxCommandEvent`'s @c GetId() method in your handler will return the
    ID of the button that was clicked. This ID can be checked by:
    - Calling IsCustomButtonId() to see if a custom button was clicked.\n
      If so, then compare the event's ID against the button IDs that were
      returned from AddButton().\n
      (GetButtonID() can also be called in your @c wxEVT_STARTPAGE_CLICKED handler
      to use the ID from buttons based on their index.)
    - Calling IsFileId() to see if a file button was clicked.\n
      If @c true, then you can get the selected file path from the event's
      string value.
    - Calling IsFileListClearId() to see if the "Clear file list" button was clicked.
*/
class wxStartPage final : public wxWindow
    {
public:
    /** @brief Constructor.
        @param parent The parent window.
        @param id The start page's ID.
        @param mruFiles A list of file paths to show in the
            most-recently-used file list.
        @param logo A logo image of the program to show on the left side,
            as well as the files in the MRU list.
        @param productDescription An optional description of the application to
            show on the left side (under the application's name).
        @sa SetAppHeaderStyle().*/
    explicit wxStartPage(wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxArrayString& mruFiles = wxArrayString{},
        const wxBitmapBundle& logo = wxBitmapBundle{},
        const wxString& productDescription = wxString{});
    /// @private
    wxStartPage() = delete;
    /// @private
    wxStartPage(const wxStartPage&) = delete;
    /// @private
    wxStartPage& operator=(const wxStartPage&) = delete;

    /// @brief Sets the list of files to be shown in the
    ///     "most-recently-used" list on the right side.
    /// @param mruFiles The list of file names.
    /// @note Files that can't be found will be filtered out,
    ///     although they will remain in the parent application's MRU list.\n
    ///     That way, if a user is disconnected from their network,
    ///     then any network files won't appear, but may appear next time
    ///     if they are then connected to the network.
    void SetMRUList(const wxArrayString& mruFiles);

    /// @name Button Functions
    /// @brief Functions for adding buttons and handling their events.
    /// @{

    /// @brief Adds a feature button on the left side.
    /// @details A feature button can be something like
    ///     "Read the Help" or "Create a New Project."
    /// @param bmp The image for the button.
    /// @param label The label on the button.
    /// @returns The ID assigned to the button. This should be used in your
    ///     @c wxEVT_STARTPAGE_CLICKED handler.
    /// @sa GetButtonID().
    wxWindowID AddButton(const wxBitmapBundle& bmp, const wxString& label)
        {
        m_buttons.push_back(wxStartPageButton(bmp, label));
        return ID_BUTTON_ID_START + (m_buttons.size() - 1);
        }
    /// @brief Adds a feature button on the left side.
    /// @details A feature button can be something like
    ///     "Read the Help" or "Create a New Project."
    /// @param artId The stock art ID to use for the button's icon.
    /// @param label The label on the button.
    /// @returns The ID assigned to the button. This should be used in your
    ///     @c wxEVT_STARTPAGE_CLICKED handler.
    /// @sa GetButtonID().
    wxWindowID AddButton(const wxArtID& artId, const wxString& label)
        {
        wxVector<wxBitmap> bmps;
        bmps.push_back(
            wxArtProvider::GetBitmap(artId, wxART_BUTTON, FromDIP(wxSize(16, 16))).
            ConvertToImage());
        bmps.push_back(
            wxArtProvider::GetBitmap(artId, wxART_BUTTON, FromDIP(wxSize(32, 32))).
            ConvertToImage());
        bmps.push_back(
            wxArtProvider::GetBitmap(artId, wxART_BUTTON, FromDIP(wxSize(64, 64))).
            ConvertToImage());
        bmps.push_back(
            wxArtProvider::GetBitmap(artId, wxART_BUTTON, FromDIP(wxSize(128, 128))).
            ConvertToImage());
        m_buttons.push_back(wxStartPageButton(
            wxBitmapBundle::FromBitmaps(bmps), label));
        return ID_BUTTON_ID_START + (m_buttons.size() - 1);
        }
    /// @returns The ID of the given index into the custom button list,
    ///     or @c wxNOT_FOUND if an invalid index is given.
    /// @param buttonIndex The index into the custom button list.
    wxNODISCARD wxWindowID GetButtonID(const size_t buttonIndex) const noexcept
        {
        return buttonIndex > m_buttons.size() ?
            wxNOT_FOUND :
            m_buttons[buttonIndex].m_id;
        }
    /// @returns @c true if @c Id is and ID for one of the custom buttons on the left.
    /// @param Id The ID from an @c wxEVT_STARTPAGE_CLICKED event after a
    ///     user clicks a button on the start page.
    wxNODISCARD bool IsCustomButtonId(const wxWindowID Id) const noexcept
        {
        return (Id >= ID_BUTTON_ID_START &&
                static_cast<size_t>(Id) < ID_BUTTON_ID_START + m_buttons.size());
        }
    /// @returns @c true if @c Id is an ID within the MRU list.
    /// @param Id The ID from an @c wxEVT_STARTPAGE_CLICKED event after a
    ///     user clicks a button on the start page.
    wxNODISCARD constexpr bool IsFileId(const wxWindowID Id) const noexcept
        { return (Id >= ID_FILE_ID_START && Id < START_PAGE_FILE_LIST_CLEAR); }
    /// @returns @c true if @c Id is the "Clear file list" button.
    /// @param Id The ID from an @c wxEVT_STARTPAGE_CLICKED event after a
    ///     user clicks a button on the start page.
    wxNODISCARD constexpr bool IsFileListClearId(const wxWindowID Id) const noexcept
        { return (Id == START_PAGE_FILE_LIST_CLEAR); }
    /// @}

    /// @name Style Functions
    /// @brief Functions for customizing the start page's visual appearance.
    /// @{

    /// @brief Sets the appearance of the start page.
    /// @param style The style for the start page.
    void SetStyle(const wxStartPageStyle style) noexcept
        { m_style = style; }
    /// @brief Sets which type of greeting (if any) to display.
    /// @param style The greeting style for the start page.
    /// @sa SetCustomGreeting().
    void SetGreetingStyle(const wxStartPageGreetingStyle style) noexcept
        { m_greetingStyle = style; }
    /// @brief Sets a custom greeting to display.
    /// @param greeting The custom greeting to use.
    void SetCustomGreeting(wxString greeting)
        {
        m_customGreeting = std::move(greeting);
        m_greetingStyle = wxStartPageGreetingStyle::wxCustomGreeting;
        }
    /// @brief Sets the name to display when style is set to @c wxDynamicGreetingWithUserName.
    /// @param name The user name to use.
    void SetUserName(wxString name)
        { m_userName = std::move(name); }
    /** @brief How to display the application name and icon
            above the custom buttons.
        @param style The style to use.*/
    void SetAppHeaderStyle(const wxStartPageAppHeaderStyle style) noexcept
        { m_appHeaderStyle = style; }
    /// @returns The color of the left side of the start page.
    wxNODISCARD wxColour GetButtonAreaBackgroundColor() const noexcept
        { return m_buttonAreaBackgroundColor; }
    /// @brief Sets the color of the left side of the start page.
    /// @param color The color to use.
    void SetButtonAreaBackgroundColor(const wxColour& color) noexcept
        {
        if (color.IsOk())
            { m_buttonAreaBackgroundColor = color; }
        }
    /// @returns The color of the right side of the start page.
    wxNODISCARD wxColour GetMRUBackgroundColor() const noexcept
        { return m_MRUBackgroundColor; }
    /// @brief Sets the color of the right side of the start page.
    /// @param color The color to use.
    void SetMRUBackgroundColor(const wxColour& color) noexcept
        {
        if (color.IsOk())
            { m_MRUBackgroundColor = color; }
        }
    /// @}
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
        wxString m_fullFilePath;
        wxWindowID m_id{ wxNOT_FOUND };
        };

    enum class ActiveButtonType
        {
        CustomButton,
        FileButton,
        FileActionButton
        };

    /// @returns The number of items in the MRU list.
    /// @note This is the number of files in the list,
    ///     not including the "clear file list" button.
    wxNODISCARD size_t GetMRUFileCount() const noexcept
        {
        // the last item is the "clear file list" button, so don't count that
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
    static constexpr int MAX_FILE_BUTTONS = 9;
    // supports 9 MRU file buttons
    static constexpr int ID_FILE_ID_START = wxID_HIGHEST;
    static constexpr int ID_BUTTON_ID_START =
        wxID_HIGHEST + MAX_FILE_BUTTONS + 1;
    /// @brief ID returned when the "Clear file list" button is clicked.
    /// @details Client code can check for this in their @c wxEVT_STARTPAGE_CLICKED
    ///     handler and clear the application's file history.
    static constexpr int START_PAGE_FILE_LIST_CLEAR =
        wxID_HIGHEST + MAX_FILE_BUTTONS;

    /// @returns The number of items in the MRU list
    ///     (including the "clear file list" button).
    wxNODISCARD size_t GetMRUFileAndClearButtonCount() const noexcept
        { return m_fileButtons.size(); }
    /// @returns The padding height around the labels.
    wxNODISCARD wxCoord GetLabelPaddingHeight() const
        { return wxSizerFlags::GetDefaultBorder(); }
    /// @returns The padding width around the labels.
    wxNODISCARD wxCoord GetLabelPaddingWidth() const
        { return wxSizerFlags::GetDefaultBorder(); }
    /// @returns The padding at the top of the control.
    wxNODISCARD wxCoord GetTopBorder() const
        { return wxSizerFlags::GetDefaultBorder() * 4; }
    /// @returns The left border around the icons/labels.
    wxNODISCARD wxCoord GetLeftBorder() const
        { return wxSizerFlags::GetDefaultBorder() * 4; }
    /// @returns The size for the app logo
    wxNODISCARD wxSize GetAppLogoSize() const
        { return FromDIP(wxSize(64, 64)); }
    /// @returns The button size, which will be smaller if there
    ///     are numerous buttons.
    wxNODISCARD wxSize GetButtonSize() const
        {
        return FromDIP(m_buttons.size() > MAX_BUTTONS_SMALL_SIZE ?
                       wxSize(16, 16) : wxSize(32, 32));
        }
    /// @returns The size of an icon scaled to 16x16,
    ///     with label padding above and below it.
    wxNODISCARD wxCoord GetMRUButtonHeight() const noexcept
        { return m_mruButtonHeight; }
    wxNODISCARD wxString GetClearFileListLabel() const
        { return _(L"\u267B Clear file list..."); }
    wxNODISCARD wxString GetRecentLabel() const
        { return _(L"Recent"); }
    void DrawHighlight(wxDC& dc, const wxRect rect, const wxColour color) const;
    void CalcButtonStart(wxDC& dc);
    void CalcMRUColumnHeaderHeight(wxDC& dc);
    wxNODISCARD wxString FormatGreeting() const;
    /// @brief Determines whether a color is dark.
    /// @details "Dark" is defined as luminance being less than 50% and
    ///     opacity higher than 32. For example, black having an opacity of 32
    ///     would mean it has 1/8 of the opacity of a fully opaque black;
    ///     this would appear more like a very light gray, rather than black, and would
    ///     be considered not dark.
    /// @param color The color to review.
    /// @returns @c true if the color is dark.
    wxNODISCARD static bool IsDark(const wxColour& color)
        {
        assert(color.IsOk() && L"Invalid color passed to IsDark()!");
        return (color.IsOk() &&
                color.Alpha() > 32 &&
                color.GetLuminance() < 0.5);
        }
    /// @brief Returns a darker (shaded) or lighter (tinted) version of a color,
    ///     depending on how dark it is to begin with.
    ///     For example, black will be returned as dark gray,
    ///     while white will return as an eggshell white.
    /// @param color The color to shade.
    /// @param shadeOrTintValue How much to lighten or darken a color
    ///      (should be between @c 0.0 to @c 1.0.)
    /// @returns The shaded or tinted color.
    wxNODISCARD static wxColour ShadeOrTint(const wxColour& color,
                                            const double shadeOrTintValue = 0.2)
        {
        return (IsDark(color) ?
            color.ChangeLightness(100 + static_cast<int>(shadeOrTintValue * 100)) :
            color.ChangeLightness(100 - static_cast<int>(shadeOrTintValue * 100)));
        }
    /// @brief Returns either black or white, depending on which better contrasts
    ///     against the specified color.
    /// @param color The color to contrast against to see if white or black should go on it.
    /// @returns Black or white; whichever contrasts better against @c color.
    wxNODISCARD static wxColour BlackOrWhiteContrast(const wxColour& color)
        { return (IsDark(color) ? *wxWHITE : *wxBLACK); }

    wxCoord m_buttonWidth{ 0 };
    wxCoord m_buttonHeight{ 0 };
    wxCoord m_buttonsStart{ 0 };
    wxCoord m_fileColumnHeaderHeight{ 0 };
    wxCoord m_mruButtonHeight{ 0 };
    wxWindowID m_activeButton{ wxNOT_FOUND };
    wxStartPageStyle m_style{ wxStartPageStyle::wxStartPageFlat };
    wxStartPageGreetingStyle m_greetingStyle
        { wxStartPageGreetingStyle::wxDynamicGreetingWithUserName };
    wxString m_customGreeting;
    wxStartPageAppHeaderStyle m_appHeaderStyle
        { wxStartPageAppHeaderStyle::wxStartPageAppNameAndLogo };
    wxFont m_logoFont;
    std::vector<wxStartPageButton> m_fileButtons;
    std::vector<wxStartPageButton> m_buttons;
    wxBitmapBundle m_logo;
    wxString m_toolTip;
    wxString m_productDescription;
    wxColour m_buttonAreaBackgroundColor{ 145, 168, 208 };
    wxColour m_MRUBackgroundColor{ *wxWHITE };
    wxString m_userName{ wxGetUserName() };
    };

/** @}*/

#endif //__WXSTART_PAGE_H__
