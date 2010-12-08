/****************************************************************************************
 * Copyright (c) 2004 Mark Kretschmann <kretschmann@kde.org>                            *
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

#ifndef AMAROK_PLUGINMANAGER_H
#define AMAROK_PLUGINMANAGER_H

#include <KPluginInfo>
#include <KService>

#include "shared/amarok_export.h"

class DeviceHandlerFactory;
class MountPointManager;
class ServicePluginManager;
class ServiceFactory;
namespace Collections {
    class CollectionFactory;
}

namespace Plugins {

class AMAROK_EXPORT PluginManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY( int pluginFrameworkVersion READ pluginFrameworkVersion )

    public:
        ~PluginManager();

        static PluginManager *instance();
        static void destroy();
        static int pluginFrameworkVersion();

        /**
         * Load any services that are configured to be loaded
         */
        void init();

        void checkPluginEnabledStates();

        KPluginInfo::List plugins( const QString &category );

        QList<ServiceFactory*> serviceFactories() const;

        ServicePluginManager *servicePluginManager();

        QList<DeviceHandlerFactory*> deviceFactories() const;

        QList<Collections::CollectionFactory*> collectionFactories() const;

    private:
        void findAllPlugins();
        void handleEmptyCollectionFactories();

        template<typename T>
            QList<T*> createFactories( const QString &category );

        template<typename T>
            T* createFactory( const KPluginInfo &plugin );

        MountPointManager *m_mountPointManager;
        ServicePluginManager *m_servicePluginManager;
        QList<ServiceFactory*> m_serviceFactories;
        QList<DeviceHandlerFactory*> m_deviceFactories;
        QList<Collections::CollectionFactory*> m_collectionFactories;
        QHash<QString, KPluginInfo::List> m_pluginInfos;
        QHash<QString, bool> m_factoryCreated;

        static const int s_pluginFrameworkVersion;
        static PluginManager *s_instance;

        PluginManager( QObject *parent = 0 );
};

} // namespace Plugins

namespace The
{
    inline Plugins::PluginManager *pluginManager() { return Plugins::PluginManager::instance(); }
}

#endif /* AMAROK_PLUGINMANAGER_H */
