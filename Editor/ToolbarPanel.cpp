#include "Precompile.h"

#include "ToolbarPanel.h"
#include "ArtProvider.h"

#include "EditorIDs.h"

#include "Editor/App.h"
#include "Editor/FileDialog.h"

using namespace Helium;
using namespace Helium::SceneGraph;
using namespace Helium::Editor;

ToolbarPanel::ToolbarPanel( wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style )
: ToolbarPanelGenerated( parent, id, pos, size, style )
, m_CommandQueue( this )
, m_ToolPropertiesGenerator( &m_ToolPropertiesCanvas )
, m_ToolPropertiesManager( &m_ToolPropertiesGenerator, &m_CommandQueue )
{
    m_ToolPropertiesCanvas.SetPanel( m_ToolsPropertiesPanel );

    SetHelpText( TXT( "This is the Toolbar, it provides access to commonly used actions and tools." ) );

#pragma TODO( "Remove this block of code if/when wxFormBuilder supports wxArtProvider" )
    {
        Freeze();

        m_PlayButton->SetBitmap( wxArtProvider::GetBitmap( ArtIDs::Play ) );
        m_PauseButton->SetBitmap( wxArtProvider::GetBitmap( ArtIDs::Pause ) );
        m_StopButton->SetBitmap( wxArtProvider::GetBitmap( ArtIDs::Stop ) );

        Layout();
        Thaw();
    }

    m_PlayButton->SetHelpText( TXT( "Play\n\nClicking this will start the game in the editing window." ) );
    m_PauseButton->SetHelpText( TXT( "Pause\n\nClicking this will pause a currently running game session." ) );
    m_StopButton->SetHelpText( TXT( "Stop\n\nClicking this will stop a currently running game session." ) );

    std::map< ToolType, wxPanel* > toolTypePanels;
    for ( i32 i=0; i < ToolModes::Count; ++i )
    {
        ToolInfo* info = &ToolInfos[ i ];

        std::map< ToolType, wxPanel* >::iterator itr = toolTypePanels.find( info->m_Type );

        wxPanel* typePanel = NULL;

        if ( itr == toolTypePanels.end() )
        {
            typePanel = new wxPanel( m_ToolsPanel );
            typePanel->SetSizer( new wxBoxSizer( wxHORIZONTAL ) );
            toolTypePanels[ info->m_Type ] = typePanel;
            m_ToolsPanel->GetSizer()->Add( typePanel, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxEXPAND, 0 );
            //m_ToolsPanel->GetSizer()->Add( new wxStaticLine( m_ToolsPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL ), 0, wxEXPAND | wxALL, 2 );
            m_ToolsPanel->GetSizer()->Add( 20, 0 ); // add spacer to the right of the panel
        }

        if ( !typePanel )
        {
            typePanel = (*itr).second;
        }

        wxBitmapToggleButton* btn = new wxBitmapToggleButton( typePanel, info->m_ID, wxArtProvider::GetBitmap( info->m_Bitmap, wxART_OTHER, ArtProvider::DefaultIconSize ), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW );
        btn->SetHelpText( info->m_Description );

        // connect its event handler to us
        btn->Connect( btn->GetId(), wxEVT_COMMAND_TOGGLEBUTTON_CLICKED, wxCommandEventHandler( ToolbarPanel::OnToggleToolButton ), NULL, this );

        typePanel->GetSizer()->Add( btn, 0, wxALL | wxALIGN_CENTER_VERTICAL, 2 );
        m_ToolsButtons.push_back( btn );
    }

    m_VaultSearchBox->SetHelpText( TXT( "This is the Vault search box.  Entering text here and pressing enter will search the Asset Vault." ) );
    m_ToolsPropertiesPanel->SetHelpText( TXT( "This is the tools properties area.  Depending on the tool you have selected, this will show its options." ) );

    Layout();
}

void ToolbarPanel::ToggleTool( i32 selectedTool )
{
    for ( std::vector< wxBitmapToggleButton* >::const_iterator itr = m_ToolsButtons.begin(), end = m_ToolsButtons.end(); itr != end; ++itr )
    {
        (*itr)->SetValue( (*itr)->GetId() == selectedTool );
    }
}

void ToolbarPanel::OnToggleToolButton( wxCommandEvent& event )
{
    GetParent()->GetEventHandler()->ProcessEvent( wxCommandEvent ( wxEVT_COMMAND_MENU_SELECTED, event.GetId() ) );
}
