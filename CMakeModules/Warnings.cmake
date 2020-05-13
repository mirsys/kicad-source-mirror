#
#  This program source code file is part of KICAD, a free EDA CAD application.
#
#  Copyright (C) 2007-2020 Kicad Developers, see AUTHORS.txt for contributors.
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, you may find one here:
#  http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
#  or you may search the http://www.gnu.org website for the version 2 license,
#  or you may write to the Free Software Foundation, Inc.,
#  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
#

# Only configure warnings for Clang and GCC
if( CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang" )
    include( CheckCXXCompilerFlag )


    # The SWIG-generated files tend to throw a lot of warnings, so
    # we do not add the warnings directly to the flags here but instead
    # keep track of them and add them to the flags later in a controlled manner
    # (that way we can not put any warnings on the SWIG-generated files)
    set( COMPILER_SUPPORTS_WARNINGS TRUE )

    # Establish -Wall early, so specialized relaxations of this may come
    # subsequently on the command line, such as in pcbnew/github/CMakeLists.txt
    set( WARN_FLAGS_C   "-Wall" )
    set( WARN_FLAGS_CXX "-Wall" )


    # Warn about missing override specifiers
    CHECK_CXX_COMPILER_FLAG( "-Wsuggest-override" COMPILER_SUPPORTS_WSUGGEST_OVERRIDE )

    if( COMPILER_SUPPORTS_WSUGGEST_OVERRIDE )
        set( WARN_FLAGS_CXX "${WARN_FLAGS_CXX} -Wsuggest-override" )
        message( STATUS "Enabling warning -Wsuggest-override" )
    endif()


    # This is Clang's version of -Wsuggest-override
    CHECK_CXX_COMPILER_FLAG( "-Winconsistent-missing-override" COMPILER_SUPPORTS_WINCONSISTENT_MISSING_OVERRIDE )

    if( COMPILER_SUPPORTS_WINCONSISTENT_MISSING_OVERRIDE )
        set( WARN_FLAGS_CXX "${WARN_FLAGS_CXX} -Winconsistent-missing-override" )
        message( STATUS "Enabling warning -Winconsistent-missing-override" )
    endif()


    # Warn on duplicated branches
    CHECK_CXX_COMPILER_FLAG( "-Wduplicated-branches" COMPILER_SUPPORTS_WDUPLICATED_BRANCHES )

    if( COMPILER_SUPPORTS_WDUPLICATED_BRANCHES )
        set( WARN_FLAGS_CXX "${WARN_FLAGS_CXX} -Wduplicated-branches" )
        message( STATUS "Enabling warning -Wduplicated-branches" )
    endif()


    # Warn on duplicated conditions
    CHECK_CXX_COMPILER_FLAG( "-Wduplicated-cond" COMPILER_SUPPORTS_WDUPLICATED_COND )

    if( COMPILER_SUPPORTS_WDUPLICATED_COND )
        set( WARN_FLAGS_CXX "${WARN_FLAGS_CXX} -Wduplicated-cond" )
        message( STATUS "Enabling warning -Wduplicated-cond" )
    endif()



    CHECK_CXX_COMPILER_FLAG( "-Wvla" COMPILER_SUPPORTS_WVLA )

    if( COMPILER_SUPPORTS_WVLA )
        set( WARN_FLAGS_CXX "${WARN_FLAGS_CXX} -Werror=vla" )
        message( STATUS "Enabling error for -Wvla" )
    endif()


    # Warn on implicit switch fallthrough
    CHECK_CXX_COMPILER_FLAG( "-Wimplicit-fallthrough" COMPILER_SUPPORTS_WIMPLICIT_FALLTHROUGH )

    if( COMPILER_SUPPORTS_WIMPLICIT_FALLTHROUGH )
        if( CMAKE_COMPILER_IS_GNUCXX )
            # GCC level 5 does not allow comments - mirrors the Clang warning
            set( WARN_FLAGS_CXX "${WARN_FLAGS_CXX} -Wimplicit-fallthrough=5" )
        else()
            set( WARN_FLAGS_CXX "${WARN_FLAGS_CXX} -Wimplicit-fallthrough" )
        endif()

        message( STATUS "Enabling warning -Wimplicit-fallthrough" )
    endif()


    # Error if there is a problem with function returns
    CHECK_CXX_COMPILER_FLAG( "-Wreturn-type" COMPILER_SUPPORTS_WRETURN_TYPE )

    if( COMPILER_SUPPORTS_WRETURN_TYPE )
        set( WARN_FLAGS_CXX "${WARN_FLAGS_CXX} -Werror=return-type" )
        message( STATUS "Enabling error for -Wreturn-type" )
    endif()


    # Warn about shadowed variables
    CHECK_CXX_COMPILER_FLAG( "-Wshadow" COMPILER_SUPPORTS_WSHADOW )

    if( COMPILER_SUPPORTS_WSHADOW )
        set( WARN_FLAGS_CXX "${WARN_FLAGS_CXX} -Wshadow" )
        message( STATUS "Enabling warning -Wshadow" )
    endif()

endif()