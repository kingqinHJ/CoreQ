#ifndef FOCUSDISMISSOR_H
#define FOCUSDISMISSOR_H

#include <QObject>
#include <QQmlEngine>
#include <QQuickWindow>

class FocusDismissorPrivate;
class FocusDismissor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQuickWindow* target READ target WRITE setTarget NOTIFY targetChanged FINAL)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    QML_ATTACHED(FocusDismissor)

public:
    explicit FocusDismissor(QObject *parent = nullptr);

    QQuickWindow *target() const;
    void setTarget(QQuickWindow *v);

    bool enabled() const;
    void setEnabled(bool v);

    static FocusDismissor *qmlAttachedProperties(QObject *object);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void targetChanged();
    void enabledChanged();

    void clickedOutside();

private:
    std::shared_ptr<FocusDismissorPrivate> d;
};

#endif // FOCUSDISMISSOR_H
