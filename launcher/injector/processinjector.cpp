/*
  processinjector.cpp

  This file is part of GammaRay, the Qt application inspection and
  manipulation tool.

  Copyright (C) 2013-2015 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
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


#include "processinjector.h"

#include <QDebug>

using namespace GammaRay;

ProcessInjector::ProcessInjector() :
  mExitCode(-1),
  mProcessError(QProcess::UnknownError),
  mExitStatus(QProcess::NormalExit)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
    m_proc.setInputChannelMode(QProcess::ForwardedInputChannel);
#endif
    connect(&m_proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processFailed()));
    connect(&m_proc, SIGNAL(finished(int)), this, SLOT(processFinished()));
}

ProcessInjector::~ProcessInjector()
{
}

void ProcessInjector::stop()
{
    m_proc.terminate();
    if (!m_proc.waitForFinished(1000))
        m_proc.kill(); // kill it softly
}

bool ProcessInjector::launchProcess(const QStringList& programAndArgs, const QProcessEnvironment& env)
{
  m_proc.setProcessEnvironment(env);
  m_proc.setProcessChannelMode(QProcess::ForwardedChannels);

  QStringList args = programAndArgs;

  if (!env.value(QStringLiteral("GAMMARAY_TARGET_WRAPPER")).isEmpty()) {
    const QString fullWrapperCmd = env.value(QStringLiteral("GAMMARAY_TARGET_WRAPPER"));
    // ### TODO properly handle quoted arguments!
    QStringList newArgs = fullWrapperCmd.split(' ');
    newArgs += args;
    args = newArgs;
    qDebug() << "Launching with target wrapper:" << args;
  } else if (env.value(QStringLiteral("GAMMARAY_GDB")).toInt()) {
    QStringList newArgs;
    newArgs << QStringLiteral("gdb");
#ifndef Q_OS_MAC
    newArgs << QStringLiteral("--eval-command") << QStringLiteral("run");
#endif
    newArgs << QStringLiteral("--args");
    newArgs += args;
    args = newArgs;
  }

  const QString program = args.takeFirst();
  m_proc.start(program, args);

  bool ret = m_proc.waitForStarted(-1);
  if (ret)
    emit started();

  return ret;
}

void ProcessInjector::processFailed()
{
    mProcessError = m_proc.error();
    mErrorString = m_proc.errorString();
}

void ProcessInjector::processFinished()
{
    mExitCode = m_proc.exitCode();
    mExitStatus = m_proc.exitStatus();

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    if (mProcessError == QProcess::FailedToStart) {
      mErrorString.prepend(QStringLiteral("Could not start '%1': ").arg(m_proc.program()));
    }
#endif
    emit finished();
}

int ProcessInjector::exitCode()
{
  return mExitCode;
}

QProcess::ExitStatus ProcessInjector::exitStatus()
{
  return mExitStatus;
}

QProcess::ProcessError ProcessInjector::processError()
{
  return mProcessError;
}

QString ProcessInjector::errorString()
{
  return mErrorString;
}
