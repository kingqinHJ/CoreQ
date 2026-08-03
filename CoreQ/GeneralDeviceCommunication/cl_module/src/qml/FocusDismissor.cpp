#include "FocusDismissor.h"
#include <QQuickItem>

class FocusDismissorPrivate
{
public:
    QQuickWindow *target = nullptr;
    bool enabled = true;

    QQuickItem *pressItem = nullptr;
};

FocusDismissor::FocusDismissor(QObject *parent)
    : QObject{parent}
{
    d.reset(new FocusDismissorPrivate);
}

QQuickWindow *FocusDismissor::target() const
{
    return d->target;
}

void FocusDismissor::setTarget(QQuickWindow *v)
{
    if (d->target == v)
        return;

    if (d->target)
        d->target->removeEventFilter(this);

    d->target = v;

    if (d->target)
        d->target->installEventFilter(this);
}

bool FocusDismissor::enabled() const
{
    return d->enabled;
}

void FocusDismissor::setEnabled(bool v)
{
    if (d->enabled == v)
        return;

    d->enabled = v;
    emit enabledChanged();
}

FocusDismissor *FocusDismissor::qmlAttachedProperties(QObject *object)
{
    return new FocusDismissor(object);
}

bool FocusDismissor::eventFilter(QObject *watched, QEvent *event)
{
    if (d->enabled && d->target == watched) {

        if (d->target->activeFocusItem() && event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            QPointF pos = me->localPos(); // window local pos

            QMetaObject::invokeMethod(this, [this, pos](){
                QQuickItem *content = d->target->contentItem();
                auto item = d->target->activeFocusItem();
                if (!item || !content) return;

                auto localPos = item->mapFromItem(content, pos);
                if (!item->contains(localPos)) {

                    QString className = item->metaObject()->className();
                    if (className.contains("text", Qt::CaseInsensitive)
                        || className.contains("input", Qt::CaseInsensitive)
                        || className.contains("button", Qt::CaseInsensitive)
                        )
                        emit clickedOutside();
                }
            }, Qt::QueuedConnection);
        }
    }

    return false;
}
