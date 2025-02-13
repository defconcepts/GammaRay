/*
  kjobtracker.cpp

  This file is part of GammaRay, the Qt application inspection and
  manipulation tool.

  Copyright (C) 2012-2015 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
  Author: Volker Krause <volker.krause@kdab.com>

  Licensees holding valid commercial KDAB GammaRay licenses may use this file in
  accordance with GammaRay Commercial License Agreement provided with the Software.

  Contact info@kdab.com if any conditions of this licensing are not clear to you.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "kjobtracker.h"
#include "kjobmodel.h"

#include <core/probeinterface.h>

#include <QDebug>
#include <QtPlugin>

using namespace GammaRay;

KJobTracker::KJobTracker(ProbeInterface *probe, QObject *parent)
  : QObject(parent), m_jobModel(new KJobModel(this))
{
  connect(probe->probe(), SIGNAL(objectCreated(QObject*)),
          m_jobModel, SLOT(objectAdded(QObject*)));
  connect(probe->probe(), SIGNAL(objectDestroyed(QObject*)),
          m_jobModel, SLOT(objectRemoved(QObject*)));

  probe->registerModel(QStringLiteral("com.kdab.GammaRay.KJobModel"), m_jobModel);
}

KJobTracker::~KJobTracker()
{
}

QString KJobTrackerFactory::name() const
{
  return tr("KJobs");
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
Q_EXPORT_PLUGIN(KJobTrackerFactory)
#endif
