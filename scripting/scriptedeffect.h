/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

 Copyright (C) 2012 Martin Gräßlin <mgraesslin@kde.org>

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

#ifndef KWIN_SCRIPTEDEFFECT_H
#define KWIN_SCRIPTEDEFFECT_H

#include <kwinanimationeffect.h>
#include <KService>

class KConfigLoader;
class QScriptEngine;
class QScriptValue;

namespace KWin
{
class ScriptedEffect : public KWin::AnimationEffect
{
    Q_OBJECT
    Q_ENUMS(DataRole)
    Q_ENUMS(Qt::Axis)
    Q_ENUMS(Anchor)
    Q_ENUMS(MetaType)
    Q_ENUMS(EasingCurve)
public:
    // copied from kwineffects.h
    enum DataRole {
        // Grab roles are used to force all other animations to ignore the window.
        // The value of the data is set to the Effect's `this` value.
        WindowAddedGrabRole = 1,
        WindowClosedGrabRole,
        WindowMinimizedGrabRole,
        WindowUnminimizedGrabRole,
        WindowForceBlurRole, ///< For fullscreen effects to enforce blurring of windows,
        WindowBlurBehindRole, ///< For single windows to blur behind
        WindowForceBackgroundContrastRole, ///< For fullscreen effects to enforce the background contrast,
        WindowBackgroundContrastRole, ///< For single windows to enable Background contrast
        LanczosCacheRole
    };
    enum EasingCurve {
        GaussianCurve = 128
    };
    const QString &scriptFile() const {
        return m_scriptFile;
    }
    virtual void reconfigure(ReconfigureFlags flags);
    int requestedEffectChainPosition() const override {
        return m_chainPosition;
    }
    QString activeConfig() const;
    void setActiveConfig(const QString &name);
    static ScriptedEffect *create(KService::Ptr effect);
    static ScriptedEffect *create(const QString &effectName, const QString &pathToScript, int chainPosition);
    virtual ~ScriptedEffect();
    /**
     * Whether another effect has grabbed the @p w with the given @p grabRole.
     * @param w The window to check
     * @param grabRole The grab role to check
     * @returns @c true if another window has grabbed the effect, @c false otherwise
     **/
    Q_SCRIPTABLE bool isGrabbed(KWin::EffectWindow *w, DataRole grabRole);
    /**
     * Reads the value from the configuration data for the given key.
     * @param key The key to search for
     * @param defaultValue The value to return if the key is not found
     * @returns The config value if present
     **/
    Q_SCRIPTABLE QVariant readConfig(const QString &key, const QVariant defaultValue = QVariant());
    void registerShortcut(QAction *a, QScriptValue callback);
    const QHash<QAction*, QScriptValue> &shortcutCallbacks() const {
        return m_shortcutCallbacks;
    }
    QHash<int, QList<QScriptValue > > &screenEdgeCallbacks() {
        return m_screenEdgeCallbacks;
    }

public Q_SLOTS:
    quint64 animate(KWin::EffectWindow *w, Attribute a, int ms, KWin::FPx2 to, KWin::FPx2 from = KWin::FPx2(), uint metaData = 0, QEasingCurve::Type curve = QEasingCurve::Linear, int delay = 0);
    quint64 set(KWin::EffectWindow *w, Attribute a, int ms, KWin::FPx2 to, KWin::FPx2 from = KWin::FPx2(), uint metaData = 0, QEasingCurve::Type curve = QEasingCurve::Linear, int delay = 0);
    bool cancel(quint64 animationId) { return AnimationEffect::cancel(animationId); }
    virtual bool borderActivated(ElectricBorder border);

Q_SIGNALS:
    /**
     * Signal emitted whenever the effect's config changed.
     **/
    void configChanged();
    void animationEnded(KWin::EffectWindow *w, quint64 animationId);

protected:
    void animationEnded(KWin::EffectWindow *w, Attribute a, uint meta);

private Q_SLOTS:
    void signalHandlerException(const QScriptValue &value);
    void globalShortcutTriggered();
private:
    ScriptedEffect();
    bool init(const QString &effectName, const QString &pathToScript);
    QScriptEngine *m_engine;
    QString m_effectName;
    QString m_scriptFile;
    QHash<QAction*, QScriptValue> m_shortcutCallbacks;
    QHash<int, QList<QScriptValue> > m_screenEdgeCallbacks;
    KConfigLoader *m_config;
    int m_chainPosition;
};

}

#endif // KWIN_SCRIPTEDEFFECT_H
