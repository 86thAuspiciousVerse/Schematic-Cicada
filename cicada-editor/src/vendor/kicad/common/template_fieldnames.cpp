/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2010 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright The KiCad Developers, see AUTHORS.TXT for contributors.
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

#include "template_fieldnames.h"

#include <mutex>

#include <string_utils.h>



// N.B. Do not change these values without transitioning the file format
#define REFERENCE_CANONICAL "Reference"
#define VALUE_CANONICAL "Value"
#define FOOTPRINT_CANONICAL "Footprint"
#define DATASHEET_CANONICAL "Datasheet"
#define DESCRIPTION_CANONICAL "Description"
#define SHEET_NAME_CANONICAL "Sheetname"
#define SHEET_FILE_CANONICAL "Sheetfile"
#define INTERSHEET_REFS_CANONICAL "Intersheetrefs"
#define USER_FIELD_CANONICAL "Field%d"

static wxString s_CanonicalReference( REFERENCE_CANONICAL );
static wxString s_CanonicalValue( VALUE_CANONICAL );
static wxString s_CanonicalFootprint( FOOTPRINT_CANONICAL );
static wxString s_CanonicalDatasheet( DATASHEET_CANONICAL );
static wxString s_CanonicalDescription( DESCRIPTION_CANONICAL );
static wxString s_CanonicalSheetName( SHEET_NAME_CANONICAL );
static wxString s_CanonicalSheetFile( SHEET_FILE_CANONICAL );
static wxString s_CanonicalIntersheetRefs( INTERSHEET_REFS_CANONICAL );


wxString GetDefaultFieldName( FIELD_T aFieldId, bool aTranslateForHI )
{
    if( !aTranslateForHI )
    {
        switch( aFieldId )
        {
        case FIELD_T::REFERENCE:       return s_CanonicalReference;   // The symbol reference, R1, C1, etc.
        case FIELD_T::VALUE:           return s_CanonicalValue;       // The symbol value
        case FIELD_T::FOOTPRINT:       return s_CanonicalFootprint;   // The footprint for use with Pcbnew
        case FIELD_T::DATASHEET:       return s_CanonicalDatasheet;   // Link to a datasheet for symbol
        case FIELD_T::DESCRIPTION:     return s_CanonicalDescription; // The symbol description
        case FIELD_T::SHEET_NAME:      return s_CanonicalSheetName;
        case FIELD_T::SHEET_FILENAME:  return s_CanonicalSheetFile;
        case FIELD_T::INTERSHEET_REFS: return s_CanonicalIntersheetRefs;
        default:                       return GetUserFieldName( 42, aTranslateForHI );
        }
    }
    else
    {
        switch( aFieldId )
        {
        case FIELD_T::REFERENCE:       return _( REFERENCE_CANONICAL );   // The symbol reference, R1, C1, etc.
        case FIELD_T::VALUE:           return _( VALUE_CANONICAL );       // The symbol value
        case FIELD_T::FOOTPRINT:       return _( FOOTPRINT_CANONICAL );   // The footprint for use with Pcbnew
        case FIELD_T::DATASHEET:       return _( DATASHEET_CANONICAL );   // Link to a datasheet for symbol
        case FIELD_T::DESCRIPTION:     return _( DESCRIPTION_CANONICAL ); // The symbol description
        case FIELD_T::SHEET_NAME:      return _( SHEET_NAME_CANONICAL );
        case FIELD_T::SHEET_FILENAME:  return _( SHEET_FILE_CANONICAL );
        case FIELD_T::INTERSHEET_REFS: return _( INTERSHEET_REFS_CANONICAL );
        default:                       return GetUserFieldName( 42, aTranslateForHI );
        }
    }
}


wxString GetUserFieldName( int aFieldNdx, bool aTranslateForHI )
{
    if( !aTranslateForHI )
        return wxString::Format( wxS( USER_FIELD_CANONICAL ), aFieldNdx );
    else
        return wxString::Format( _( USER_FIELD_CANONICAL ), aFieldNdx );
}


bool FieldNamesAreDuplicates( const wxString& aLhs, const wxString& aRhs,
                              std::initializer_list<FIELD_T> aMandatoryFields )
{
    if( aLhs == aRhs )
        return true;

    // If they don't even match case-insensitively they can't both be variants of the same
    // canonical mandatory field name.
    if( aLhs.CmpNoCase( aRhs ) != 0 )
        return false;

    // Mandatory field names are folded case-insensitively by the s-expression parser, so any
    // case variant of a mandatory canonical name collides with the canonical mandatory field.
    for( FIELD_T fieldId : aMandatoryFields )
    {
        if( aLhs.CmpNoCase( GetCanonicalFieldName( fieldId ) ) == 0 )
            return true;
    }

    return false;
}


bool FieldNamesAreDuplicates( const wxString& aLhs, const wxString& aRhs )
{
    return FieldNamesAreDuplicates( aLhs, aRhs, MANDATORY_FIELDS );
}










/**
 * Flatten project and global templates into a single list.  (Project templates take
 * precedence.)
 */
void TEMPLATES::resolveTemplates()
{
    m_resolved = m_project;

    // Note: order N^2 algorithm.  Would need changing if fieldname template sets ever
    // get large.

    for( const TEMPLATE_FIELDNAME& global : m_globals )
    {
        bool overriddenInProject = false;

        for( const TEMPLATE_FIELDNAME& project : m_project )
        {
            if( global.m_Name == project.m_Name )
            {
                overriddenInProject = true;
                break;
            }
        }

        if( !overriddenInProject )
            m_resolved.push_back( global );
    }

    m_resolvedDirty = false;
}


void TEMPLATES::AddTemplateFieldName( const TEMPLATE_FIELDNAME& aFieldName, bool aGlobal )
{
    // Reject any case variant of a mandatory fieldname; the s-expression parser folds those
    // onto the canonical mandatory field, so they can never become a distinct user field.
    for( FIELD_T fieldId : MANDATORY_FIELDS )
    {
        if( GetCanonicalFieldName( fieldId ).CmpNoCase( aFieldName.m_Name ) == 0 )
            return;
    }

    std::vector<TEMPLATE_FIELDNAME>& target = aGlobal ? m_globals : m_project;

    // ensure uniqueness, overwrite any template fieldname by the same name.
    for( TEMPLATE_FIELDNAME& temp : target )
    {
        if( temp.m_Name == aFieldName.m_Name )
        {
            temp = aFieldName;
            m_resolvedDirty = true;
            return;
        }
    }

    // the name is legal and not previously added to the config container, append
    // it and return its index within the container.
    target.push_back( aFieldName );
    m_resolvedDirty = true;
}




void TEMPLATES::DeleteAllFieldNameTemplates( bool aGlobal )
{
    if( aGlobal )
    {
        m_globals.clear();
        m_resolved = m_project;
    }
    else
    {
        m_project.clear();
        m_resolved = m_globals;
    }

    m_resolvedDirty = false;
}


const std::vector<TEMPLATE_FIELDNAME>& TEMPLATES::GetTemplateFieldNames()
{
    if( m_resolvedDirty )
        resolveTemplates();

    return m_resolved;
}


const std::vector<TEMPLATE_FIELDNAME>& TEMPLATES::GetTemplateFieldNames( bool aGlobal )
{
    if( aGlobal )
        return m_globals;
    else
        return m_project;
}


const TEMPLATE_FIELDNAME* TEMPLATES::GetFieldName( const wxString& aName )
{
    if( m_resolvedDirty )
        resolveTemplates();

    for( const TEMPLATE_FIELDNAME& field : m_resolved )
    {
        if( field.m_Name == aName )
            return &field;
    }

    return nullptr;
}

