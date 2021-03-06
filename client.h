/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 1999, 2000 Matthias Ettrich <ettrich@kde.org>
Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

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

#ifndef KWIN_CLIENT_H
#define KWIN_CLIENT_H

// kwin
#include "options.h"
#include "rules.h"
#include "tabgroup.h"
#include "abstract_client.h"
#include "xcbutils.h"
// Qt
#include <QElapsedTimer>
#include <QFlags>
#include <QPointer>
#include <QPixmap>
#include <QWindow>
// X
#include <xcb/sync.h>

// TODO: Cleanup the order of things in this .h file

class QTimer;
class KStartupInfoData;
class KStartupInfoId;

struct xcb_sync_alarm_notify_event_t;

namespace KDecoration2
{
class Decoration;
}

namespace KWin
{

namespace Decoration
{
class DecoratedClientImpl;
}


/**
 * @brief Defines Predicates on how to search for a Client.
 *
 * Used by Workspace::findClient.
 */
enum class Predicate {
    WindowMatch,
    WrapperIdMatch,
    FrameIdMatch,
    InputIdMatch
};

class Client
    : public AbstractClient
{
    Q_OBJECT
    /**
     * The Caption of the Client. Read from WM_NAME property together with a suffix for hostname and shortcut.
     * To read only the caption as provided by WM_NAME, use the getter with an additional @c false value.
     **/
    Q_PROPERTY(QString caption READ caption NOTIFY captionChanged)
    /**
     * Whether this Client is fullScreen. A Client might either be fullScreen due to the _NET_WM property
     * or through a legacy support hack. The fullScreen state can only be changed if the Client does not
     * use the legacy hack. To be sure whether the state changed, connect to the notify signal.
     **/
    Q_PROPERTY(bool fullScreen READ isFullScreen WRITE setFullScreen NOTIFY fullScreenChanged)
    /**
     * Whether the Client can be set to fullScreen. The property is evaluated each time it is invoked.
     * Because of that there is no notify signal.
     **/
    Q_PROPERTY(bool fullScreenable READ isFullScreenable)
    /**
     * The geometry of this Client. Be aware that depending on resize mode the geometryChanged signal
     * might be emitted at each resize step or only at the end of the resize operation.
     **/
    Q_PROPERTY(QRect geometry READ geometry WRITE setGeometry)
    /**
     * Whether the Client can be maximized both horizontally and vertically.
     * The property is evaluated each time it is invoked.
     * Because of that there is no notify signal.
     **/
    Q_PROPERTY(bool maximizable READ isMaximizable)
    /**
     * Whether the Client represents a modal window.
     **/
    Q_PROPERTY(bool modal READ isModal NOTIFY modalChanged)
    /**
     * Whether the Client is moveable. Even if it is not moveable, it might be possible to move
     * it to another screen. The property is evaluated each time it is invoked.
     * Because of that there is no notify signal.
     * @see moveableAcrossScreens
     **/
    Q_PROPERTY(bool moveable READ isMovable)
    /**
     * Whether the Client can be moved to another screen. The property is evaluated each time it is invoked.
     * Because of that there is no notify signal.
     * @see moveable
     **/
    Q_PROPERTY(bool moveableAcrossScreens READ isMovableAcrossScreens)
    /**
     * Whether the Client provides context help. Mostly needed by decorations to decide whether to
     * show the help button or not.
     **/
    Q_PROPERTY(bool providesContextHelp READ providesContextHelp CONSTANT)
    /**
     * Whether the Client can be resized. The property is evaluated each time it is invoked.
     * Because of that there is no notify signal.
     **/
    Q_PROPERTY(bool resizeable READ isResizable)
    /**
     * Whether the Client is a transient Window to another Window.
     * @see transientFor
     **/
    Q_PROPERTY(bool transient READ isTransient NOTIFY transientChanged)
    /**
     * The Client to which this Client is a transient if any.
     **/
    Q_PROPERTY(KWin::Client *transientFor READ transientFor NOTIFY transientChanged)
    /**
     * By how much the window wishes to grow/shrink at least. Usually QSize(1,1).
     * MAY BE DISOBEYED BY THE WM! It's only for information, do NOT rely on it at all.
     * The value is evaluated each time the getter is called.
     * Because of that no changed signal is provided.
     */
    Q_PROPERTY(QSize basicUnit READ basicUnit)
    /**
     * Whether the Client is currently being moved by the user.
     * Notify signal is emitted when the Client starts or ends move/resize mode.
     **/
    Q_PROPERTY(bool move READ isMove NOTIFY moveResizedChanged)
    /**
     * Whether the Client is currently being resized by the user.
     * Notify signal is emitted when the Client starts or ends move/resize mode.
     **/
    Q_PROPERTY(bool resize READ isResize NOTIFY moveResizedChanged)
    /**
     * The optional geometry representing the minimized Client in e.g a taskbar.
     * See _NET_WM_ICON_GEOMETRY at http://standards.freedesktop.org/wm-spec/wm-spec-latest.html .
     * The value is evaluated each time the getter is called.
     * Because of that no changed signal is provided.
     **/
    Q_PROPERTY(QRect iconGeometry READ iconGeometry)
    /**
     * Whether the Client can accept keyboard focus.
     * The value is evaluated each time the getter is called.
     * Because of that no changed signal is provided.
     **/
    Q_PROPERTY(bool wantsInput READ wantsInput)
    /**
     * Indicates that the window should not be included on a taskbar.
     **/
    Q_PROPERTY(bool skipTaskbar READ skipTaskbar WRITE setSkipTaskbar NOTIFY skipTaskbarChanged)
    /**
     * Indicates that the window should not be included on a Pager.
     **/
    Q_PROPERTY(bool skipPager READ skipPager WRITE setSkipPager NOTIFY skipPagerChanged)
    /**
     * The "Window Tabs" Group this Client belongs to.
     **/
    Q_PROPERTY(KWin::TabGroup* tabGroup READ tabGroup NOTIFY tabGroupChanged SCRIPTABLE false)
    /**
     * Minimum size as specified in WM_NORMAL_HINTS
     **/
    Q_PROPERTY(QSize minSize READ minSize)
    /**
     * Maximum size as specified in WM_NORMAL_HINTS
     **/
    Q_PROPERTY(QSize maxSize READ maxSize)
    /**
     * Whether the window has a decoration or not.
     * This property is not allowed to be set by applications themselves.
     * The decision whether a window has a border or not belongs to the window manager.
     * If this property gets abused by application developers, it will be removed again.
     **/
    Q_PROPERTY(bool noBorder READ noBorder WRITE setNoBorder)
    /**
     * A client can block compositing. That is while the Client is alive and the state is set,
     * Compositing is suspended and is resumed when there are no Clients blocking compositing any
     * more.
     *
     * This is actually set by a window property, unfortunately not used by the target application
     * group. For convenience it's exported as a property to the scripts.
     *
     * Use with care!
     **/
    Q_PROPERTY(bool blocksCompositing READ isBlockingCompositing WRITE setBlockingCompositing NOTIFY blockingCompositingChanged)
    /**
     * Whether the decoration is currently using an alpha channel.
     **/
    Q_PROPERTY(bool decorationHasAlpha READ decorationHasAlpha)
    /**
     * Whether the Client uses client side window decorations.
     * Only GTK+ are detected.
     **/
    Q_PROPERTY(bool clientSideDecorated READ isClientSideDecorated NOTIFY clientSideDecoratedChanged)
public:
    explicit Client();
    xcb_window_t wrapperId() const;
    xcb_window_t inputId() const { return m_decoInputExtent; }
    virtual xcb_window_t frameId() const override;

    const Client* transientFor() const;
    Client* transientFor();
    bool isTransient() const override;
    bool groupTransient() const;
    bool wasOriginallyGroupTransient() const;
    ClientList mainClients() const; // Call once before loop , is not indirect
    ClientList allMainClients() const; // Call once before loop , is indirect
    bool hasTransient(const Client* c, bool indirect) const;
    const ClientList& transients() const; // Is not indirect
    void checkTransient(xcb_window_t w);
    AbstractClient* findModal(bool allow_itself = false) override;
    const Group* group() const;
    Group* group();
    void checkGroup(Group* gr = NULL, bool force = false);
    void changeClientLeaderGroup(Group* gr);
    const WindowRules* rules() const override;
    void removeRule(Rules* r);
    void setupWindowRules(bool ignore_temporary);
    void applyWindowRules();
    void updateWindowRules(Rules::Types selection) override;
    void updateFullscreenMonitors(NETFullscreenMonitors topology);

    bool hasNETSupport() const;

    QSize minSize() const;
    QSize maxSize() const;
    QSize basicUnit() const;
    virtual QPoint clientPos() const; // Inside of geometry()
    virtual QSize clientSize() const;
    QPoint inputPos() const { return input_offset; } // Inside of geometry()

    bool windowEvent(xcb_generic_event_t *e);
    void syncEvent(xcb_sync_alarm_notify_event_t* e);
    NET::WindowType windowType(bool direct = false, int supported_types = 0) const;

    bool manage(xcb_window_t w, bool isMapped);
    void releaseWindow(bool on_shutdown = false);
    void destroyClient();

    /// How to resize the window in order to obey constains (mainly aspect ratios)
    enum Sizemode {
        SizemodeAny,
        SizemodeFixedW, ///< Try not to affect width
        SizemodeFixedH, ///< Try not to affect height
        SizemodeMax ///< Try not to make it larger in either direction
    };
    QSize adjustedSize(const QSize&, Sizemode mode = SizemodeAny) const;
    QSize adjustedSize() const;

    virtual int desktop() const;

    void sendToScreen(int screen) override;

    virtual QStringList activities() const;
    void setOnActivity(const QString &activity, bool enable);
    void setOnAllActivities(bool set) override;
    void setOnActivities(QStringList newActivitiesList);
    void updateActivities(bool includeTransients);
    void blockActivityUpdates(bool b = true) override;

    /// Is not minimized and not hidden. I.e. normally visible on some virtual desktop.
    bool isShown(bool shaded_is_shown) const override;
    bool isHiddenInternal() const; // For compositing

    ShadeMode shadeMode() const override; // Prefer isShade()
    void setShade(ShadeMode mode) override;
    bool isShadeable() const override;

    bool isMaximizable() const override;
    QRect geometryRestore() const;
    MaximizeMode maximizeMode() const override;

    QuickTileMode quickTileMode() const;
    bool isMinimizable() const override;
    void setMaximize(bool vertically, bool horizontally);
    QRect iconGeometry() const;

    void setFullScreen(bool set, bool user = true) override;
    bool isFullScreen() const override;
    bool isFullScreenable(bool fullscreen_hack = false) const;
    bool isActiveFullScreen() const;
    bool userCanSetFullScreen() const override;
    QRect geometryFSRestore() const {
        return geom_fs_restore;    // Only for session saving
    }
    int fullScreenMode() const {
        return fullscreen_mode;    // only for session saving
    }

    bool noBorder() const override;
    void setNoBorder(bool set) override;
    bool userCanSetNoBorder() const override;
    void checkNoBorder();

    bool skipTaskbar(bool from_outside = false) const;
    void setSkipTaskbar(bool set, bool from_outside = false);

    bool skipPager() const;
    void setSkipPager(bool);

    virtual Layer layer() const;
    Layer belongsToLayer() const;
    void invalidateLayer();
    void updateLayer() override;
    int sessionStackingOrder() const;

    void setModal(bool modal);
    bool isModal() const;

    // Auxiliary functions, depend on the windowType
    bool wantsInput() const override;

    bool isResizable() const override;
    bool isMovable() const override;
    bool isMovableAcrossScreens() const override;
    bool isCloseable() const override; ///< May be closed by the user (May have a close button)

    void takeFocus() override;

    void updateDecoration(bool check_workspace_pos, bool force = false);
    void triggerDecorationRepaint();

    void updateShape();

    enum ForceGeometry_t { NormalGeometrySet, ForceGeometrySet };
    void setGeometry(int x, int y, int w, int h, ForceGeometry_t force = NormalGeometrySet);
    void setGeometry(const QRect& r, ForceGeometry_t force = NormalGeometrySet);
    void move(int x, int y, ForceGeometry_t force = NormalGeometrySet);
    void move(const QPoint& p, ForceGeometry_t force = NormalGeometrySet);
    /// plainResize() simply resizes
    void plainResize(int w, int h, ForceGeometry_t force = NormalGeometrySet);
    void plainResize(const QSize& s, ForceGeometry_t force = NormalGeometrySet);
    /// resizeWithChecks() resizes according to gravity, and checks workarea position
    void resizeWithChecks(int w, int h, xcb_gravity_t gravity = XCB_GRAVITY_BIT_FORGET, ForceGeometry_t force = NormalGeometrySet);
    void resizeWithChecks(const QSize& s, xcb_gravity_t gravity = XCB_GRAVITY_BIT_FORGET, ForceGeometry_t force = NormalGeometrySet);
    void keepInArea(QRect area, bool partial = false);
    void setElectricBorderMode(QuickTileMode mode);
    QuickTileMode electricBorderMode() const;
    void setElectricBorderMaximizing(bool maximizing);
    bool isElectricBorderMaximizing() const;
    QRect electricBorderMaximizeGeometry(QPoint pos, int desktop);
    QSize sizeForClientSize(const QSize&, Sizemode mode = SizemodeAny, bool noframe = false) const;

    /** Set the quick tile mode ("snap") of this window.
     * This will also handle preserving and restoring of window geometry as necessary.
     * @param mode The tile mode (left/right) to give this window.
     */
    void setQuickTileMode(QuickTileMode mode, bool keyboard = false) override;

    void growHorizontal() override;
    void shrinkHorizontal() override;
    void growVertical() override;
    void shrinkVertical() override;

    bool providesContextHelp() const;
    const QKeySequence &shortcut() const override;
    void setShortcut(const QString& cut) override;

    Options::WindowOperation mouseButtonToWindowOperation(Qt::MouseButtons button);
    bool performMouseCommand(Options::MouseCommand, const QPoint& globalPos) override;

    QRect adjustedClientArea(const QRect& desktop, const QRect& area) const;

    xcb_colormap_t colormap() const;

    /// Updates visibility depending on being shaded, virtual desktop, etc.
    void updateVisibility();
    /// Hides a client - Basically like minimize, but without effects, it's simply hidden
    void hideClient(bool hide);
    bool hiddenPreview() const; ///< Window is mapped in order to get a window pixmap

    virtual bool setupCompositing();
    void finishCompositing(ReleaseReason releaseReason = ReleaseReason::Release) override;
    void setBlockingCompositing(bool block);
    inline bool isBlockingCompositing() { return blocks_compositing; }

    QString caption(bool full = true, bool stripped = false) const override;

    void keyPressEvent(uint key_code, xcb_timestamp_t time = XCB_TIME_CURRENT_TIME);   // FRAME ??
    void updateMouseGrab() override;
    xcb_window_t moveResizeGrabWindow() const;

    const QPoint calculateGravitation(bool invert, int gravity = 0) const;   // FRAME public?

    void NETMoveResize(int x_root, int y_root, NET::Direction direction);
    void NETMoveResizeWindow(int flags, int x, int y, int width, int height);
    void restackWindow(xcb_window_t above, int detail, NET::RequestSource source, xcb_timestamp_t timestamp,
                       bool send_event = false);

    void gotPing(xcb_timestamp_t timestamp);

    void checkWorkspacePosition(QRect oldGeometry = QRect(), int oldDesktop = -2) override;
    void updateUserTime(xcb_timestamp_t time = XCB_TIME_CURRENT_TIME);
    xcb_timestamp_t userTime() const override;
    bool hasUserTimeSupport() const;

    /// Does 'delete c;'
    static void deleteClient(Client* c);

    static bool belongToSameApplication(const Client* c1, const Client* c2, bool active_hack = false);
    static bool sameAppWindowRoleMatch(const Client* c1, const Client* c2, bool active_hack);

    void killWindow();
    void maximize(MaximizeMode) override;
    void toggleShade();
    void showContextHelp();
    void cancelShadeHoverTimer();
    void checkActiveModal();
    StrutRect strutRect(StrutArea area) const;
    StrutRects strutRects() const;
    bool hasStrut() const;

    // Tabbing functions
    TabGroup* tabGroup() const override; // Returns a pointer to client_group
    Q_INVOKABLE inline bool tabBefore(Client *other, bool activate) { return tabTo(other, false, activate); }
    Q_INVOKABLE inline bool tabBehind(Client *other, bool activate) { return tabTo(other, true, activate); }
    /**
     * Syncs the *dynamic* @param property @param fromThisClient or the @link currentTab() to
     * all members of the @link tabGroup() (if there is one)
     *
     * eg. if you call:
     * client->setProperty("kwin_tiling_floats", true);
     * client->syncTabGroupFor("kwin_tiling_floats", true)
     * all clients in this tabGroup will have ::property("kwin_tiling_floats").toBool() == true
     *
     * WARNING: non dynamic properties are ignored - you're not supposed to alter/update such explicitly
     */
    Q_INVOKABLE void syncTabGroupFor(QString property, bool fromThisClient = false);
    Q_INVOKABLE bool untab(const QRect &toGeometry = QRect(), bool clientRemoved = false) override;
    /**
     * Set tab group - this is to be invoked by TabGroup::add/remove(client) and NO ONE ELSE
     */
    void setTabGroup(TabGroup* group);
    /*
    *   If shown is true the client is mapped and raised, if false
    *   the client is unmapped and hidden, this function is called
    *   when the tabbing group of the client switches its visible
    *   client.
    */
    void setClientShown(bool shown);
    /*
    *   When a click is done in the decoration and it calls the group
    *   to change the visible client it starts to move-resize the new
    *   client, this function stops it.
    */
    void dontMoveResize();
    bool isCurrentTab() const override;

    /**
     * Whether or not the window has a strut that expands through the invisible area of
     * an xinerama setup where the monitors are not the same resolution.
     */
    bool hasOffscreenXineramaStrut() const;

    bool isMove() const {
        return moveResizeMode && mode == PositionCenter;
    }
    bool isResize() const {
        return moveResizeMode && mode != PositionCenter;
    }

    // Decorations <-> Effects
    KDecoration2::Decoration *decoration() {
        return m_decoration;
    }
    const KDecoration2::Decoration *decoration() const {
        return m_decoration;
    }
    QPointer<Decoration::DecoratedClientImpl> decoratedClient() const;
    bool isDecorated() const {
        return m_decoration != nullptr;
    }
    void setDecoratedClient(QPointer<Decoration::DecoratedClientImpl> client);

    QRect decorationRect() const;

    QRect transparentRect() const;

    bool decorationHasAlpha() const;
    bool isClientSideDecorated() const;
    bool wantsShadowToBeRendered() const override;

    void layoutDecorationRects(QRect &left, QRect &top, QRect &right, QRect &bottom) const;

    Xcb::Property fetchFirstInTabBox() const;
    void readFirstInTabBox(Xcb::Property &property);
    void updateFirstInTabBox();
    Xcb::StringProperty fetchColorScheme() const;
    void readColorScheme(Xcb::StringProperty &property);
    void updateColorScheme();

    //sets whether the client should be treated as a SessionInteract window
    void setSessionInteract(bool needed);
    virtual bool isClient() const;
    // a helper for the workspace window packing. tests for screen validity and updates since in maximization case as with normal moving
    void packTo(int left, int top) override;

    template <typename T>
    void print(T &stream) const;

    void cancelFocusOutTimer();

    /**
     * Restores the Client after it had been hidden due to show on screen edge functionality.
     * In addition the property gets deleted so that the Client knows that it is visible again.
     **/
    void showOnScreenEdge();

public Q_SLOTS:
    void closeWindow() override;
    void updateCaption();

private Q_SLOTS:
    void shadeHover();
    void shadeUnhover();

private:
    // Use Workspace::createClient()
    virtual ~Client(); ///< Use destroyClient() or releaseWindow()

    Position mousePosition(const QPoint&) const;
    void updateCursor();

    // Handlers for X11 events
    bool mapRequestEvent(xcb_map_request_event_t *e);
    void unmapNotifyEvent(xcb_unmap_notify_event_t *e);
    void destroyNotifyEvent(xcb_destroy_notify_event_t *e);
    void configureRequestEvent(xcb_configure_request_event_t *e);
    virtual void propertyNotifyEvent(xcb_property_notify_event_t *e) override;
    void clientMessageEvent(xcb_client_message_event_t *e) override;
    void enterNotifyEvent(xcb_enter_notify_event_t *e);
    void leaveNotifyEvent(xcb_leave_notify_event_t *e);
    void focusInEvent(xcb_focus_in_event_t *e);
    void focusOutEvent(xcb_focus_out_event_t *e);
    virtual void damageNotifyEvent();

    bool buttonPressEvent(xcb_window_t w, int button, int state, int x, int y, int x_root, int y_root, xcb_timestamp_t time = XCB_CURRENT_TIME);
    bool buttonReleaseEvent(xcb_window_t w, int button, int state, int x, int y, int x_root, int y_root);
    bool motionNotifyEvent(xcb_window_t w, int state, int x, int y, int x_root, int y_root);
    void checkQuickTilingMaximizationZones(int xroot, int yroot);

    bool processDecorationButtonPress(int button, int state, int x, int y, int x_root, int y_root,
                                      bool ignoreMenu = false);
    Client* findAutogroupCandidate() const;
    void resetShowingDesktop();

protected:
    virtual void debug(QDebug& stream) const;
    virtual bool shouldUnredirect() const;
    void addDamage(const QRegion &damage) override;
    bool belongsToSameApplication(const AbstractClient *other, bool active_hack) const override;
    void doSetActive() override;
    void doSetKeepAbove() override;
    void doSetKeepBelow() override;
    void doSetDesktop(int desktop, int was_desk) override;
    void doMinimize() override;

private Q_SLOTS:
    void delayedSetShortcut();
    void performMoveResize();

    //Signals for the scripting interface
    //Signals make an excellent way for communication
    //in between objects as compared to simple function
    //calls
Q_SIGNALS:
    void clientManaging(KWin::Client*);
    void clientFullScreenSet(KWin::Client*, bool, bool);
    void clientMaximizedStateChanged(KWin::Client*, MaximizeMode);
    void clientMaximizedStateChanged(KWin::Client* c, bool h, bool v);
    void clientStartUserMovedResized(KWin::Client*);
    void clientStepUserMovedResized(KWin::Client *, const QRect&);
    void clientFinishUserMovedResized(KWin::Client*);
    void captionChanged();
    void fullScreenChanged();
    void transientChanged();
    void modalChanged();
    void moveResizedChanged();
    void skipTaskbarChanged();
    void skipPagerChanged();

    /**
     * Emitted whenever the Client's TabGroup changed. That is whenever the Client is moved to
     * another group, but not when a Client gets added or removed to the Client's ClientGroup.
     **/
    void tabGroupChanged();

    /**
     * Emitted whenever the Client want to show it menu
     */
    void showRequest();
    /**
     * Emitted whenever the Client's menu is closed
     */
    void menuHidden();
    /**
     * Emitted whenever the Client's menu is available
     **/
    void appMenuAvailable();
    /**
     * Emitted whenever the Client's menu is unavailable
     */
    void appMenuUnavailable();

    /**
     * Emitted whenever the Client's block compositing state changes.
     **/
    void blockingCompositingChanged(KWin::Client *client);
    void clientSideDecoratedChanged();
    void quickTileModeChanged();

    void closeableChanged(bool);
    void minimizeableChanged(bool);
    void shadeableChanged(bool);
    void maximizeableChanged(bool);

private:
    int borderLeft() const;
    int borderRight() const;
    int borderTop() const;
    int borderBottom() const;
    void exportMappingState(int s);   // ICCCM 4.1.3.1, 4.1.4, NETWM 2.5.1
    bool isManaged() const; ///< Returns false if this client is not yet managed
    void updateAllowedActions(bool force = false);
    QRect fullscreenMonitorsArea(NETFullscreenMonitors topology) const;
    void changeMaximize(bool horizontal, bool vertical, bool adjust);
    int checkFullScreenHack(const QRect& geom) const;   // 0 - None, 1 - One xinerama screen, 2 - Full area
    void updateFullScreenHack(const QRect& geom);
    void getWmNormalHints();
    void getMotifHints();
    void getIcons();
    void fetchName();
    void fetchIconicName();
    QString readName() const;
    void setCaption(const QString& s, bool force = false);
    bool hasTransientInternal(const Client* c, bool indirect, ConstClientList& set) const;
    void finishWindowRules();
    void setShortcutInternal(const QKeySequence &cut = QKeySequence());

    void configureRequest(int value_mask, int rx, int ry, int rw, int rh, int gravity, bool from_tool);
    NETExtendedStrut strut() const;
    int checkShadeGeometry(int w, int h);
    void blockGeometryUpdates(bool block);
    void getSyncCounter();
    void sendSyncRequest();
    bool startMoveResize();
    void finishMoveResize(bool cancel);
    void leaveMoveResize();
    void checkUnrestrictedMoveResize();
    void handleMoveResize(int x, int y, int x_root, int y_root);
    void startDelayedMoveResize();
    void stopDelayedMoveResize();
    void positionGeometryTip();
    void grabButton(int mod);
    void ungrabButton(int mod);
    void resizeDecoration();
    void createDecoration(const QRect &oldgeom);

    void pingWindow();
    void killProcess(bool ask, xcb_timestamp_t timestamp = XCB_TIME_CURRENT_TIME);
    void updateUrgency();
    static void sendClientMessage(xcb_window_t w, xcb_atom_t a, xcb_atom_t protocol,
                                  uint32_t data1 = 0, uint32_t data2 = 0, uint32_t data3 = 0);

    void embedClient(xcb_window_t w, xcb_visualid_t visualid, xcb_colormap_t colormap, uint8_t depth);
    void detectNoBorder();
    Xcb::Property fetchGtkFrameExtents() const;
    void readGtkFrameExtents(Xcb::Property &prop);
    void detectGtkFrameExtents();
    void destroyDecoration();
    void updateFrameExtents();

    void internalShow();
    void internalHide();
    void internalKeep();
    void map();
    void unmap();
    void updateHiddenPreview();

    void updateInputShape();

    xcb_timestamp_t readUserTimeMapTimestamp(const KStartupInfoId* asn_id, const KStartupInfoData* asn_data,
                                  bool session) const;
    xcb_timestamp_t readUserCreationTime() const;
    void startupIdChanged();

    void checkOffscreenPosition (QRect* geom, const QRect& screenArea);

    void updateInputWindow();

    bool tabTo(Client *other, bool behind, bool activate);

    Xcb::Property fetchShowOnScreenEdge() const;
    void readShowOnScreenEdge(Xcb::Property &property);
    /**
     * Reads the property and creates/destroys the screen edge if required
     * and shows/hides the client.
     **/
    void updateShowOnScreenEdge();

    Xcb::Window m_client;
    Xcb::Window m_wrapper;
    Xcb::Window m_frame;
    // wrapper around m_frame to use as a parent for the decoration
    QScopedPointer<QWindow> m_frameWrapper;
    KDecoration2::Decoration *m_decoration;
    QPointer<Decoration::DecoratedClientImpl> m_decoratedClient;
    QElapsedTimer m_decorationDoubleClickTimer;
    QStringList activityList;
    int m_activityUpdatesBlocked;
    bool m_blockedActivityUpdatesRequireTransients;
    bool buttonDown;
    bool moveResizeMode;
    Xcb::Window m_moveResizeGrabWindow;
    bool move_resize_has_keyboard_grab;
    bool unrestrictedMoveResize;
    int moveResizeStartScreen;
    static bool s_haveResizeEffect;
    bool m_managed;

    Position mode;
    QPoint moveOffset;
    QPoint invertedMoveOffset;
    QRect moveResizeGeom;
    QRect initialMoveResizeGeom;
    Xcb::GeometryHints m_geometryHints;
    void sendSyntheticConfigureNotify();
    enum MappingState {
        Withdrawn, ///< Not handled, as per ICCCM WithdrawnState
        Mapped, ///< The frame is mapped
        Unmapped, ///< The frame is not mapped
        Kept ///< The frame should be unmapped, but is kept (For compositing)
    };
    MappingState mapping_state;

    /** The quick tile mode of this window.
     */
    int quick_tile_mode;

    Xcb::TransientFor fetchTransient() const;
    void readTransientProperty(Xcb::TransientFor &transientFor);
    void readTransient();
    xcb_window_t verifyTransientFor(xcb_window_t transient_for, bool set);
    void addTransient(Client* cl);
    void removeTransient(Client* cl);
    void removeFromMainClients();
    void cleanGrouping();
    void checkGroupTransients();
    void setTransient(xcb_window_t new_transient_for_id);
    Client* transient_for;
    xcb_window_t m_transientForId;
    xcb_window_t m_originalTransientForId;
    ClientList transients_list; // SELI TODO: Make this ordered in stacking order?
    ShadeMode shade_mode;
    Client *shade_below;
    uint deleting : 1; ///< True when doing cleanup and destroying the client
    uint skip_taskbar : 1;
    uint original_skip_taskbar : 1; ///< Unaffected by KWin
    uint skip_pager : 1;
    Xcb::MotifHints m_motif;
    uint hidden : 1; ///< Forcibly hidden by calling hide()
    uint modal : 1; ///< NET::Modal
    uint noborder : 1;
    uint app_noborder : 1; ///< App requested no border via window type, shape extension, etc.
    uint ignore_focus_stealing : 1; ///< Don't apply focus stealing prevention to this client
    bool blocks_compositing;
    WindowRules client_rules;
    Qt::CursorShape m_cursor;
    // DON'T reorder - Saved to config files !!!
    enum FullScreenMode {
        FullScreenNone,
        FullScreenNormal,
        FullScreenHack ///< Non-NETWM fullscreen (noborder and size of desktop)
    };
    FullScreenMode fullscreen_mode;
    MaximizeMode max_mode;
    QRect geom_restore;
    QRect geom_fs_restore;
    QTimer* shadeHoverTimer;
    QTimer* delayedMoveResizeTimer;
    xcb_colormap_t m_colormap;
    QString cap_normal, cap_iconic, cap_suffix, cap_deco;
    Group* in_group;
    TabGroup* tab_group;
    Layer in_layer;
    QTimer* ping_timer;
    qint64 m_killHelperPID;
    xcb_timestamp_t m_pingTimestamp;
    xcb_timestamp_t m_userTime;
    NET::Actions allowed_actions;
    QSize client_size;
    int block_geometry_updates; // > 0 = New geometry is remembered, but not actually set
    enum PendingGeometry_t {
        PendingGeometryNone,
        PendingGeometryNormal,
        PendingGeometryForced
    };
    PendingGeometry_t pending_geometry_update;
    QRect geom_before_block;
    QRect deco_rect_before_block;
    bool shade_geometry_change;
    struct {
        xcb_sync_counter_t counter;
        xcb_sync_int64_t value;
        xcb_sync_alarm_t alarm;
        xcb_timestamp_t lastTimestamp;
        QTimer *timeout, *failsafeTimeout;
        bool isPending;
    } syncRequest;
    static bool check_active_modal; ///< \see Client::checkActiveModal()
    QKeySequence _shortcut;
    int sm_stacking_order;
    friend struct ResetupRulesProcedure;
    friend class GeometryUpdatesBlocker;

    bool electricMaximizing;
    QuickTileMode electricMode;

    friend bool performTransiencyCheck();

    Xcb::StringProperty fetchActivities() const;
    void readActivities(Xcb::StringProperty &property);
    void checkActivities();
    bool activitiesDefined; //whether the x property was actually set

    bool needsSessionInteract;
    bool needsXWindowMove;

    Xcb::Window m_decoInputExtent;
    QPoint input_offset;

    QTimer *m_focusOutTimer;

    QList<QMetaObject::Connection> m_connections;
    bool m_clientSideDecorated;
};

/**
 * Helper for Client::blockGeometryUpdates() being called in pairs (true/false)
 */
class GeometryUpdatesBlocker
{
public:
    explicit GeometryUpdatesBlocker(Client* c)
        : cl(c) {
        cl->blockGeometryUpdates(true);
    }
    ~GeometryUpdatesBlocker() {
        cl->blockGeometryUpdates(false);
    }

private:
    Client* cl;
};

inline xcb_window_t Client::wrapperId() const
{
    return m_wrapper;
}

inline bool Client::isClientSideDecorated() const
{
    return m_clientSideDecorated;
}

inline const Client* Client::transientFor() const
{
    return transient_for;
}

inline Client* Client::transientFor()
{
    return transient_for;
}

inline bool Client::groupTransient() const
{
    return m_transientForId == rootWindow();
}

// Needed because verifyTransientFor() may set transient_for_id to root window,
// if the original value has a problem (window doesn't exist, etc.)
inline bool Client::wasOriginallyGroupTransient() const
{
    return m_originalTransientForId == rootWindow();
}

inline bool Client::isTransient() const
{
    return m_transientForId != XCB_WINDOW_NONE;
}

inline const ClientList& Client::transients() const
{
    return transients_list;
}

inline const Group* Client::group() const
{
    return in_group;
}

inline Group* Client::group()
{
    return in_group;
}

inline TabGroup* Client::tabGroup() const
{
    return tab_group;
}

inline bool Client::isShown(bool shaded_is_shown) const
{
    return !isMinimized() && (!isShade() || shaded_is_shown) && !hidden &&
           (!tabGroup() || tabGroup()->current() == this);
}

inline bool Client::isHiddenInternal() const
{
    return hidden;
}

inline ShadeMode Client::shadeMode() const
{
    return shade_mode;
}

inline QRect Client::geometryRestore() const
{
    return geom_restore;
}

inline MaximizeMode Client::maximizeMode() const
{
    return max_mode;
}

inline Client::QuickTileMode Client::quickTileMode() const
{
    return (Client::QuickTileMode)quick_tile_mode;
}

inline bool Client::skipTaskbar(bool from_outside) const
{
    return from_outside ? original_skip_taskbar : skip_taskbar;
}

inline bool Client::skipPager() const
{
    return skip_pager;
}

inline bool Client::isFullScreen() const
{
    return fullscreen_mode != FullScreenNone;
}

inline bool Client::isModal() const
{
    return modal;
}

inline bool Client::hasNETSupport() const
{
    return info->hasNETSupport();
}

inline xcb_colormap_t Client::colormap() const
{
    return m_colormap;
}

inline void Client::invalidateLayer()
{
    in_layer = UnknownLayer;
}

inline int Client::sessionStackingOrder() const
{
    return sm_stacking_order;
}

inline bool Client::isManaged() const
{
    return m_managed;
}

inline QPoint Client::clientPos() const
{
    return QPoint(borderLeft(), borderTop());
}

inline QSize Client::clientSize() const
{
    return client_size;
}

inline void Client::setGeometry(const QRect& r, ForceGeometry_t force)
{
    setGeometry(r.x(), r.y(), r.width(), r.height(), force);
}

inline void Client::move(const QPoint& p, ForceGeometry_t force)
{
    move(p.x(), p.y(), force);
}

inline void Client::plainResize(const QSize& s, ForceGeometry_t force)
{
    plainResize(s.width(), s.height(), force);
}

inline void Client::resizeWithChecks(const QSize& s, xcb_gravity_t gravity, ForceGeometry_t force)
{
    resizeWithChecks(s.width(), s.height(), gravity, force);
}

inline bool Client::hasUserTimeSupport() const
{
    return info->userTime() != -1U;
}

inline const WindowRules* Client::rules() const
{
    return &client_rules;
}

inline xcb_window_t Client::moveResizeGrabWindow() const
{
    return m_moveResizeGrabWindow;
}

inline const QKeySequence &Client::shortcut() const
{
    return _shortcut;
}

inline void Client::removeRule(Rules* rule)
{
    client_rules.remove(rule);
}

inline bool Client::hiddenPreview() const
{
    return mapping_state == Kept;
}

template <typename T>
inline void Client::print(T &stream) const
{
    stream << "\'ID:" << window() << ";WMCLASS:" << resourceClass() << ":"
           << resourceName() << ";Caption:" << caption() << "\'";
}

} // namespace
Q_DECLARE_METATYPE(KWin::Client*)
Q_DECLARE_METATYPE(QList<KWin::Client*>)

#endif
