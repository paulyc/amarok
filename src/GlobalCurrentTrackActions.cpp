/****************************************************************************************
 * Copyright (c) 2009 Nikolaj Hald Nielsen <nhn@kde.org>                                *
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
 
#include "GlobalCurrentTrackActions.h"

#include <QAction>


namespace The
{
    static GlobalCurrentTrackActions* s_GlobalCurrentTrackActions_instance = nullptr;

    GlobalCurrentTrackActions* globalCurrentTrackActions()
    {
        if( !s_GlobalCurrentTrackActions_instance )
            s_GlobalCurrentTrackActions_instance = new GlobalCurrentTrackActions();

        return s_GlobalCurrentTrackActions_instance;
    }
}


GlobalCurrentTrackActions::GlobalCurrentTrackActions()
{}

GlobalCurrentTrackActions::~GlobalCurrentTrackActions()
{}

void GlobalCurrentTrackActions::addAction( QAction * action )
{
    m_actions.append( action );
    QObject::connect( action, &QObject::destroyed, this, [this, action]() { m_actions.removeAll( action ); } );
}

QList< QAction * > GlobalCurrentTrackActions::actions()
{
    return m_actions;
}


