#ifndef SOUNDBOARDFRAME_H
#define SOUNDBOARDFRAME_H

#include <QFrame>
#include <QPointer>

namespace Ui {
class SoundboardFrame;
}

class Campaign;
class QVBoxLayout;
class SoundBoardGroupFrame;
class SoundboardGroup;
class SoundboardMixer;
class AudioTrack;
class CampaignObjectBase;

class SoundboardFrame : public QFrame
{
    Q_OBJECT

public:
    explicit SoundboardFrame(QWidget *parent = nullptr);
    ~SoundboardFrame();

    void setMixer(SoundboardMixer* mixer);

signals:
    void trackCreated(CampaignObjectBase* track);
    void dirty();

public slots:
    void setCampaign(Campaign* campaign);
    void addTrackToTree(AudioTrack* track);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void showEvent(QShowEvent *event) override;

    void updateTrackLayout();

protected slots:
    void addGroup();
    void removeGroup(SoundboardGroup* group);

    void addSound();
    void addYoutube();
    void addSyrinscape();

    void addTrack(const QUrl& url);

private:
    void addGroupToLayout(SoundboardGroup* group);

    Ui::SoundboardFrame *ui;

    QVBoxLayout* _layout;
    QList<SoundBoardGroupFrame*> _groupList;
    bool _mouseDown;
    QPoint _mouseDownPos;
    Campaign* _campaign;
    QPointer<SoundboardMixer> _mixer;
};

#endif // SOUNDBOARDFRAME_H
