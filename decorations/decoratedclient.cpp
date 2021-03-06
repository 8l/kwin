/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2014 Martin Gräßlin <mgraesslin@kde.org>

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
#include "decoratedclient.h"
#include "decorationpalette.h"
#include "decorationrenderer.h"
#include "client.h"
#include "composite.h"
#include "cursor.h"
#include "options.h"
#include "scene.h"
#include "workspace.h"

#include <KDecoration2/DecoratedClient>
#include <KDecoration2/Decoration>

#include <QDebug>

namespace KWin
{
namespace Decoration
{

DecoratedClientImpl::DecoratedClientImpl(Client *client, KDecoration2::DecoratedClient *decoratedClient, KDecoration2::Decoration *decoration)
    : QObject()
    , DecoratedClientPrivate(decoratedClient, decoration)
    , m_client(client)
    , m_renderer(nullptr)
{
    createRenderer();
    client->setDecoratedClient(QPointer<DecoratedClientImpl>(this));
    connect(client, &Client::activeChanged, this,
        [decoratedClient, client]() {
            emit decoratedClient->activeChanged(client->isActive());
        }
    );
    connect(client, &Client::geometryChanged, this,
        [decoratedClient, client]() {
            emit decoratedClient->widthChanged(client->clientSize().width());
            emit decoratedClient->heightChanged(client->clientSize().height());
        }
    );
    connect(client, &Client::desktopChanged, this,
        [decoratedClient, client]() {
            emit decoratedClient->onAllDesktopsChanged(client->isOnAllDesktops());
        }
    );
    connect(client, &Client::captionChanged, this,
        [decoratedClient, client]() {
            emit decoratedClient->captionChanged(client->caption());
        }
    );
    connect(client, &Client::iconChanged, this,
        [decoratedClient, client]() {
            emit decoratedClient->iconChanged(client->icon());
        }
    );
    connect(client, &Client::shadeChanged, this,
        [decoratedClient, client]() {
            // TODO: geometry is wrong
            emit decoratedClient->shadedChanged(client->isShade());
        }
    );
    connect(client, &Client::keepAboveChanged, decoratedClient, &KDecoration2::DecoratedClient::keepAboveChanged);
    connect(client, &Client::keepBelowChanged, decoratedClient, &KDecoration2::DecoratedClient::keepBelowChanged);
    connect(Compositor::self(), &Compositor::compositingToggled, this,
        [this, decoration]() {
            delete m_renderer;
            m_renderer = nullptr;
            createRenderer();
            decoration->update();
        }
    );
    connect(client, &Client::quickTileModeChanged, decoratedClient,
        [this, decoratedClient]() {
            emit decoratedClient->adjacentScreenEdgesChanged(adjacentScreenEdges());
        }
    );
    connect(client, &Client::closeableChanged, decoratedClient, &KDecoration2::DecoratedClient::closeableChanged);
    connect(client, &Client::shadeableChanged, decoratedClient, &KDecoration2::DecoratedClient::shadeableChanged);
    connect(client, &Client::minimizeableChanged, decoratedClient, &KDecoration2::DecoratedClient::minimizeableChanged);
    connect(client, &Client::maximizeableChanged, decoratedClient, &KDecoration2::DecoratedClient::maximizeableChanged);

    connect(client, &Client::paletteChanged, decoratedClient, &KDecoration2::DecoratedClient::paletteChanged);
}

DecoratedClientImpl::~DecoratedClientImpl() = default;

#define DELEGATE(type, name, clientName) \
    type DecoratedClientImpl::name() const \
    { \
        return m_client->clientName(); \
    }

#define DELEGATE2(type, name) DELEGATE(type, name, name)

DELEGATE2(QString, caption)
DELEGATE2(bool, isActive)
DELEGATE2(bool, isCloseable)
DELEGATE(bool, isMaximizeable, isMaximizable)
DELEGATE(bool, isMinimizeable, isMinimizable)
DELEGATE2(bool, isModal)
DELEGATE(bool, isMoveable, isMovable)
DELEGATE(bool, isResizeable, isResizable)
DELEGATE2(bool, isShadeable)
DELEGATE2(bool, providesContextHelp)
DELEGATE2(int, desktop)
DELEGATE2(bool, isOnAllDesktops)
DELEGATE2(QPalette, palette)
DELEGATE2(QIcon, icon)

#undef DELEGATE2
#undef DELEGATE

#define DELEGATE(type, name, clientName) \
    type DecoratedClientImpl::name() const \
    { \
        return m_client->clientName(); \
    }

DELEGATE(bool, isKeepAbove, keepAbove)
DELEGATE(bool, isKeepBelow, keepBelow)
DELEGATE(bool, isShaded, isShade)
DELEGATE(WId, windowId, window)
DELEGATE(WId, decorationId, frameId)

#undef DELEGATE

#define DELEGATE(name, op) \
    void DecoratedClientImpl::name() \
    { \
        Workspace::self()->performWindowOperation(m_client, Options::op); \
    }

DELEGATE(requestToggleShade, ShadeOp)
DELEGATE(requestToggleOnAllDesktops, OnAllDesktopsOp)
DELEGATE(requestToggleKeepAbove, KeepAboveOp)
DELEGATE(requestToggleKeepBelow, KeepBelowOp)

#undef DELEGATE

#define DELEGATE(name, clientName) \
    void DecoratedClientImpl::name() \
    { \
        m_client->clientName(); \
    }

DELEGATE(requestContextHelp, showContextHelp)
DELEGATE(requestMinimize, minimize)
DELEGATE(requestClose, closeWindow)

#undef DELEGATE

QColor DecoratedClientImpl::color(KDecoration2::ColorGroup group, KDecoration2::ColorRole role) const
{
    auto dp = m_client->decorationPalette();
    if (dp) {
        return dp->color(group, role);
    }

    return QColor();
}

void DecoratedClientImpl::requestShowWindowMenu()
{
    // TODO: add rect to requestShowWindowMenu
    Workspace::self()->showWindowMenu(QRect(Cursor::pos(), Cursor::pos()), m_client);
}

void DecoratedClientImpl::requestToggleMaximization(Qt::MouseButtons buttons)
{
    Workspace::self()->performWindowOperation(m_client, options->operationMaxButtonClick(buttons));
}

int DecoratedClientImpl::width() const
{
    return m_client->clientSize().width();
}

int DecoratedClientImpl::height() const
{
    return m_client->clientSize().height();
}

bool DecoratedClientImpl::isMaximizedVertically() const
{
    return m_client->maximizeMode() & MaximizeVertical;
}

bool DecoratedClientImpl::isMaximized() const
{
    return isMaximizedHorizontally() && isMaximizedVertically();
}

bool DecoratedClientImpl::isMaximizedHorizontally() const
{
    return m_client->maximizeMode() & MaximizeHorizontal;
}

Qt::Edges DecoratedClientImpl::adjacentScreenEdges() const
{
    Qt::Edges edges;
    const Client::QuickTileMode mode = m_client->quickTileMode();
    if (mode.testFlag(Client::QuickTileLeft)) {
        edges |= Qt::LeftEdge;
        if (!mode.testFlag(Client::QuickTileTop) && !mode.testFlag(Client::QuickTileBottom)) {
            // using complete side
            edges |= Qt::TopEdge | Qt::BottomEdge;
        }
    }
    if (mode.testFlag(Client::QuickTileTop)) {
        edges |= Qt::TopEdge;
    }
    if (mode.testFlag(Client::QuickTileRight)) {
        edges |= Qt::RightEdge;
        if (!mode.testFlag(Client::QuickTileTop) && !mode.testFlag(Client::QuickTileBottom)) {
            // using complete side
            edges |= Qt::TopEdge | Qt::BottomEdge;
        }
    }
    if (mode.testFlag(Client::QuickTileBottom)) {
        edges |= Qt::BottomEdge;
    }
    return edges;
}

void DecoratedClientImpl::createRenderer()
{
    if (Compositor::self()->hasScene()) {
        m_renderer = Compositor::self()->scene()->createDecorationRenderer(this);
    } else {
        m_renderer = new X11Renderer(this);
    }
}

void DecoratedClientImpl::destroyRenderer()
{
    delete m_renderer;
    m_renderer = nullptr;
}

}
}
