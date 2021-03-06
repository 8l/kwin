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
#ifndef KWIN_ABSTRACT_CLIENT_H
#define KWIN_ABSTRACT_CLIENT_H

#include "toplevel.h"
#include "options.h"
#include "rules.h"

#include <memory>

namespace KWin
{

class TabGroup;

namespace TabBox
{
class TabBoxClientImpl;
}

namespace Decoration
{
class DecorationPalette;
}

class AbstractClient : public Toplevel
{
    Q_OBJECT
    /**
     * Whether this Client is the currently visible Client in its Client Group (Window Tabs).
     * For change connect to the visibleChanged signal on the Client's Group.
     **/
    Q_PROPERTY(bool isCurrentTab READ isCurrentTab)
    /**
     * Whether this Client is active or not. Use Workspace::activateClient() to activate a Client.
     * @see Workspace::activateClient
     **/
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)
    /**
     * The desktop this Client is on. If the Client is on all desktops the property has value -1.
     **/
    Q_PROPERTY(int desktop READ desktop WRITE setDesktop NOTIFY desktopChanged)
    /**
     * Whether the Client is on all desktops. That is desktop is -1.
     **/
    Q_PROPERTY(bool onAllDesktops READ isOnAllDesktops WRITE setOnAllDesktops NOTIFY desktopChanged)
    /**
     * Whether the Client should be excluded from window switching effects.
     **/
    Q_PROPERTY(bool skipSwitcher READ skipSwitcher WRITE setSkipSwitcher NOTIFY skipSwitcherChanged)
    /**
     * Whether the window can be closed by the user. The value is evaluated each time the getter is called.
     * Because of that no changed signal is provided.
     **/
    Q_PROPERTY(bool closeable READ isCloseable)
    Q_PROPERTY(QIcon icon READ icon NOTIFY iconChanged)
    /**
     * Whether the Client is set to be kept above other windows.
     **/
    Q_PROPERTY(bool keepAbove READ keepAbove WRITE setKeepAbove NOTIFY keepAboveChanged)
    /**
     * Whether the Client is set to be kept below other windows.
     **/
    Q_PROPERTY(bool keepBelow READ keepBelow WRITE setKeepBelow NOTIFY keepBelowChanged)
    /**
     * Whether the Client can be shaded. The property is evaluated each time it is invoked.
     * Because of that there is no notify signal.
     **/
    Q_PROPERTY(bool shadeable READ isShadeable)
    /**
     * Whether the Client is shaded.
     **/
    Q_PROPERTY(bool shade READ isShade WRITE setShade NOTIFY shadeChanged)
    /**
     * Whether the Client can be minimized. The property is evaluated each time it is invoked.
     * Because of that there is no notify signal.
     **/
    Q_PROPERTY(bool minimizable READ isMinimizable)
    /**
     * Whether the Client is minimized.
     **/
    Q_PROPERTY(bool minimized READ isMinimized WRITE setMinimized NOTIFY minimizedChanged)
    /**
     * Returns whether the window is any of special windows types (desktop, dock, splash, ...),
     * i.e. window types that usually don't have a window frame and the user does not use window
     * management (moving, raising,...) on them.
     * The value is evaluated each time the getter is called.
     * Because of that no changed signal is provided.
     **/
    Q_PROPERTY(bool specialWindow READ isSpecialWindow)
    /**
     * Whether window state _NET_WM_STATE_DEMANDS_ATTENTION is set. This state indicates that some
     * action in or with the window happened. For example, it may be set by the Window Manager if
     * the window requested activation but the Window Manager refused it, or the application may set
     * it if it finished some work. This state may be set by both the Client and the Window Manager.
     * It should be unset by the Window Manager when it decides the window got the required attention
     * (usually, that it got activated).
     **/
    Q_PROPERTY(bool demandsAttention READ isDemandingAttention WRITE demandAttention NOTIFY demandsAttentionChanged)
public:
    virtual ~AbstractClient();

    QWeakPointer<TabBox::TabBoxClientImpl> tabBoxClient() const {
        return m_tabBoxClient.toWeakRef();
    }
    bool isFirstInTabBox() const {
        return m_firstInTabBox;
    }
    bool skipSwitcher() const {
        return m_skipSwitcher;
    }
    void setSkipSwitcher(bool set);

    const QIcon &icon() const {
        return m_icon;
    }

    bool isActive() const {
        return m_active;
    }
    /**
     * Sets the client's active state to \a act.
     *
     * This function does only change the visual appearance of the client,
     * it does not change the focus setting. Use
     * Workspace::activateClient() or Workspace::requestFocus() instead.
     *
     * If a client receives or looses the focus, it calls setActive() on
     * its own.
     **/
    void setActive(bool);

    bool keepAbove() const {
        return m_keepAbove;
    }
    void setKeepAbove(bool);
    bool keepBelow() const {
        return m_keepBelow;
    }
    void setKeepBelow(bool);

    void demandAttention(bool set = true);
    bool isDemandingAttention() const {
        return m_demandsAttention;
    }

    void cancelAutoRaise();

    bool wantsTabFocus() const;

    virtual void updateMouseGrab();
    virtual QString caption(bool full = true, bool stripped = false) const = 0;
    virtual bool isCloseable() const = 0;
    // TODO: remove boolean trap
    virtual bool isShown(bool shaded_is_shown) const = 0;
    virtual bool isFullScreen() const = 0;
    // TODO: remove boolean trap
    virtual AbstractClient *findModal(bool allow_itself = false) = 0;
    virtual bool isTransient() const;
    /**
     * Returns true for "special" windows and false for windows which are "normal"
     * (normal=window which has a border, can be moved by the user, can be closed, etc.)
     * true for Desktop, Dock, Splash, Override and TopMenu (and Toolbar??? - for now)
     * false for Normal, Dialog, Utility and Menu (and Toolbar??? - not yet) TODO
     */
    bool isSpecialWindow() const;
    virtual void sendToScreen(int screen) = 0;
    virtual const QKeySequence &shortcut() const  = 0;
    virtual void setShortcut(const QString &cut) = 0;
    virtual bool performMouseCommand(Options::MouseCommand, const QPoint &globalPos) = 0;
    void setOnAllDesktops(bool set);
    void setDesktop(int);
    int desktop() const override {
        return m_desktop;
    }
    void setMinimized(bool set);
    /**
    * Minimizes this client plus its transients
    */
    void minimize(bool avoid_animation = false);
    void unminimize(bool avoid_animation = false);
    bool isMinimized() const {
        return m_minimized;
    }
    virtual void setFullScreen(bool set, bool user = true) = 0;
    virtual TabGroup *tabGroup() const;
    Q_INVOKABLE virtual bool untab(const QRect &toGeometry = QRect(), bool clientRemoved = false);
    virtual bool isCurrentTab() const;
    virtual MaximizeMode maximizeMode() const = 0;
    virtual void maximize(MaximizeMode) = 0;
    virtual bool noBorder() const = 0;
    virtual void setNoBorder(bool set) = 0;
    virtual void blockActivityUpdates(bool b = true) = 0;
    QPalette palette() const;
    const Decoration::DecorationPalette *decorationPalette() const;
    virtual bool isResizable() const = 0;
    virtual bool isMovable() const = 0;
    virtual bool isMovableAcrossScreens() const = 0;
    /**
     * @c true only for @c ShadeNormal
     **/
    bool isShade() const {
        return shadeMode() == ShadeNormal;
    }
    /**
     * Default implementation returns @c ShadeNone
     **/
    virtual ShadeMode shadeMode() const; // Prefer isShade()
    void setShade(bool set);
    /**
     * Default implementation does nothing
     **/
    virtual void setShade(ShadeMode mode);
    /**
     * Whether the Client can be shaded. Default implementation returns @c false.
     **/
    virtual bool isShadeable() const;
    virtual bool isMaximizable() const = 0;
    virtual bool isMinimizable() const = 0;
    virtual bool userCanSetFullScreen() const = 0;
    virtual bool userCanSetNoBorder() const = 0;
    virtual void setOnAllActivities(bool set) = 0;
    virtual const WindowRules* rules() const = 0;
    virtual void takeFocus() = 0;
    virtual bool wantsInput() const = 0;
    virtual void checkWorkspacePosition(QRect oldGeometry = QRect(), int oldDesktop = -2) = 0;
    virtual xcb_timestamp_t userTime() const;
    virtual void updateWindowRules(Rules::Types selection) = 0;

    virtual void growHorizontal();
    virtual void shrinkHorizontal();
    virtual void growVertical();
    virtual void shrinkVertical();

    /**
     * These values represent positions inside an area
     */
    enum Position {
        // without prefix, they'd conflict with Qt::TopLeftCorner etc. :(
        PositionCenter         = 0x00,
        PositionLeft           = 0x01,
        PositionRight          = 0x02,
        PositionTop            = 0x04,
        PositionBottom         = 0x08,
        PositionTopLeft        = PositionLeft | PositionTop,
        PositionTopRight       = PositionRight | PositionTop,
        PositionBottomLeft     = PositionLeft | PositionBottom,
        PositionBottomRight    = PositionRight | PositionBottom
    };
    Position titlebarPosition() const;

    // a helper for the workspace window packing. tests for screen validity and updates since in maximization case as with normal moving
    virtual void packTo(int left, int top);

    enum QuickTileFlag {
        QuickTileNone = 0,
        QuickTileLeft = 1,
        QuickTileRight = 1<<1,
        QuickTileTop = 1<<2,
        QuickTileBottom = 1<<3,
        QuickTileHorizontal = QuickTileLeft|QuickTileRight,
        QuickTileVertical = QuickTileTop|QuickTileBottom,
        QuickTileMaximize = QuickTileLeft|QuickTileRight|QuickTileTop|QuickTileBottom
    };
    Q_DECLARE_FLAGS(QuickTileMode, QuickTileFlag)
    /** Set the quick tile mode ("snap") of this window.
     * This will also handle preserving and restoring of window geometry as necessary.
     * @param mode The tile mode (left/right) to give this window.
     */
    virtual void setQuickTileMode(QuickTileMode mode, bool keyboard = false) = 0;
    virtual void updateLayer();

    // TODO: remove boolean trap
    static bool belongToSameApplication(const AbstractClient* c1, const AbstractClient* c2, bool active_hack = false);

public Q_SLOTS:
    virtual void closeWindow() = 0;

Q_SIGNALS:
    void skipSwitcherChanged();
    void iconChanged();
    void activeChanged();
    void keepAboveChanged(bool);
    void keepBelowChanged(bool);
    /**
     * Emitted whenever the demands attention state changes.
     **/
    void demandsAttentionChanged();
    void desktopPresenceChanged(KWin::AbstractClient*, int); // to be forwarded by Workspace
    void desktopChanged();
    void shadeChanged();
    void minimizedChanged();
    void clientMinimized(KWin::AbstractClient* client, bool animate);
    void clientUnminimized(KWin::AbstractClient* client, bool animate);
    void paletteChanged(const QPalette &p);

protected:
    AbstractClient();
    void setFirstInTabBox(bool enable) {
        m_firstInTabBox = enable;
    }
    void setIcon(const QIcon &icon);
    void startAutoRaise();
    void autoRaise();
    /**
     * Called from ::setActive once the active value got updated, but before the changed signal
     * is emitted.
     *
     * Default implementation does nothing.
     **/
    virtual void doSetActive();
    /**
     * Called from ::setKeepAbove once the keepBelow value got updated, but before the changed signal
     * is emitted.
     *
     * Default implementation does nothing.
     **/
    virtual void doSetKeepAbove();
    /**
     * Called from ::setKeepBelow once the keepBelow value got updated, but before the changed signal
     * is emitted.
     *
     * Default implementation does nothing.
     **/
    virtual void doSetKeepBelow();
    /**
     * Called from ::setDeskop once the desktop value got updated, but before the changed signal
     * is emitted.
     *
     * Default implementation does nothing.
     * @param desktop The new desktop the Client is on
     * @param was_desk The desktop the Client was on before
     **/
    virtual void doSetDesktop(int desktop, int was_desk);
    /**
     * Called from ::minimize and ::unminimize once the minimized value got updated, but before the
     * changed signal is emitted.
     *
     * Default implementation does nothig.
     **/
    virtual void doMinimize();
    // TODO: remove boolean trap
    virtual bool belongsToSameApplication(const AbstractClient *other, bool active_hack) const = 0;

    void updateColorScheme(QString path);

private:
    void handlePaletteChange();
    QSharedPointer<TabBox::TabBoxClientImpl> m_tabBoxClient;
    bool m_firstInTabBox = false;
    bool m_skipSwitcher = false;
    QIcon m_icon;
    bool m_active = false;
    bool m_keepAbove = false;
    bool m_keepBelow = false;
    bool m_demandsAttention = false;
    bool m_minimized = false;
    QTimer *m_autoRaiseTimer = nullptr;
    int m_desktop = 0; // 0 means not on any desktop yet

    QString m_colorScheme;
    std::shared_ptr<Decoration::DecorationPalette> m_palette;
    static QHash<QString, std::weak_ptr<Decoration::DecorationPalette>> s_palettes;
    static std::shared_ptr<Decoration::DecorationPalette> s_defaultPalette;
};

}

Q_DECLARE_METATYPE(KWin::AbstractClient*)
Q_DECLARE_METATYPE(QList<KWin::AbstractClient*>)
Q_DECLARE_OPERATORS_FOR_FLAGS(KWin::AbstractClient::QuickTileMode)

#endif
