/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004-2020 KiCad Developers.
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

#include <common.h>
#include <class_board.h>
#include <class_drawsegment.h>
#include <class_pad.h>

#include <convert_basic_shapes_to_polygon.h>
#include <geometry/polygon_test_point_inside.h>

#include <geometry/seg.h>
#include <geometry/shape_poly_set.h>
#include <geometry/shape_rect.h>
#include <geometry/shape_segment.h>

#include <drc_proto/drc_engine.h>
#include <drc_proto/drc_item.h>
#include <drc_proto/drc_rule.h>
#include <drc_proto/drc_test_provider_clearance_base.h>

/*
    Couartyard clearance. Tests for malformed component courtyards and overlapping footprints.
    Generated errors:
    - DRCE_OVERLAPPING_FOOTPRINTS
    - DRCE_MISSING_COURTYARD
    - DRCE_MALFORMED_COURTYARD

    TODO: do an actual clearance check instead of polygon intersection. Treat closed outlines
    as filled and allow open curves in the courtyard.
*/

namespace test {

class DRC_TEST_PROVIDER_COURTYARD_CLEARANCE : public DRC_TEST_PROVIDER_CLEARANCE_BASE
{
public:
    DRC_TEST_PROVIDER_COURTYARD_CLEARANCE ()
    {
        m_isRuleDriven = false;
    }

    virtual ~DRC_TEST_PROVIDER_COURTYARD_CLEARANCE () 
    {
    }

    virtual bool Run() override;

    virtual const wxString GetName() const override 
    {
        return "courtyard_clearance";
    };

    virtual const wxString GetDescription() const override
    {
        return "Tests components' courtyard clearance";
    }

    virtual std::set<test::DRC_CONSTRAINT_TYPE_T> GetMatchingConstraintIds() const override;

private:

    void testFootprintCourtyardDefinitions();
    void testOverlappingComponentCourtyards();

};

};


void test::DRC_TEST_PROVIDER_COURTYARD_CLEARANCE::testFootprintCourtyardDefinitions()
{
    ReportStage( _("Testing component courtyard definitions"), 0, 2 );

    for( MODULE* footprint : m_board->Modules() )
    {
        if( footprint->BuildPolyCourtyard() )
        {
            if( footprint->GetPolyCourtyardFront().OutlineCount() == 0
                && footprint->GetPolyCourtyardBack().OutlineCount() == 0 )
            {
                if( isErrorLimitExceeded( DRCE_MISSING_COURTYARD ) )
                    continue;

                std::shared_ptr<DRC_ITEM> drcItem = DRC_ITEM::Create( DRCE_MISSING_COURTYARD );
                wxString msg;

                msg.Printf( drcItem->GetErrorText( ));

                drcItem->SetItems( footprint );
                ReportWithMarker( drcItem, footprint->GetPosition() );
            }
            else
            {
                footprint->GetPolyCourtyardFront().BuildBBoxCaches();
                footprint->GetPolyCourtyardBack().BuildBBoxCaches();
            }
        }
        else
        {
            if( !isErrorLimitExceeded( DRCE_MALFORMED_COURTYARD) )
            {
                wxString msg;
                std::shared_ptr<DRC_ITEM> drcItem = DRC_ITEM::Create( DRCE_MALFORMED_COURTYARD );

                msg.Printf( drcItem->GetErrorText() + _( " (not a closed shape)" ) );

                drcItem->SetErrorMessage( msg );
                drcItem->SetItems( footprint );
                ReportWithMarker( drcItem, footprint->GetPosition() );
            }
        }
    }
}


void test::DRC_TEST_PROVIDER_COURTYARD_CLEARANCE::testOverlappingComponentCourtyards()
{
    ReportStage( _("Testing component courtyard overlap"), 0, 2 );

    for( auto it1 = m_board->Modules().begin(); it1 != m_board->Modules().end(); it1++ )
        {
            MODULE*         footprint = *it1;
            SHAPE_POLY_SET& footprintFront = footprint->GetPolyCourtyardFront();
            SHAPE_POLY_SET& footprintBack = footprint->GetPolyCourtyardBack();

            if( footprintFront.OutlineCount() == 0 && footprintBack.OutlineCount() == 0 )
                continue; // No courtyards defined

            for( auto it2 = it1 + 1; it2 != m_board->Modules().end(); it2++ )
            {
                MODULE*         test = *it2;
                SHAPE_POLY_SET& testFront = test->GetPolyCourtyardFront();
                SHAPE_POLY_SET& testBack = test->GetPolyCourtyardBack();
                SHAPE_POLY_SET  intersection;
                bool            overlap = false;
                wxPoint         pos;

                if( footprintFront.OutlineCount() > 0 && testFront.OutlineCount() > 0
                    && footprintFront.BBoxFromCaches().Intersects( testFront.BBoxFromCaches() ) )
                {
                    intersection.RemoveAllContours();
                    intersection.Append( footprintFront );

                    // Build the common area between footprint and the test:
                    intersection.BooleanIntersection( testFront, SHAPE_POLY_SET::PM_FAST );

                    // If the intersection exists then they overlap
                    if( intersection.OutlineCount() > 0 )
                    {
                        overlap = true;
                        pos = (wxPoint) intersection.CVertex( 0, 0, -1 );
                    }
                }

                if( footprintBack.OutlineCount() > 0 && testBack.OutlineCount() > 0
                    && footprintBack.BBoxFromCaches().Intersects( testBack.BBoxFromCaches() ) )
                {
                    intersection.RemoveAllContours();
                    intersection.Append( footprintBack );

                    intersection.BooleanIntersection( testBack, SHAPE_POLY_SET::PM_FAST );

                    if( intersection.OutlineCount() > 0 )
                    {
                        overlap = true;
                        pos = (wxPoint) intersection.CVertex( 0, 0, -1 );
                    }
                }

                if( overlap )
                {
                    std::shared_ptr<DRC_ITEM> drcItem = DRC_ITEM::Create( DRCE_OVERLAPPING_FOOTPRINTS );
                    drcItem->SetItems( footprint, test );
                    ReportWithMarker ( drcItem, pos );

                    if( isErrorLimitExceeded( DRCE_OVERLAPPING_FOOTPRINTS ) )
                        return;
                }
            }
        }
}


bool test::DRC_TEST_PROVIDER_COURTYARD_CLEARANCE::Run()
{
    m_board = m_drcEngine->GetBoard();

    // fixme: don't use polygon intersection but distance for clearance tests
    //m_largestClearance = 0;
    //ReportAux( "Worst courtyard clearance : %d nm", m_largestClearance );

    testFootprintCourtyardDefinitions();
    testOverlappingComponentCourtyards();

    return true;
}


std::set<test::DRC_CONSTRAINT_TYPE_T> test::DRC_TEST_PROVIDER_COURTYARD_CLEARANCE::GetMatchingConstraintIds() const
{
    return { DRC_CONSTRAINT_TYPE_T::DRC_CONSTRAINT_TYPE_COURTYARD_CLEARANCE };
}


namespace detail
{
    static test::DRC_REGISTER_TEST_PROVIDER<test::DRC_TEST_PROVIDER_COURTYARD_CLEARANCE> dummy;
}