/********************************************/
/*	tool_lib.cpp: construction des toolbars */
/********************************************/

#include "fctsys.h"
#include "gr_basic.h"

#include "common.h"
#include "program.h"
#include "libcmp.h"
#include "general.h"

#include "protos.h"
#include "hotkeys.h"

#define BITMAP wxBitmap

#ifdef __UNIX__
#define LISTBOX_WIDTH 140
#else
#define LISTBOX_WIDTH 120
#endif

// ----------------------------------------------------------------------------
// resources
// ----------------------------------------------------------------------------

// USE_XPM_BITMAPS
#include "bitmaps.h"

#include "id.h"


/****************************************************/
void WinEDA_LibeditFrame::ReCreateVToolbar()
/****************************************************/
{
    if( m_VToolBar == NULL )
    {
        m_VToolBar = new WinEDA_Toolbar( TOOLBAR_TOOL, this, ID_V_TOOLBAR, FALSE );

        // Set up toolbar
        m_VToolBar->AddTool( ID_NO_SELECT_BUTT, wxEmptyString,
                             BITMAP( cursor_xpm ),
                             _( "deselect current tool" ), wxITEM_CHECK );
        m_VToolBar->ToggleTool( ID_NO_SELECT_BUTT, TRUE );

        m_VToolBar->AddSeparator();
        m_VToolBar->AddTool( ID_LIBEDIT_PIN_BUTT, wxEmptyString,
                             BITMAP( pin_xpm ),
                             _( "Add Pins" ), wxITEM_CHECK  );

        m_VToolBar->AddTool( ID_LIBEDIT_BODY_TEXT_BUTT, wxEmptyString,
                             BITMAP( add_text_xpm ),
                             _( "Add graphic text" ), wxITEM_CHECK  );

        m_VToolBar->AddTool( ID_LIBEDIT_BODY_RECT_BUTT, wxEmptyString,
                             BITMAP( add_rectangle_xpm ),
                             _( "Add rectangles" ), wxITEM_CHECK );

        m_VToolBar->AddTool( ID_LIBEDIT_BODY_CIRCLE_BUTT, wxEmptyString,
                             BITMAP( add_circle_xpm ),
                             _( "Add circles" ), wxITEM_CHECK  );

        m_VToolBar->AddTool( ID_LIBEDIT_BODY_ARC_BUTT, wxEmptyString,
                             BITMAP( add_arc_xpm ),
                             _( "Add arcs" ), wxITEM_CHECK  );

        m_VToolBar->AddTool( ID_LIBEDIT_BODY_LINE_BUTT, wxEmptyString,
                             BITMAP( add_polygon_xpm ),
                             _( "Add lines and polygons" ), wxITEM_CHECK  );

        m_VToolBar->AddSeparator();
        m_VToolBar->AddTool( ID_LIBEDIT_ANCHOR_ITEM_BUTT, wxEmptyString,
                             BITMAP( anchor_xpm ),
                             _( "Move part anchor" ), wxITEM_CHECK  );

        m_VToolBar->AddSeparator();
        m_VToolBar->AddTool( ID_LIBEDIT_IMPORT_BODY_BUTT, wxEmptyString,
                             BITMAP( import_xpm ),
                             _( "Import existing drawings" ), wxITEM_CHECK  );

        m_VToolBar->AddTool( ID_LIBEDIT_EXPORT_BODY_BUTT, wxEmptyString,
                             BITMAP( export_xpm ),
                             _( "Export current drawing" ), wxITEM_CHECK  );

        m_VToolBar->AddSeparator();
        m_VToolBar->AddTool( ID_LIBEDIT_DELETE_ITEM_BUTT, wxEmptyString,
                             BITMAP( delete_body_xpm ),
                             _( "Delete items" ), wxITEM_CHECK  );

        m_VToolBar->Realize();
    }

    SetToolbars();
}


/*************************************************/
void WinEDA_LibeditFrame::ReCreateHToolbar()
/*************************************************/

/* Create or update the main Horizontal Toolbar for the schematic library editor
 */
{
    int      ii;
    wxString msg;

    // Create the toolbar if not exists
    if( m_HToolBar == NULL )
    {
        m_HToolBar = new WinEDA_Toolbar( TOOLBAR_MAIN, this, ID_H_TOOLBAR, TRUE );
        SetToolBar( m_HToolBar );

        // Set up toolbar
        m_HToolBar->AddTool( ID_LIBEDIT_SAVE_CURRENT_LIB, wxEmptyString, BITMAP( save_library_xpm ),
                            _( "Save current loaded library on disk (file update)" ) );

        m_HToolBar->AddTool( ID_LIBEDIT_SELECT_CURRENT_LIB, wxEmptyString, BITMAP( library_xpm ),
                            _( "Select working library" ) );

        m_HToolBar->AddTool( ID_LIBEDIT_DELETE_PART, wxEmptyString, BITMAP( delete_xpm ),
                            _( "Delete component in current library" ) );

        m_HToolBar->AddSeparator();
        m_HToolBar->AddTool( ID_LIBEDIT_NEW_PART, wxEmptyString, BITMAP( new_component_xpm ),
                            _( "New component" ) );

        m_HToolBar->AddTool( ID_LIBEDIT_SELECT_PART, BITMAP( add_component_xpm ),
                            _( "Select component to edit" ) );

        m_HToolBar->AddTool( ID_LIBEDIT_SAVE_CURRENT_PART, wxEmptyString,
                            BITMAP( save_part_in_mem_xpm ),
                            _( "Save current component into current loaded library (in memory)" ) );

        m_HToolBar->AddTool( ID_LIBEDIT_IMPORT_PART, wxEmptyString, BITMAP( import_xpm ),
                            _( "import component" ) );

        m_HToolBar->AddTool( ID_LIBEDIT_EXPORT_PART, wxEmptyString, BITMAP( export_xpm ),
                            _( "export component" ) );

        m_HToolBar->AddTool( ID_LIBEDIT_CREATE_NEW_LIB_AND_SAVE_CURRENT_PART,
                            wxEmptyString, BITMAP( new_library_xpm ),
                            _( "Create a new library an save current component into" ) );

        m_HToolBar->AddSeparator();
		msg = AddHotkeyName( _( "Undo last edition" ), s_Schematic_Hokeys_Descr, HK_UNDO );
        m_HToolBar->AddTool( ID_LIBEDIT_UNDO, wxEmptyString, BITMAP( undo_xpm ), msg );
		msg = AddHotkeyName( _( "Redo the last undo command" ), s_Schematic_Hokeys_Descr, HK_REDO );
        m_HToolBar->AddTool( ID_LIBEDIT_REDO, wxEmptyString, BITMAP( redo_xpm ), msg );

        m_HToolBar->AddSeparator();
        m_HToolBar->AddTool( ID_LIBEDIT_GET_FRAME_EDIT_PART, BITMAP( part_properties_xpm ),
                            wxNullBitmap,
                            FALSE,
                            -1, -1, (wxObject*) NULL,
                            _( "Edit component properties" ) );

        m_HToolBar->AddSeparator();
        m_HToolBar->AddTool( ID_LIBEDIT_CHECK_PART, BITMAP( erc_xpm ),
                            wxNullBitmap,
                            FALSE,
                            -1, -1, (wxObject*) NULL,
                            _( "Test duplicate pins" ) );

        m_HToolBar->AddSeparator();
        msg = AddHotkeyName( _( "Zoom in" ), s_Libedit_Hokeys_Descr, HK_ZOOM_IN );
        m_HToolBar->AddTool( ID_ZOOM_IN_BUTT, wxEmptyString, BITMAP( zoom_in_xpm ),
                             msg );

        msg = AddHotkeyName( _( "Zoom out" ), s_Libedit_Hokeys_Descr, HK_ZOOM_OUT );
        m_HToolBar->AddTool( ID_ZOOM_OUT_BUTT, wxEmptyString, BITMAP( zoom_out_xpm ),
                             msg );

        msg = AddHotkeyName( _( "Redraw view" ), s_Libedit_Hokeys_Descr, HK_ZOOM_REDRAW );
        m_HToolBar->AddTool( ID_ZOOM_REDRAW_BUTT, wxEmptyString, BITMAP( zoom_redraw_xpm ),
                             msg );

        m_HToolBar->AddTool( ID_ZOOM_PAGE_BUTT, wxEmptyString,
                            BITMAP( zoom_auto_xpm ),
                            _( "Zoom auto" ) );

        m_HToolBar->AddSeparator();
        m_HToolBar->AddTool( ID_DE_MORGAN_NORMAL_BUTT, wxEmptyString,
                             BITMAP( morgan1_xpm ),
                             _( "show as \"De Morgan\" normal part" ), wxITEM_CHECK );
        m_HToolBar->ToggleTool( ID_DE_MORGAN_NORMAL_BUTT,
                                (CurrentConvert <= 1) ? TRUE : FALSE );

        m_HToolBar->AddTool( ID_DE_MORGAN_CONVERT_BUTT, wxEmptyString,
                             BITMAP( morgan2_xpm ),
                             _( "show as \"De Morgan\" convert part" ), wxITEM_CHECK );
        m_HToolBar->ToggleTool( ID_DE_MORGAN_CONVERT_BUTT,
                                (CurrentConvert >= 2) ? TRUE : FALSE );

        m_HToolBar->AddSeparator();
        m_HToolBar->AddTool( ID_LIBEDIT_VIEW_DOC, BITMAP( datasheet_xpm ),
                            wxNullBitmap,
                            FALSE,
                            -1, -1, (wxObject*) NULL,
                            _( "Documents" ) );
        m_HToolBar->EnableTool( ID_LIBEDIT_VIEW_DOC, FALSE );

        m_HToolBar->AddSeparator();
        m_SelpartBox = new WinEDAChoiceBox( m_HToolBar, ID_LIBEDIT_SELECT_PART_NUMBER,
                                           wxDefaultPosition, wxSize( LISTBOX_WIDTH, -1 ) );
        m_HToolBar->AddControl( m_SelpartBox );

        m_SelAliasBox = new WinEDAChoiceBox( m_HToolBar, ID_LIBEDIT_SELECT_ALIAS,
                                            wxDefaultPosition, wxSize( LISTBOX_WIDTH, -1 ) );
        m_HToolBar->AddControl( m_SelAliasBox );

        m_HToolBar->AddSeparator();
        m_HToolBar->AddTool( ID_LIBEDIT_EDIT_PIN_BY_PIN, BITMAP( pin2pin_xpm ),
                            wxNullBitmap,
                            TRUE,
                            -1, -1, (wxObject*) NULL,
                            _( "Edit pins part per part (Carefully use!)" ) );
        m_HToolBar->ToggleTool( ID_LIBEDIT_EDIT_PIN_BY_PIN, g_EditPinByPinIsOn );

        // after adding the buttons to the toolbar, must call Realize() to reflect the changes
        m_HToolBar->Realize();
    }
    else    /* Toolbar already created, it only must be updated */
    {
        m_SelAliasBox->Clear();
        m_SelpartBox->Clear();
    }

    /* Update the part selection box */
    int jj = 1;
    if( CurrentLibEntry )
        jj = CurrentLibEntry->m_UnitCount;
    if( jj > 1 )
        for( ii = 0; ii < jj; ii++ )
        {
            wxString msg;
            msg.Printf( _( "Part %c" ), 'A' + ii );
            m_SelpartBox->Append( msg );
        }

    else
        m_SelpartBox->Append( wxEmptyString );
    m_SelpartBox->SetSelection( ( CurrentUnit > 0 ) ? CurrentUnit - 1 : 0 );

    if( CurrentLibEntry )
    {
        if( CurrentLibEntry->m_UnitCount > 1 )
            m_SelpartBox->Enable( TRUE );
        else
            m_SelpartBox->Enable( FALSE );
        m_SelAliasBox->Append( CurrentLibEntry->m_Name.m_Text );
        m_SelAliasBox->SetSelection( 0 );
        int count = CurrentLibEntry->m_AliasList.GetCount();
        if( count > 0 ) /* Update the part selection box */
        {
            m_SelAliasBox->Enable( TRUE );
            for( ii = 0, jj = 1; ii < count; ii += ALIAS_NEXT, jj++ )
            {
                m_SelAliasBox->Append( CurrentLibEntry->m_AliasList[ii] );
                if( CurrentAliasName == CurrentLibEntry->m_AliasList[ii] )
                    m_SelAliasBox->SetSelection( jj );
            }
        }
        else
            m_SelAliasBox->Enable( FALSE );
    }
    else
    {
        m_SelAliasBox->Enable( FALSE );
        m_SelpartBox->Enable( FALSE );
    }

    // Must be called AFTER Realize():
    SetToolbars();
}
