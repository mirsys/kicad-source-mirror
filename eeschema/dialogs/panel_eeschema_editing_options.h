/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2017-2020 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PANEL_EESCHEMA_EDITING_OPTIONS_H
#define PANEL_EESCHEMA_EDITING_OPTIONS_H

#include <widgets/unit_binder.h>
#include "panel_eeschema_editing_options_base.h"

class SCH_EDIT_FRAME;


class PANEL_EESCHEMA_EDITING_OPTIONS : public PANEL_EESCHEMA_EDITING_OPTIONS_BASE
{
    SCH_EDIT_FRAME* m_frame;

    UNIT_BINDER     m_hPitch;
    UNIT_BINDER     m_vPitch;

public:
    PANEL_EESCHEMA_EDITING_OPTIONS( SCH_EDIT_FRAME* aFrame, wxWindow* aWindow );

private:
    bool TransferDataToWindow() override;
    bool TransferDataFromWindow() override;
};


#endif //PANEL_EESCHEMA_EDITING_OPTIONS_H
