/////////////////////////////////////////////////////////////////////////////
// Name:        startdemo.cpp
// Purpose:     wxStartPage sample
// Author:      Blake Madden
// Modified by:
// Created:     01/08/2023
// Copyright:   (c) Blake Madden
// License:     BSD-3-Clause
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

#include <wx/wx.h>
#include <wx/dir.h>
#include <wx/stdpaths.h>
#include <wx/bmpbndl.h>
#include <wx/uilocale.h>
#include "../startpage.h"

// Define a new application type, each program should derive a class from wxApp
class MyApp : public wxApp
{
public:
    bool OnInit() override;
};

// Define a new frame type: this is going to be our main frame
class MyFrame : public wxFrame
{
public:
    // ctor(s)
    explicit MyFrame(const wxString& title);

    void OnStartPageClick(wxCommandEvent& event);
private:
    wxStartPage* m_startPage{ nullptr };
    wxWindowID m_aboutButtonID{ wxID_ANY };
    wxWindowID m_wxWebsiteButtonID{ wxID_ANY };
    wxWindowID m_fileOpenButtonID{ wxID_ANY };
    wxWindowID m_exitButtonID{ wxID_ANY };
};

// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-pro-type-static-cast-downcast)
wxIMPLEMENT_APP(MyApp);
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables,cppcoreguidelines-pro-type-static-cast-downcast)

// ============================================================================
// implementation
// ============================================================================

// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

// 'Main program' equivalent: the program execution "starts" here
//---------------------------------------------------
bool MyApp::OnInit()
{
    if (!wxApp::OnInit())
    {
        return false;
    }

    wxUILocale::UseDefault();

    SetAppName(L"Start Page Demo");

    // create the main application window
    auto* frame = new MyFrame(_(L"Start Page Demo"));
    frame->Show(true);

    return true;
}

// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

// frame constructor
//---------------------------------------------------
MyFrame::MyFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title)
{
    SetSize(FromDIP(wxSize(900, 700)));

    const wxString appDir{ wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath() };
    SetIcon(
        wxBitmapBundle::FromSVGFile(appDir + L"/res/x-office-document.svg", wxSize(64, 64)).
        GetIcon(wxSize(64, 64)));

    wxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    // build and show our start page
    //==============================

    // Get a list of files to show in the MRU list.
    // Normally, you would get this from your application's document history,
    // but for simplicity we will just look for PDF files in the documents folder.
    wxArrayString mruFiles;
    wxDir::GetAllFiles(wxStandardPaths::Get().GetDocumentsDir(),
        &mruFiles, "*.pdf", wxDIR_FILES);

    // construct the start page with the file list and app logo
    m_startPage = new wxStartPage(this, wxID_ANY, mruFiles,
        wxBitmapBundle::FromSVGFile(appDir + L"/res/x-office-document.svg",
            FromDIP(wxSize(64, 64))));

    // By default, the application name and its logo are shown on the left
    // (above the custom buttons). Uncomment the following to turn this off:

    // m_startPage->SetAppHeaderStyle(wxStartPageAppHeaderStyle::wxStartPageNoHeader);

    // Add some custom buttons to appear on the left side.
    // Note that we capture the IDs of the buttons as we add them;
    // we will use this in our wxEVT_STARTPAGE_CLICKED handler.
    m_fileOpenButtonID = m_startPage->AddButton(wxART_FILE_OPEN, _(L"Open File"));
    m_wxWebsiteButtonID = m_startPage->AddButton(wxART_WX_LOGO,
        _(L"Visit the wxWidgets Website"));
    m_aboutButtonID = m_startPage->AddButton(
        wxBitmapBundle::FromSVGFile(appDir + L"/res/jean_victor_balin_unknown_green.svg",
            FromDIP(wxSize(64, 64))),
        _(L"About"));
    m_exitButtonID = m_startPage->AddButton(wxART_QUIT, _(L"Exit"));

    /* By default, a greeting such as "Good morning" or "Good evening"
       will be displayed. (This is determined by the time of day).
       This can be changed (or turned off) via SetGreetingStyle()
       and SetCustomGreeting(). Uncomment the following to see an example:*/

       // m_startPage->SetCustomGreeting(_(L"Welcome to the Demonstration!"));

       /* The visual effect when the buttons are moused over can also
          be customized. Uncomment the following to use a glassy, 3D look
          for the buttons:*/

          // m_startPage->SetStyle(wxStartPageStyle::wxStartPage3D);

          /* By default, the start page will use either a light blue & white
             theme or a dark theme (depending on the app's settings).
             Uncomment the following to apply a different theme:*/

             // m_startPage->SetButtonAreaBackgroundColor(wxColour("#FF69B4"));
             // m_startPage->SetMRUBackgroundColor(wxColour("#FFB6DA"));

             // bind our event handler to the start page's buttons
    Bind(wxEVT_STARTPAGE_CLICKED, &MyFrame::OnStartPageClick, this);

    // finally, add the start page to the frame
    sizer->Add(m_startPage, wxSizerFlags{ 1 }.Expand());
    SetSizer(sizer);

    Centre();
}

// event handlers
//---------------------------------------------------
void MyFrame::OnStartPageClick(wxCommandEvent& event)
{
    // a custom button (on the left) was clicked
    if (m_startPage->IsCustomButtonId(event.GetId()))
    {
        // the About button was clicked
        if (m_aboutButtonID == event.GetId())
        {
            wxMessageBox(wxString::Format
            (
                _(L"Welcome to wxStartPage demo\n"
                    "running with %s\nunder %s."),
                wxVERSION_STRING,
                wxGetOsDescription()
            ),
                _(L"About wxStartPage demo"),
                wxOK | wxICON_INFORMATION,
                this);
        }
        // the file open button was clicked
        else if (m_fileOpenButtonID == event.GetId())
        {
            wxFileDialog fd(this, _(L"Open a File"));
            if (fd.ShowModal() == wxID_CANCEL)
            {
                return;
            }
            wxMessageBox(wxString::Format
            (
                _(L"You selected '%s'."),
                fd.GetFilename()
            ),
                _(L"File Open"),
                wxOK | wxICON_INFORMATION,
                this);
        }
        else if (m_wxWebsiteButtonID == event.GetId())
        {
            wxLaunchDefaultBrowser("https://www.wxwidgets.org");
        }
        else if (m_exitButtonID == event.GetId())
        {
            Close(true);
        }
    }
    // something in the MRU list was clicked
    else if (wxStartPage::IsFileId(event.GetId()))
    {
        // Here, we could open the document in our application if
        // we are using a docview architecture. In this example,
        // just open the file with the default application.
        wxLaunchDefaultApplication(event.GetString());
    }
    // the "Clear file list" button for the MRU list was clicked
    else if (wxStartPage::IsFileListClearId(event.GetId()))
    {
        // if we reach this point, then the list requested to clear
        // the MRU menu. This will clear the list on the start page,
        // and here we could also do something like call
        // ClearFileHistoryMenu() if our program is using a
        // docview architecture.
    }
}
