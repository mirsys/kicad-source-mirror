/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2010-2016 Jean-Pierre Charras, jean-pierre.charras at wanadoo.fr
 * Copyright (C) 1992-2019 KiCad Developers, see AUTHORS.txt for contributors.
 * Copyright (C) 2018 CERN
 * Author: Maciej Suminski <maciej.suminski@cern.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <kiface_i.h>
#include <confirm.h>
#include <base_units.h>
#include <pcbnew_settings.h>
#include <pcbplot.h>
#include <class_board.h>
#include <tool/tool_manager.h>
#include <tools/pcb_actions.h>
#include <tools/pcbnew_control.h>
#include <dialog_print_generic.h>
#include <pcbnew_printout.h>

class DIALOG_PRINT_PCBNEW : public DIALOG_PRINT_GENERIC
{
public:
    DIALOG_PRINT_PCBNEW( PCB_BASE_EDIT_FRAME* aParent, PCBNEW_PRINTOUT_SETTINGS* aSettings );
    ~DIALOG_PRINT_PCBNEW() {}

private:
    PCBNEW_PRINTOUT_SETTINGS* settings() const
    {
        wxASSERT( dynamic_cast<PCBNEW_PRINTOUT_SETTINGS*>( m_settings ) );
        return static_cast<PCBNEW_PRINTOUT_SETTINGS*>( m_settings );
    }

    bool TransferDataToWindow() override;

    void createExtraOptions();
    void createLeftPanel();

    void onSelectAllClick( wxCommandEvent& event );
    void onDeselectAllClick( wxCommandEvent& event );

    void onUseThemeChecked( wxCommandEvent& event );
    void onColorModeChanged( wxCommandEvent& event );

    ///> (Un)checks all items in a checklist box
    void setListBoxValue( wxCheckListBox* aList, bool aValue );

    ///> Check whether a layer is enabled in a listbox
    bool isLayerEnabled( unsigned int aLayer ) const;

    ///> Enable/disable layer in a listbox
    void enableLayer( unsigned int aLayer, bool aValue );

    ///> Update layerset basing on the selected layers
    int setLayerSetFromList();

    void saveSettings() override;

    wxPrintout* createPrintout( const wxString& aTitle ) override
    {
        return new PCBNEW_PRINTOUT( m_parent->GetBoard(), *settings(),
                                    m_parent->GetCanvas()->GetView(), aTitle );
    }

    PCB_BASE_EDIT_FRAME* m_parent;

    // List of existing board layers in wxCheckListBox, with the board layers id:
    std::pair<wxCheckListBox*, int> m_layers[PCB_LAYER_ID_COUNT];

    // Extra widgets
    wxCheckListBox* m_listTechLayers;
    wxCheckListBox* m_listCopperLayers;
    wxButton* m_buttonSelectAll;
    wxButton* m_buttonDeselectAll;
    wxCheckBox* m_checkboxNoEdge;
    wxCheckBox* m_checkboxMirror;
    wxChoice* m_drillMarksChoice;
    wxRadioBox* m_boxPagination;
    wxCheckBox* m_checkAsItems;
    wxCheckBox* m_checkBackground;
    wxCheckBox* m_checkUseTheme;
    wxChoice* m_colorTheme;
};


DIALOG_PRINT_PCBNEW::DIALOG_PRINT_PCBNEW( PCB_BASE_EDIT_FRAME* aParent,
                                          PCBNEW_PRINTOUT_SETTINGS* aSettings ) :
    DIALOG_PRINT_GENERIC( aParent, aSettings ),
    m_parent( aParent )
{
    m_config = Kiface().KifaceSettings();

    createExtraOptions();
    createLeftPanel();

    m_outputMode->Bind( wxEVT_COMMAND_CHOICE_SELECTED, &DIALOG_PRINT_PCBNEW::onColorModeChanged, this );
}


bool DIALOG_PRINT_PCBNEW::TransferDataToWindow()
{
    if( !DIALOG_PRINT_GENERIC::TransferDataToWindow() )
        return false;

    BOARD* board = m_parent->GetBoard();

    // Create layer list
    for( LSEQ seq = board->GetEnabledLayers().UIOrder(); seq; ++seq )
    {
        PCB_LAYER_ID layer = *seq;
        int checkIndex;

        if( IsCopperLayer( layer ) )
        {
            checkIndex = m_listCopperLayers->Append( board->GetLayerName( layer ) );
            m_layers[layer] = std::make_pair( m_listCopperLayers, checkIndex );
        }
        else
        {
            checkIndex = m_listTechLayers->Append( board->GetLayerName( layer ) );
            m_layers[layer] = std::make_pair( m_listTechLayers, checkIndex );
        }

        m_layers[layer].first->Check( checkIndex, settings()->m_layerSet.test( layer ) );
    }

    m_checkboxMirror->SetValue( settings()->m_mirror );
    m_checkboxNoEdge->SetValue( settings()->m_noEdgeLayer );
    m_titleBlock->SetValue( settings()->m_titleBlock );

    PCBNEW_SETTINGS* cfg = m_parent->GetPcbNewSettings();

    m_checkBackground->SetValue( cfg->m_Printing.background );
    m_checkUseTheme->SetValue( cfg->m_Printing.use_theme );

    m_colorTheme->Clear();

    int width    = 0;
    int height   = 0;
    int minwidth = width;

    wxString target = cfg->m_Printing.use_theme ? cfg->m_Printing.color_theme : cfg->m_ColorTheme;

    for( COLOR_SETTINGS* settings : Pgm().GetSettingsManager().GetColorSettingsList() )
    {
        int pos = m_colorTheme->Append( settings->GetName(), static_cast<void*>( settings ) );

        if( settings->GetFilename() == target )
            m_colorTheme->SetSelection( pos );

        m_colorTheme->GetTextExtent( settings->GetName(), &width, &height );
        minwidth = std::max( minwidth, width );
    }

    m_colorTheme->SetMinSize( wxSize( minwidth + 50, -1 ) );

    wxCommandEvent dummy;
    onColorModeChanged( dummy );

    // Options to plot pads and vias holes
    m_drillMarksChoice->SetSelection( settings()->m_drillMarks );

    // Print all layers one one page or separately
    m_boxPagination->SetSelection( settings()->m_pagination );

    // Update the dialog layout when layers are added
    GetSizer()->Fit( this );

    return true;
}


void DIALOG_PRINT_PCBNEW::createExtraOptions()
{
    wxGridBagSizer* optionsSizer = getOptionsSizer();
    wxStaticBox* box = getOptionsBox();
    int rows = optionsSizer->GetEffectiveRowsCount();
    int cols = optionsSizer->GetEffectiveColsCount();

    m_checkAsItems = new wxCheckBox( sbOptionsSizer->GetStaticBox(), wxID_ANY,
                                     _( "Print according to objects tab of appearance manager" ),
                                     wxDefaultPosition, wxDefaultSize, 0 );
    optionsSizer->Add( m_checkAsItems, wxGBPosition( rows++, 0 ), wxGBSpan( 1, 3 ), wxALL, 5 );

    m_checkBackground = new wxCheckBox( sbOptionsSizer->GetStaticBox(), wxID_ANY,
                                        _( "Print background color" ), wxDefaultPosition,
                                        wxDefaultSize, 0 );
    optionsSizer->Add( m_checkBackground, wxGBPosition( rows++, 0 ), wxGBSpan( 1, 3 ), wxALL, 5 );

    m_checkUseTheme = new wxCheckBox( sbOptionsSizer->GetStaticBox(), wxID_ANY,
                                      _( "Use a different color theme for printing:" ),
                                      wxDefaultPosition, wxDefaultSize, 0 );
    optionsSizer->Add( m_checkUseTheme, wxGBPosition( rows++, 0 ), wxGBSpan( 1, 3 ), wxALL, 5 );

    m_checkUseTheme->Bind( wxEVT_COMMAND_CHECKBOX_CLICKED, &DIALOG_PRINT_PCBNEW::onUseThemeChecked, this );

    wxArrayString m_colorThemeChoices;
    m_colorTheme = new wxChoice( sbOptionsSizer->GetStaticBox(), wxID_ANY, wxDefaultPosition,
                                 wxDefaultSize, m_colorThemeChoices, 0 );
    m_colorTheme->SetSelection( 0 );

    optionsSizer->Add( m_colorTheme, wxGBPosition( rows++, 0 ), wxGBSpan( 1, 2 ),
                       wxALIGN_CENTER_VERTICAL | wxLEFT, 25 );

    // Drill marks option
    auto drillMarksLabel = new wxStaticText( box, wxID_ANY, _( "Drill marks:" ) );
    std::vector<wxString> drillMarkChoices = { _( "No drill mark" ),
                                               _( "Small mark" ),
                                               _( "Real drill" ) };
    m_drillMarksChoice = new wxChoice( box, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                       drillMarkChoices.size(), drillMarkChoices.data(), 0 );
    m_drillMarksChoice->SetSelection( 0 );

    // Print mirrored
    m_checkboxMirror = new wxCheckBox( box, wxID_ANY, _( "Print mirrored" ) );

    // Pagination
    std::vector<wxString> pagesOption = { _( "One page per layer" ),
                                          _( "All layers on single page" ) };
    m_boxPagination = new wxRadioBox( box, wxID_ANY, _( "Pagination" ), wxDefaultPosition,
                                      wxDefaultSize, pagesOption.size(), pagesOption.data(), 1,
                                      wxRA_SPECIFY_COLS );
    m_boxPagination->SetSelection( 0 );

    // Sizer layout
    rows++;

    optionsSizer->Add( drillMarksLabel, wxGBPosition( rows, 0 ), wxGBSpan( 1, 1 ),
                       wxBOTTOM | wxRIGHT | wxLEFT | wxALIGN_CENTER_VERTICAL, 5 );
    optionsSizer->Add( m_drillMarksChoice, wxGBPosition( rows, 1 ), wxGBSpan( 1, cols - 1 ),
                       wxBOTTOM | wxRIGHT | wxLEFT, 5 );
    optionsSizer->Add( m_checkboxMirror, wxGBPosition( rows + 1, 0 ), wxGBSpan( 1, cols ),
                       wxBOTTOM | wxRIGHT | wxLEFT, 5 );
    optionsSizer->Add( m_boxPagination, wxGBPosition( rows + 2, 0 ), wxGBSpan( 1, cols ),
                       wxALL | wxEXPAND, 5 );
}


void DIALOG_PRINT_PCBNEW::createLeftPanel()
{
    wxStaticBox* box = new wxStaticBox( this, wxID_ANY, _( "Included Layers" ) );
    wxStaticBoxSizer* sbLayersSizer = new wxStaticBoxSizer( box, wxVERTICAL );

    // Copper layer list
    auto copperLabel = new wxStaticText( sbLayersSizer->GetStaticBox(), wxID_ANY,
                                         _( "Copper layers:" ) );
    m_listCopperLayers = new wxCheckListBox( sbLayersSizer->GetStaticBox(), wxID_ANY );

    wxBoxSizer* sizerLeft = new wxBoxSizer( wxVERTICAL );
    sizerLeft->Add( copperLabel, 0, wxRIGHT, 5 );
    sizerLeft->Add( m_listCopperLayers, 1, wxEXPAND | wxBOTTOM | wxRIGHT, 5 );


    // Technical layer list
    auto technicalLabel = new wxStaticText( sbLayersSizer->GetStaticBox(), wxID_ANY,
                                            _( "Technical layers:" ) );
    m_listTechLayers = new wxCheckListBox( sbLayersSizer->GetStaticBox(), wxID_ANY );

    wxBoxSizer* sizerRight = new wxBoxSizer( wxVERTICAL );
    sizerRight->Add( technicalLabel, 0, wxLEFT, 5 );
    sizerRight->Add( m_listTechLayers, 1, wxEXPAND | wxBOTTOM | wxLEFT, 5 );


    // Layer list layout
    wxBoxSizer* bLayerListsSizer = new wxBoxSizer( wxHORIZONTAL );
    bLayerListsSizer->Add( sizerLeft, 1, wxEXPAND, 5 );
    bLayerListsSizer->Add( sizerRight, 1, wxEXPAND, 5 );


    // Select/Unselect all buttons
    m_buttonSelectAll = new wxButton( sbLayersSizer->GetStaticBox(), wxID_ANY, _( "Select all" ) );
    m_buttonDeselectAll = new wxButton( sbLayersSizer->GetStaticBox(), wxID_ANY, _( "Deselect all" ) );

    m_buttonSelectAll->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler( DIALOG_PRINT_PCBNEW::onSelectAllClick ), NULL, this );
    m_buttonDeselectAll->Connect( wxEVT_COMMAND_BUTTON_CLICKED,
            wxCommandEventHandler( DIALOG_PRINT_PCBNEW::onDeselectAllClick ), NULL, this );

    wxBoxSizer* buttonSizer = new wxBoxSizer( wxHORIZONTAL );
    buttonSizer->Add( m_buttonSelectAll, 1, wxRIGHT | wxTOP | wxBOTTOM, 5 );
    buttonSizer->Add( m_buttonDeselectAll, 1, wxLEFT | wxTOP | wxBOTTOM, 5 );


    // Exclude Edge.Pcb layer checkbox
    m_checkboxNoEdge = new wxCheckBox( sbLayersSizer->GetStaticBox(), wxID_ANY, _( "Exclude PCB edge layer" ) );
    m_checkboxNoEdge->SetToolTip( _("Exclude contents of Edges_Pcb layer from all other layers") );

    // Static box sizer layout
    sbLayersSizer->Add( bLayerListsSizer, 1, wxRIGHT | wxEXPAND, 5 );
    sbLayersSizer->Add( buttonSizer, 0, wxRIGHT | wxEXPAND, 5 );
    sbLayersSizer->Add( m_checkboxNoEdge, 0, wxTOP | wxRIGHT | wxBOTTOM | wxEXPAND, 5 );

    getMainSizer()->Insert( 0, sbLayersSizer, 1, wxEXPAND | wxALL, 5 );
}


void DIALOG_PRINT_PCBNEW::onSelectAllClick( wxCommandEvent& event )
{
    setListBoxValue( m_listCopperLayers, true );
    setListBoxValue( m_listTechLayers, true );
}


void DIALOG_PRINT_PCBNEW::onDeselectAllClick( wxCommandEvent& event )
{
    setListBoxValue( m_listCopperLayers, false );
    setListBoxValue( m_listTechLayers, false );
}


void DIALOG_PRINT_PCBNEW::onUseThemeChecked( wxCommandEvent& event )
{
    m_colorTheme->Enable( m_checkUseTheme->GetValue() );
}


void DIALOG_PRINT_PCBNEW::onColorModeChanged( wxCommandEvent& event )
{
    PCBNEW_SETTINGS* cfg = m_parent->GetPcbNewSettings();

    m_settings->m_blackWhite = m_outputMode->GetSelection();

    m_checkBackground->Enable( !m_settings->m_blackWhite );
    m_checkUseTheme->Enable( !m_settings->m_blackWhite );
    m_colorTheme->Enable( !m_settings->m_blackWhite && cfg->m_Printing.use_theme );
}


void DIALOG_PRINT_PCBNEW::setListBoxValue( wxCheckListBox* aList, bool aValue )
{
    for( unsigned int i = 0; i < aList->GetCount(); ++i )
        aList->Check( i, aValue );
}


bool DIALOG_PRINT_PCBNEW::isLayerEnabled( unsigned int aLayer ) const
{
    wxCHECK( aLayer < arrayDim( m_layers ), false );
    const auto& layerInfo = m_layers[aLayer];

    if( layerInfo.first )
        return layerInfo.first->IsChecked( layerInfo.second );

    return false;
}


void DIALOG_PRINT_PCBNEW::enableLayer( unsigned int aLayer, bool aValue )
{
    wxCHECK( aLayer < arrayDim( m_layers ), /* void */ );
    const auto& layerInfo = m_layers[aLayer];
    layerInfo.first->Check( layerInfo.second, aValue );
}


int DIALOG_PRINT_PCBNEW::setLayerSetFromList()
{
    settings()->m_layerSet = LSET();
    int& pageCount = settings()->m_pageCount;
    pageCount = 0;

    for( unsigned int layer = 0; layer < arrayDim( m_layers ); ++layer )
    {
        if( isLayerEnabled( layer ) )
        {
            ++pageCount;
            settings()->m_layerSet.set( layer );
        }
    }

    // In Pcbnew force the EDGE layer to be printed or not with the other layers
    settings()->m_noEdgeLayer = m_checkboxNoEdge->IsChecked();

    // All layers on one page (only if there is at least one layer selected)
    if( m_boxPagination->GetSelection() != 0 && pageCount > 0 )
        pageCount = 1;

    return pageCount;
}


void DIALOG_PRINT_PCBNEW::saveSettings()
{
    setLayerSetFromList();

    settings()->m_asItemCheckboxes = m_checkAsItems->GetValue();

    settings()->m_drillMarks =
        (PCBNEW_PRINTOUT_SETTINGS::DRILL_MARK_SHAPE_T) m_drillMarksChoice->GetSelection();

    settings()->m_pagination = m_boxPagination->GetSelection() == 0
        ? PCBNEW_PRINTOUT_SETTINGS::LAYER_PER_PAGE : PCBNEW_PRINTOUT_SETTINGS::ALL_LAYERS;

    settings()->m_mirror = m_checkboxMirror->GetValue();

    PCBNEW_SETTINGS* cfg = m_parent->GetPcbNewSettings();

    cfg->m_Printing.background = m_checkBackground->GetValue();
    settings()->m_background   = cfg->m_Printing.background;
    cfg->m_Printing.use_theme  = m_checkUseTheme->GetValue();

    int sel = m_colorTheme->GetSelection();
    COLOR_SETTINGS* theme = static_cast<COLOR_SETTINGS*>( m_colorTheme->GetClientData( sel ) );

    if( theme && m_checkUseTheme->IsChecked() )
    {
        cfg->m_Printing.color_theme = theme->GetFilename();
        settings()->m_colorSettings = theme;
    }
    else
    {
        // This should always work, but in case it doesn't we fall back on default colors
        if( PCB_BASE_EDIT_FRAME* pcbframe = dynamic_cast<PCB_BASE_EDIT_FRAME*>( m_parent ) )
            settings()->m_colorSettings = pcbframe->GetColorSettings();
        else
            settings()->m_colorSettings = m_parent->GetColorSettings();
    }

    DIALOG_PRINT_GENERIC::saveSettings();
}


int PCBNEW_CONTROL::Print( const TOOL_EVENT& aEvent )
{
    // Selection affects the origin item visibility
    m_toolMgr->RunAction( PCB_ACTIONS::selectionClear, true );

    PCBNEW_PRINTOUT_SETTINGS settings( m_frame->GetPageSettings() );
    DIALOG_PRINT_PCBNEW dlg( (PCB_BASE_EDIT_FRAME*) m_frame, &settings );

    if( m_isFootprintEditor )
        dlg.ForcePrintBorder( false );

    dlg.ShowModal();

    return 0;
}


