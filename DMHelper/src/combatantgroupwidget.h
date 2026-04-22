#ifndef COMBATANTGROUPWIDGET_H
#define COMBATANTGROUPWIDGET_H

#include <QFrame>
#include <QUuid>
#include <QList>

namespace Ui {
class CombatantGroupWidget;
}

class BattleDialogModelCombatantGroup;
class CombatantWidget;

class CombatantGroupWidget : public QFrame
{
    Q_OBJECT

public:
    explicit CombatantGroupWidget(BattleDialogModelCombatantGroup* group, QWidget *parent = nullptr);
    virtual ~CombatantGroupWidget() override;

    BattleDialogModelCombatantGroup* getGroup() const;
    QUuid getGroupId() const;

    void addMemberWidget(CombatantWidget* widget);
    void removeMemberWidget(CombatantWidget* widget);
    QList<CombatantWidget*> getMemberWidgets() const;

    void setCollapsed(bool collapsed);
    bool isCollapsed() const;

    void updateMasterCheckboxes();

signals:
    void clicked(CombatantGroupWidget* groupWidget);
    void contextMenu(BattleDialogModelCombatantGroup* group, const QPoint& position);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private slots:
    void handleCollapseToggled(bool checked);
    void handleNameChanged(const QString& name);
    void handleInitiativeChanged(const QString& text);
    void handleVisibleClicked(bool checked);
    void handleKnownClicked(bool checked);
    void handleMemberVisibilityChanged();

private:
    Ui::CombatantGroupWidget *ui;
    BattleDialogModelCombatantGroup* _group;
    QList<CombatantWidget*> _memberWidgets;
    bool _updatingCheckboxes;
};

#endif // COMBATANTGROUPWIDGET_H
