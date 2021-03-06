/*
  Copyright (c) 2009 Sebastian Sauer <sebsauer@kdab.net>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "editorconfig.h"
#include "korganizereditorconfig.h"

#include <QtCore/QCoreApplication>

using namespace IncidenceEditorNG;

class EditorConfig::Private
{
  public:
    static EditorConfig *config;
    static void cleanup_config()
    {
      delete config;
      config = 0;
    }

    QHash<KCalCore::IncidenceBase::IncidenceType, QStringList> mTemplates;
};

EditorConfig *EditorConfig::Private::config = 0;

EditorConfig::EditorConfig()
  : d( new Private )
{
}

EditorConfig::~EditorConfig()
{
  delete d;
}

EditorConfig *EditorConfig::instance()
{
  if ( !Private::config ) {
    // No one called setEditorConfig(), so we default to a KorganizerEditorConfig.
    EditorConfig::setEditorConfig( new IncidenceEditorNG::KOrganizerEditorConfig );
  }

  return Private::config;
}

void EditorConfig::setEditorConfig( EditorConfig *config )
{
  delete Private::config;
  Private::config = config;
  qAddPostRoutine( Private::cleanup_config );
}

QString EditorConfig::fullName() const
{
  if ( Private::config != this ) {
    return Private::config->fullName();
  }
  return QString();
}

QString EditorConfig::email() const
{
  if ( Private::config != this ) {
    return Private::config->email();
  }
  return QString();
}

bool EditorConfig::thatIsMe( const QString &mail ) const
{
  if ( Private::config != this ) {
    return Private::config->thatIsMe( mail );
  }
  return false;
}

QStringList EditorConfig::allEmails() const
{
  if ( Private::config != this ) {
    return Private::config->allEmails();
  }

  QStringList mails;
  const QString m = email();
  if ( !m.isEmpty() ) {
    mails << m;
  }
  return mails;
}

QStringList EditorConfig::fullEmails() const
{
  if ( Private::config != this ) {
    return Private::config->fullEmails();
  }
  return QStringList();
}

bool EditorConfig::showTimeZoneSelectorInIncidenceEditor() const
{
  if ( Private::config != this ) {
    return Private::config->showTimeZoneSelectorInIncidenceEditor();
  }
  return true;
}

QStringList &EditorConfig::templates( KCalCore::IncidenceBase::IncidenceType type )
{
  return d->mTemplates[type];
}
