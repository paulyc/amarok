/****************************************************************************************
 * Copyright (c) 2011 Sergey Ivanov <123kash@gmail.com>                                 *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/


#include "ClearSpinBox.h"

ClearSpinBox::ClearSpinBox( QWidget* parent )
            : QSpinBox( parent )
{
}

QValidator::State
ClearSpinBox::validate( QString &input, int &pos ) const
{
    return input.isEmpty() ? QValidator::Acceptable : QSpinBox::validate( input, pos );
}

int
ClearSpinBox::valueFromText( const QString &text ) const
{
    return text.isEmpty() ? minimum() : QSpinBox::valueFromText( text );
}

QString
ClearSpinBox::textFromValue( int val ) const
{
    return ( val == minimum() ) ? QString() : QSpinBox::textFromValue( val );
}

