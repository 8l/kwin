/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2015 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#include "virtual_terminal.h"
// kwin
#include "logind.h"
#include "main.h"
#include "utils.h"
// Qt
#include <QDebug>
#include <QSocketNotifier>
// linux
#include <linux/major.h>
#include <linux/kd.h>
#include <linux/vt.h>
// system
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/signalfd.h>
#include <sys/stat.h>

#define RELEASE_SIGNAL SIGUSR1
#define ACQUISITION_SIGNAL SIGUSR2

namespace KWin
{

KWIN_SINGLETON_FACTORY(VirtualTerminal)

VirtualTerminal::VirtualTerminal(QObject *parent)
    : QObject(parent)
{
}

void VirtualTerminal::init()
{
    auto logind = LogindIntegration::self();
    if (logind->vt() != -1) {
        setup(logind->vt());
    }
    connect(logind, &LogindIntegration::virtualTerminalChanged, this, &VirtualTerminal::setup);
    if (logind->isConnected()) {
        logind->takeControl();
    } else {
        connect(logind, &LogindIntegration::connectedChanged, logind, &LogindIntegration::takeControl);
    }
}

VirtualTerminal::~VirtualTerminal()
{
    s_self = nullptr;
    closeFd();
}

static bool isTty(int fd)
{
    if (fd < 0) {
        return false;
    }
    struct stat st;
    if (fstat(fd, &st) == -1) {
        return false;
    }
    if (major(st.st_rdev) != TTY_MAJOR || minor (st.st_rdev) <= 0 || minor(st.st_rdev) >= 64) {
        return false;
    }
    return true;
}

void VirtualTerminal::setup(int vtNr)
{
    if (m_vt != -1) {
        return;
    }
    if (vtNr == -1) {
        // error condition
        return;
    }
    QString ttyName = QStringLiteral("/dev/tty%1").arg(vtNr);

    m_vt = LogindIntegration::self()->takeDevice(ttyName.toUtf8().constData());
    if (m_vt < 0) {
        qCWarning(KWIN_CORE) << "Failed to open tty through logind, trying without";
    }
    m_vt = open(ttyName.toUtf8().constData(), O_RDWR|O_CLOEXEC|O_NONBLOCK);
    if (m_vt < 0) {
        qCWarning(KWIN_CORE) << "Failed to open tty" << vtNr;
        return;
    }
    if (!isTty(m_vt)) {
        qCWarning(KWIN_CORE) << vtNr << " is no tty";
        closeFd();
        return;
    }
    if (ioctl(m_vt, KDSETMODE, KD_GRAPHICS) < 0) {
        qCWarning(KWIN_CORE()) << "Failed to set tty " << vtNr << " in graphics mode";
        closeFd();
        return;
    }
    if (!createSignalHandler()) {
        qCWarning(KWIN_CORE) << "Failed to create signalfd";
        closeFd();
        return;
    }
    vt_mode mode = {
        VT_PROCESS,
        0,
        RELEASE_SIGNAL,
        ACQUISITION_SIGNAL,
        0
    };
    if (ioctl(m_vt, VT_SETMODE, &mode) < 0) {
        qCWarning(KWIN_CORE) << "Failed to take over virtual terminal";
        closeFd();
        return;
    }
    setActive(true);
    emit kwinApp()->virtualTerminalCreated();
}

void VirtualTerminal::closeFd()
{
    if (m_vt < 0) {
        return;
    }
    if (m_notifier) {
        const int sfd = m_notifier->socket();
        delete m_notifier;
        m_notifier = nullptr;
        close(sfd);
    }
    close(m_vt);
    m_vt = -1;
}

bool VirtualTerminal::createSignalHandler()
{
    if (m_notifier) {
        return false;
    }
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, RELEASE_SIGNAL);
    sigaddset(&mask, ACQUISITION_SIGNAL);
    pthread_sigmask(SIG_BLOCK, &mask, nullptr);

    const int fd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
    if (fd < 0) {
        return false;
    }
    m_notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated, this,
        [this] {
            if (m_vt < 0) {
                return;
            }
            while (true) {
                signalfd_siginfo sigInfo;
                if (read(m_notifier->socket(), &sigInfo, sizeof(sigInfo)) != sizeof(sigInfo)) {
                    break;
                }
                switch (sigInfo.ssi_signo) {
                case RELEASE_SIGNAL:
                    setActive(false);
                    ioctl(m_vt, VT_RELDISP, 1);
                    break;
                case ACQUISITION_SIGNAL:
                    ioctl(m_vt, VT_RELDISP, VT_ACKACQ);
                    setActive(true);
                    break;
                }
            }
        }
    );
    return true;
}

void VirtualTerminal::activate(int vt)
{
    if (m_vt < 0) {
        return;
    }
    ioctl(m_vt, VT_ACTIVATE, vt);
    setActive(false);
}

void VirtualTerminal::setActive(bool active)
{
    if (m_active == active) {
        return;
    }
    m_active = active;
    emit activeChanged(m_active);
}

}
