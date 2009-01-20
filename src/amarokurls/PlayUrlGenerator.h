/***************************************************************************
*   Copyright (c) 2009  Casey Link <unnamedrambler@gmail.com>             *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
***************************************************************************/

#ifndef PLAYURLGENERATOR_H
#define PLAYURLGENERATOR_H

#include "amarok_export.h"
#include "Meta.h"

class AmarokUrl;

/**
 *
 * A class used to generate play urls.
 *
 * @author Casey Link <unnaemdrambler@gmail.com>
 */
class AMAROK_EXPORT PlayUrlGenerator {
    public:
        PlayUrlGenerator();
        ~PlayUrlGenerator();

        AmarokUrl CreateCurrentTrackBookmark();
};
#endif // PLAYURLGENERATOR_H
