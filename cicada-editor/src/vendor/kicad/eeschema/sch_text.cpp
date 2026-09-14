/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2016 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 2015 Wayne Stambaugh <stambaughw@gmail.com>
 * Copyright The KiCad Developers, see AUTHORS.txt for contributors.
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

#include <sch_symbol.h>
#include <lib_symbol.h>
#include <sch_sheet.h>
#include "markup_parser.h"
#include <advanced_config.h>
#include <base_units.h>
#include <pgm_base.h>
#include <sch_plotter.h>
#include <bitmaps.h>
#include <string_utils.h>
#include <geometry/geometry_utils.h>
#include <sch_text.h>
#include <schematic.h>
#include <settings/color_settings.h>
#include <default_values.h>
#include <wx/debug.h>
#include <wx/log.h>
#include <project/project_file.h>
#include <project/net_settings.h>
#include <core/mirror.h>
#include <core/kicad_algo.h>
#include <trigo.h>
#include <markup_parser.h>
#include <properties/property.h>
#include <properties/property_mgr.h>


SCH_TEXT::SCH_TEXT( const VECTOR2I& aPos, const wxString& aText, SCH_LAYER_ID aLayer, KICAD_T aType ) :
        SCH_ITEM( nullptr, aType ),
        EDA_TEXT( schIUScale, aText )
{
    m_layer = aLayer;

    SetTextPos( aPos );
    SetMultilineAllowed( true );

    m_excludedFromSim = false;
}


SCH_TEXT::SCH_TEXT( const SCH_TEXT& aText ) :
        SCH_ITEM( aText ),
        EDA_TEXT( aText )
{
    m_excludedFromSim = aText.m_excludedFromSim;
}


VECTOR2I SCH_TEXT::GetSchematicTextOffset( const RENDER_SETTINGS* aSettings ) const
{
    // Fudge factor to match KiCad 6
    return VECTOR2I( 0, -2500 );
}


void SCH_TEXT::NormalizeJustification( bool inverse )
{
    if( GetHorizJustify() == GR_TEXT_H_ALIGN_CENTER && GetVertJustify() == GR_TEXT_V_ALIGN_CENTER )
        return;

    VECTOR2I delta( 0, 0 );
    BOX2I    bbox = GetTextBox( nullptr );

    if( GetTextAngle().IsHorizontal() )
    {
        if( GetHorizJustify() == GR_TEXT_H_ALIGN_LEFT )
            delta.x = bbox.GetWidth() / 2;
        else if( GetHorizJustify() == GR_TEXT_H_ALIGN_RIGHT )
            delta.x = -bbox.GetWidth() / 2;

        if( GetVertJustify() == GR_TEXT_V_ALIGN_TOP )
            delta.y = -bbox.GetHeight() / 2;
        else if( GetVertJustify() == GR_TEXT_V_ALIGN_BOTTOM )
            delta.y = bbox.GetHeight() / 2;
    }
    else
    {
        if( GetHorizJustify() == GR_TEXT_H_ALIGN_LEFT )
            delta.y = bbox.GetWidth() / 2;
        else if( GetHorizJustify() == GR_TEXT_H_ALIGN_RIGHT )
            delta.y = -bbox.GetWidth() / 2;

        if( GetVertJustify() == GR_TEXT_V_ALIGN_TOP )
            delta.x = +bbox.GetHeight() / 2;
        else if( GetVertJustify() == GR_TEXT_V_ALIGN_BOTTOM )
            delta.x = -bbox.GetHeight() / 2;
    }

    if( inverse )
        SetTextPos( GetTextPos() - delta );
    else
        SetTextPos( GetTextPos() + delta );
}


void SCH_TEXT::MirrorHorizontally( int aCenter )
{
    if( m_layer == LAYER_DEVICE )
    {
        NormalizeJustification( false );
        int x = GetTextPos().x;

        x -= aCenter;
        x *= -1;
        x += aCenter;

        if( GetTextAngle().IsHorizontal() )
        {
            if( GetHorizJustify() == GR_TEXT_H_ALIGN_LEFT )
                SetHorizJustify( GR_TEXT_H_ALIGN_RIGHT );
            else if( GetHorizJustify() == GR_TEXT_H_ALIGN_RIGHT )
                SetHorizJustify( GR_TEXT_H_ALIGN_LEFT );
        }
        else
        {
            if( GetVertJustify() == GR_TEXT_V_ALIGN_TOP )
                SetVertJustify( GR_TEXT_V_ALIGN_BOTTOM );
            else if( GetVertJustify() == GR_TEXT_V_ALIGN_BOTTOM )
                SetVertJustify( GR_TEXT_V_ALIGN_TOP );
        }

        SetTextX( x );
        NormalizeJustification( true );
    }
    else
    {
        if( GetTextAngle() == ANGLE_HORIZONTAL )
            FlipHJustify();

        SetTextX( MIRRORVAL( GetTextPos().x, aCenter ) );
    }
}


void SCH_TEXT::MirrorVertically( int aCenter )
{
    if( m_layer == LAYER_DEVICE )
    {
        NormalizeJustification( false );
        int y = GetTextPos().y;

        y -= aCenter;
        y *= -1;
        y += aCenter;

        if( GetTextAngle().IsHorizontal() )
        {
            if( GetVertJustify() == GR_TEXT_V_ALIGN_TOP )
                SetVertJustify( GR_TEXT_V_ALIGN_BOTTOM );
            else if( GetVertJustify() == GR_TEXT_V_ALIGN_BOTTOM )
                SetVertJustify( GR_TEXT_V_ALIGN_TOP );
        }
        else
        {
            if( GetHorizJustify() == GR_TEXT_H_ALIGN_LEFT )
                SetHorizJustify( GR_TEXT_H_ALIGN_RIGHT );
            else if( GetHorizJustify() == GR_TEXT_H_ALIGN_RIGHT )
                SetHorizJustify( GR_TEXT_H_ALIGN_LEFT );
        }

        SetTextY( y );
        NormalizeJustification( true );
    }
    else
    {
        if( GetTextAngle() == ANGLE_VERTICAL )
            FlipHJustify();

        SetTextY( MIRRORVAL( GetTextPos().y, aCenter ) );
    }
}


void SCH_TEXT::Rotate( const VECTOR2I& aCenter, bool aRotateCCW )
{
    VECTOR2I pt = GetTextPos();
    RotatePoint( pt, aCenter, aRotateCCW ? ANGLE_90 : ANGLE_270 );
    VECTOR2I offset = pt - GetTextPos();

    Rotate90( false );

    SetTextPos( GetTextPos() + offset );
}


void SCH_TEXT::Rotate90( bool aClockwise )
{
    if( ( GetTextAngle() == ANGLE_HORIZONTAL && aClockwise ) || ( GetTextAngle() == ANGLE_VERTICAL && !aClockwise ) )
    {
        FlipHJustify();
    }

    SetTextAngle( GetTextAngle() == ANGLE_VERTICAL ? ANGLE_HORIZONTAL : ANGLE_VERTICAL );
}


void SCH_TEXT::MirrorSpinStyle( bool aLeftRight )
{
    if( ( GetTextAngle() == ANGLE_HORIZONTAL && aLeftRight ) || ( GetTextAngle() == ANGLE_VERTICAL && !aLeftRight ) )
    {
        FlipHJustify();
    }
}


void SCH_TEXT::swapData( SCH_ITEM* aItem )
{
    SCH_TEXT* item = static_cast<SCH_TEXT*>( aItem );

    SwapText( *item );
    SwapAttributes( *item );
}


bool SCH_TEXT::operator<( const SCH_ITEM& aItem ) const
{
    if( Type() != aItem.Type() )
        return Type() < aItem.Type();

    auto other = static_cast<const SCH_TEXT*>( &aItem );

    if( GetLayer() != other->GetLayer() )
        return GetLayer() < other->GetLayer();

    if( GetPosition().x != other->GetPosition().x )
        return GetPosition().x < other->GetPosition().x;

    if( GetPosition().y != other->GetPosition().y )
        return GetPosition().y < other->GetPosition().y;

    if( GetExcludedFromSim() != other->GetExcludedFromSim() )
        return GetExcludedFromSim() - other->GetExcludedFromSim();

    return GetText() < other->GetText();
}


int SCH_TEXT::GetTextOffset( const RENDER_SETTINGS* aSettings ) const
{
    double ratio;

    if( aSettings )
        ratio = static_cast<const SCH_RENDER_SETTINGS*>( aSettings )->m_TextOffsetRatio;
    else if( Schematic() )
        ratio = Schematic()->Settings().m_TextOffsetRatio;
    else
        ratio = DEFAULT_TEXT_OFFSET_RATIO; // For previews (such as in Preferences), etc.

    return KiROUND( ratio * GetTextSize().y );
}


int SCH_TEXT::GetPenWidth() const
{
    return GetEffectiveTextPenWidth();
}


KIFONT::FONT* SCH_TEXT::GetDrawFont( const RENDER_SETTINGS* aSettings ) const
{
    KIFONT::FONT* font = EDA_TEXT::GetFont();

    if( !font )
        font = KIFONT::FONT::GetFont( GetDefaultFont( aSettings ), IsBold(), IsItalic() );

    return font;
}


const BOX2I SCH_TEXT::GetBoundingBox() const
{
    BOX2I bbox = GetTextBox( nullptr );

    if( !GetTextAngle().IsZero() ) // Rotate bbox.
    {
        VECTOR2I pos = bbox.GetOrigin();
        VECTOR2I end = bbox.GetEnd();

        RotatePoint( pos, GetTextPos(), GetTextAngle() );
        RotatePoint( end, GetTextPos(), GetTextAngle() );

        bbox.SetOrigin( pos );
        bbox.SetEnd( end );
    }

    bbox.Normalize();
    return bbox;
}


wxString SCH_TEXT::GetShownText( const SCH_SHEET_PATH* aPath, bool aAllowExtraText, int aDepth ) const
{
    // Use local depth counter so each text element starts fresh
    int depth = 0;

    SCH_SHEET* sheet = nullptr;

    if( aPath )
        sheet = aPath->Last();
    else if( SCHEMATIC* schematic = Schematic() )
        sheet = schematic->CurrentSheet().Last();

    std::function<bool( wxString* )> textResolver = [&]( wxString* token ) -> bool
    {
        if( SCH_SYMBOL* sch_symbol = dynamic_cast<SCH_SYMBOL*>( m_parent ) )
        {
            if( sch_symbol->ResolveTextVar( aPath, token, depth + 1 ) )
                return true;
        }
        else if( LIB_SYMBOL* lib_symbol = dynamic_cast<LIB_SYMBOL*>( m_parent ) )
        {
            if( lib_symbol->ResolveTextVar( token, depth + 1 ) )
                return true;
        }

        if( sheet )
        {
            if( sheet->ResolveTextVar( aPath, token, depth + 1 ) )
                return true;
        }

        return false;
    };

    wxString text = EDA_TEXT::GetShownText( aAllowExtraText, depth );

    if( HasTextVars() )
        text = ResolveTextVars( text, &textResolver, depth );

    // Convert escape markers back to literals for final display
    text.Replace( wxT( "<<<ESC_DOLLAR:" ), wxT( "${" ) );
    text.Replace( wxT( "<<<ESC_AT:" ), wxT( "@{" ) );

    return text;
}


bool SCH_TEXT::HasHypertext() const
{
    return HasHyperlink() || containsURL();
}


bool SCH_TEXT::HasHoveredHypertext() const
{
    return !m_activeUrl.IsEmpty();
}




wxString SCH_TEXT::GetItemDescription( UNITS_PROVIDER* aUnitsProvider, bool aFull ) const
{
    return wxString::Format( _( "Graphic Text '%s'" ),
                             aFull ? GetShownText( false ) : GetText() );
}


BITMAPS SCH_TEXT::GetMenuImage() const
{
    return BITMAPS::text;
}


bool SCH_TEXT::HitTest( const VECTOR2I& aPosition, int aAccuracy ) const
{
    BOX2I bBox = GetBoundingBox();
    bBox.Inflate( aAccuracy );
    return bBox.Contains( aPosition );
}


bool SCH_TEXT::HitTest( const BOX2I& aRect, bool aContained, int aAccuracy ) const
{
    if( m_flags & ( STRUCT_DELETED | SKIP_STRUCT ) )
        return false;

    BOX2I rect = aRect;
    BOX2I bBox = GetBoundingBox();

    rect.Inflate( aAccuracy );

    if( aContained )
        return aRect.Contains( bBox );

    return aRect.Intersects( bBox );
}


bool SCH_TEXT::HitTest( const SHAPE_LINE_CHAIN& aPoly, bool aContained ) const
{
    if( m_flags & ( STRUCT_DELETED | SKIP_STRUCT ) )
        return false;

    return KIGEOM::BoxHitTest( aPoly, GetBoundingBox(), aContained );
}


void SCH_TEXT::BeginEdit( const VECTOR2I& aPosition )
{
    SetTextPos( aPosition );
}


void SCH_TEXT::CalcEdit( const VECTOR2I& aPosition )
{
    SetTextPos( aPosition );
}


std::vector<int> SCH_TEXT::ViewGetLayers() const
{
    if( IsPrivate() )
        return { LAYER_PRIVATE_NOTES, LAYER_SELECTION_SHADOWS };

    return { m_layer, LAYER_SELECTION_SHADOWS };
}


VECTOR2I SCH_TEXT::GetOffsetToMatchSCH_FIELD( SCH_RENDER_SETTINGS* aRenderSettings ) const
{
    if( GetDrawFont( aRenderSettings )->IsOutline() )
    {
        BOX2I    firstLineBBox = GetTextBox( aRenderSettings, 0 );
        int      sizeDiff = firstLineBBox.GetHeight() - GetTextSize().y;
        int      adjust = KiROUND( sizeDiff * 0.4 );
        VECTOR2I adjust_offset( 0, -adjust );

        RotatePoint( adjust_offset, GetDrawRotation() );
        return adjust_offset;
    }

    return { 0, 0 };
}






bool SCH_TEXT::operator==( const SCH_ITEM& aOther ) const
{
    if( Type() != aOther.Type() )
        return false;

    const SCH_TEXT* other = static_cast<const SCH_TEXT*>( &aOther );

    if( GetLayer() != other->GetLayer() )
        return false;

    if( GetExcludedFromSim() != other->GetExcludedFromSim() )
        return false;

    return EDA_TEXT::operator==( *other );
}


double SCH_TEXT::Similarity( const SCH_ITEM& aOther ) const
{
    if( m_Uuid == aOther.m_Uuid )
        return 1.0;

    if( Type() != aOther.Type() )
        return 0.0;

    const SCH_TEXT* other = static_cast<const SCH_TEXT*>( &aOther );

    double retval = SimilarityBase( aOther );

    if( GetLayer() != other->GetLayer() )
        retval *= 0.9;

    if( GetExcludedFromSim() != other->GetExcludedFromSim() )
        retval *= 0.9;

    retval *= EDA_TEXT::Similarity( *other );

    return retval;
}


int SCH_TEXT::compare( const SCH_ITEM& aOther, int aCompareFlags ) const
{
    wxASSERT( aOther.Type() == SCH_TEXT_T );

    int retv = SCH_ITEM::compare( aOther, aCompareFlags );

    if( retv )
        return retv;

    const SCH_TEXT& tmp = static_cast<const SCH_TEXT&>( aOther );

    int result = GetText().CmpNoCase( tmp.GetText() );

    if( result != 0 )
        return result;

    if( GetTextPos().x != tmp.GetTextPos().x )
        return GetTextPos().x - tmp.GetTextPos().x;

    if( GetTextPos().y != tmp.GetTextPos().y )
        return GetTextPos().y - tmp.GetTextPos().y;

    if( GetTextWidth() != tmp.GetTextWidth() )
        return GetTextWidth() - tmp.GetTextWidth();

    if( GetTextHeight() != tmp.GetTextHeight() )
        return GetTextHeight() - tmp.GetTextHeight();

    return 0;
}


#if defined( DEBUG )

void SCH_TEXT::Show( int nestLevel, std::ostream& os ) const
{
    // XML output:
    wxString s = GetClass();

    NestedSpace( nestLevel, os ) << '<' << s.Lower().mb_str() << " layer=\"" << m_layer << '"' << '>'
                                 << TO_UTF8( GetText() ) << "</" << s.Lower().mb_str() << ">\n";
}

#endif


static struct SCH_TEXT_DESC
{
    SCH_TEXT_DESC()
    {
        PROPERTY_MANAGER& propMgr = PROPERTY_MANAGER::Instance();
        REGISTER_TYPE( SCH_TEXT );
        propMgr.AddTypeCast( new TYPE_CAST<SCH_TEXT, SCH_ITEM> );
        propMgr.AddTypeCast( new TYPE_CAST<SCH_TEXT, EDA_TEXT> );
        propMgr.InheritsAfter( TYPE_HASH( SCH_TEXT ), TYPE_HASH( SCH_ITEM ) );
        propMgr.InheritsAfter( TYPE_HASH( SCH_TEXT ), TYPE_HASH( EDA_TEXT ) );

        propMgr.Mask( TYPE_HASH( SCH_TEXT ), TYPE_HASH( EDA_TEXT ), _HKI( "Mirrored" ) );
        propMgr.Mask( TYPE_HASH( SCH_TEXT ), TYPE_HASH( EDA_TEXT ), _HKI( "Width" ) );
        propMgr.Mask( TYPE_HASH( SCH_TEXT ), TYPE_HASH( EDA_TEXT ), _HKI( "Height" ) );
        propMgr.Mask( TYPE_HASH( SCH_TEXT ), TYPE_HASH( EDA_TEXT ), _HKI( "Thickness" ) );

        propMgr.AddProperty( new PROPERTY<SCH_TEXT, int>( _HKI( "Text Size" ), &SCH_TEXT::SetSchTextSize,
                                                          &SCH_TEXT::GetSchTextSize, PROPERTY_DISPLAY::PT_SIZE ),
                             _HKI( "Text Properties" ) );

        // Orientation is exposed differently in schematic; mask the base for now
        propMgr.Mask( TYPE_HASH( SCH_TEXT ), TYPE_HASH( EDA_TEXT ), _HKI( "Orientation" ) );
    }
} _SCH_TEXT_DESC;
