/////////////////////////////////////////////////////////////////////////////
// Name:        startdemo.cpp
// Purpose:     wxStartPage sample
// Author:      Blake Madden
// Modified by:
// Created:     01/08/2023
// Copyright:   (c) Blake Madden
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// ============================================================================
// declarations
// ============================================================================

#include <wx/wx.h>
#include <wx/dir.h>
#include <wx/stdpaths.h>
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
    MyFrame(const wxString& title);

    void OnStartPageClick(wxCommandEvent& event);
private:
    wxStartPage* m_startPage{ nullptr };
};

wxIMPLEMENT_APP(MyApp);

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
    if ( !wxApp::OnInit() )
        return false;

    SetAppName("Start Page Demo");

    // create the main application window
    MyFrame *frame = new MyFrame("Start Page Demo");
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

    wxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    // build and show our start page
    //==============================

    // Get a list of files to show in the MRU list.
    // Normally, you would get this from your application's document history,
    // but for simplicity we will just look for PDF files in the documents folder.
    wxArrayString mruFiles;
    wxDir dir;
    dir.GetAllFiles(wxStandardPaths::Get().GetDocumentsDir(),
                    &mruFiles, "*.pdf", wxDIR_FILES);

    // construct the start page with the file list and app logo
    m_startPage = new wxStartPage(this, wxID_ANY, mruFiles,
       wxArtProvider::GetBitmapBundle(wxART_REPORT_VIEW));
    // add some custom buttons to appear on the left side
    m_startPage->AddButton(
        wxArtProvider::GetBitmapBundle(wxART_REPORT_VIEW), L"About");
    m_startPage->AddButton(
        wxArtProvider::GetBitmapBundle(wxART_FILE_OPEN), L"Open File");
    m_startPage->AddButton(
        wxArtProvider::GetBitmapBundle(wxART_WX_LOGO),
            L"Visit the wxWidgets Website");
    m_startPage->AddButton(
        wxArtProvider::GetBitmapBundle(wxART_QUIT), L"Exit");

    // bind our event handler to the start page's buttons
    Bind(wxEVT_STARTPAGE_CLICKED, &MyFrame::OnStartPageClick, this);

    // finally, add the start page to the frame
    sizer->Add(m_startPage, 1, wxEXPAND);
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
        // the first button was clicked
        if (m_startPage->GetButtonID(0) == event.GetId())
        {
            wxMessageBox(wxString::Format
                 (
                    "Welcome to %s!\n"
                    "\n"
                    "This is the wxStartPage demo\n"
                    "running under %s.",
                    wxVERSION_STRING,
                    wxGetOsDescription()
                 ),
                 "About wxStartPage demo",
                 wxOK | wxICON_INFORMATION,
                 this);
        }
        // the second button was clicked
        else if (m_startPage->GetButtonID(1) == event.GetId())
        {
            wxFileDialog fd(this, _("Open a File"));
            if (fd.ShowModal() == wxID_CANCEL)
                { return; }
            wxMessageBox(wxString::Format
                 (
                    "You selected '%s'.",
                    fd.GetFilename()
                 ),
                 "File Open",
                 wxOK | wxICON_INFORMATION,
                 this);
        }
        else if (m_startPage->GetButtonID(2) == event.GetId())
        {
            wxLaunchDefaultBrowser("https://www.wxwidgets.org");
        }
        else if (m_startPage->GetButtonID(3) == event.GetId())
        {
            Close(true);
        }
    }
    // something in the MRU list was clicked
    else if (m_startPage->IsFileId(event.GetId()))
    {
        // Here, we could open the document in our application if
        // we are using a docview architecture. In this example,
        // just open the file with the default application.
        wxLaunchDefaultApplication(event.GetString());
    }
    // the "Clear file list" button for the MRU list was clicked
    else if (m_startPage->IsFileListClearId(event.GetId()))
    {
        // if we reach this point, then the list requested to clear
        // the MRU menu. This will clear the list on the start page,
        // and here we could also do something like call
        // ClearFileHistoryMenu() if our program is using a
        // docview architecture.
    }
}
