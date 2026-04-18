#include "soundboardtrackframe.h"
#include "ui_soundboardtrackframe.h"
#include "soundboardtrack.h"
#include "soundboardmixer.h"
#include "audiotrack.h"
#include "dmconstants.h"
#include <QPainter>
#include <QTimer>

SoundboardTrackFrame::SoundboardTrackFrame(SoundboardTrack* track, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::SoundboardTrackFrame),
    _track(nullptr),
    _localMute(false),
    _currentMute(false),
    _trackLength("0:00"),
    _trackPosition("0:00"),
    _mixer(nullptr),
    _parentGroup(nullptr)
{
    ui->setupUi(this);

    // btnRepeat now doubles as the Loop <-> OneShot toggle. Checked = Loop
    // (the natural meaning of "repeat"), unchecked = OneShot.
    ui->btnRepeat->setToolTip(tr("Loop (checked) or one-shot (unchecked) playback"));

    setTrack(track);

    connect(ui->btnMute, &QAbstractButton::clicked, this, &SoundboardTrackFrame::toggleMute);
    connect(ui->btnPlay, &QAbstractButton::toggled, this, &SoundboardTrackFrame::togglePlay);
    connect(ui->btnRepeat, &QAbstractButton::toggled, this, &SoundboardTrackFrame::repeatChanged);
    connect(ui->btnRepeat, &QAbstractButton::toggled, this, &SoundboardTrackFrame::handlePlaybackModeToggle);
    connect(ui->btnRemove, &QAbstractButton::clicked, this, &SoundboardTrackFrame::handleRemove);
}

void SoundboardTrackFrame::setMixer(SoundboardMixer* mixer, SoundboardGroup* parentGroup)
{
    _mixer = mixer;
    _parentGroup = parentGroup;
}

void SoundboardTrackFrame::handlePlaybackModeToggle(bool loopChecked)
{
    if(!_track)
        return;

    _track->setPlaybackMode(loopChecked
        ? SoundboardTrack::PlaybackMode_Loop
        : SoundboardTrack::PlaybackMode_OneShot);
}

SoundboardTrackFrame::~SoundboardTrackFrame()
{
    delete ui;
}

SoundboardTrack* SoundboardTrackFrame::getTrack() const
{
    return _track;
}

int SoundboardTrackFrame::getAudioType() const
{
    if(!_track)
        return DMHelper::AudioType_Unknown;

    return _track->getAudioType();
}

void SoundboardTrackFrame::setMute(bool mute)
{
    setCurrentMute(mute);
    _localMute = mute;
}

void SoundboardTrackFrame::parentMuteChanged(bool mute)
{
    setCurrentMute(mute || _localMute);
}

void SoundboardTrackFrame::setTrackLength(int trackLength)
{
    _trackLength = QString("%1:%2").arg(QString::number(trackLength / 60), 2, QChar('0')).arg(QString::number(trackLength % 60), 2, QChar('0'));
    updateProgress();
}

void SoundboardTrackFrame::setTrackPosition(int trackPosition)
{
    _trackPosition = QString("%1:%2").arg(QString::number(trackPosition / 60), 2, QChar('0')).arg(QString::number(trackPosition % 60), 2, QChar('0'));
    updateProgress();
}

void SoundboardTrackFrame::setRepeat(bool repeat)
{
    ui->btnRepeat->setChecked(repeat);
}

void SoundboardTrackFrame::togglePlay(bool checked)
{
    if(checked)
    {
        ui->btnPlay->setStyleSheet("background-color: rgb(115, 18, 0); border: 5px solid rgb(115, 18, 0);");

        if(_currentMute)
            ui->btnMute->click();

        emit play();

        if(getAudioType() == DMHelper::AudioType_Syrinscape)
            QTimer::singleShot(500, ui->btnPlay, &QAbstractButton::click);
    }
    else
    {
        ui->btnPlay->setStyleSheet("");
        emit stop();
    }
}

void SoundboardTrackFrame::toggleMute()
{
    bool newMute = ui->btnMute->isChecked();
    setCurrentMute(newMute);
    _localMute = newMute;
}

void SoundboardTrackFrame::setCurrentMute(bool mute)
{
    if(_currentMute != mute)
    {
        emit muteChanged(mute);
        _currentMute = mute;
        ui->btnMute->setIcon(mute ? QIcon(QPixmap(":/img/data/icon_volumeoff.png")) : QIcon(QPixmap(":/img/data/icon_volumeon.png")));
        ui->btnMute->setChecked(mute);
        if(getAudioType() != DMHelper::AudioType_Syrinscape)
            ui->slideVolume->setEnabled(!mute);
    }
}

void SoundboardTrackFrame::updateProgress()
{
    ui->lblProgress->setText(_trackPosition +  QString(" / ") + _trackLength);
}

void SoundboardTrackFrame::handleRemove()
{
    if(_track)
        emit removeTrack(_track);
}

void SoundboardTrackFrame::setTrack(SoundboardTrack* track)
{
    if((_track) || (!track) || (!track->getTrack()))
        return;

    _track = track;

    switch(getAudioType())
    {
        case DMHelper::AudioType_Syrinscape:
            ui->btnPlay->setIcon(QIcon(QString(":/img/data/icon_syrinscape.png")));
            ui->btnRepeat->setEnabled(false);
            ui->btnMute->setEnabled(false);
            ui->slideVolume->setEnabled(false);
            ui->lblProgress->setText(QString("--:-- / --:--"));
            ui->lblProgress->setEnabled(false);
            break;
        case DMHelper::AudioType_Youtube:
            ui->btnPlay->setIcon(QIcon(QString(":/img/data/icon_playerswindow.png")));
            break;
        default:
            break;
    }

    ui->lblName->setText(_track->getTrackName());
    setToolTip(_track->getTrackDetails());
    ui->slideVolume->setValue(_track->getVolume());
    setMute(_track->getMute());
    ui->btnRepeat->setChecked(_track->getPlaybackMode() == SoundboardTrack::PlaybackMode_Loop);

    // Playback is routed through the mixer when one is installed (via
    // setMixer) so crossfade and channel management work; otherwise fall
    // back to the track's own direct play/stop.
    connect(this, &SoundboardTrackFrame::play, this, [this]() {
        if(!_track) return;
        if(_mixer)
            _mixer->playTrack(_track, _parentGroup.data());
        else
            _track->play();
    });
    connect(this, &SoundboardTrackFrame::stop, this, [this]() {
        if(!_track) return;
        _track->stop();
    });
    connect(this, &SoundboardTrackFrame::muteChanged, _track, &SoundboardTrack::setMute);
    connect(ui->slideVolume, &QAbstractSlider::valueChanged, _track, &SoundboardTrack::setVolume);
    connect(_track, &SoundboardTrack::trackLengthChanged, this, &SoundboardTrackFrame::setTrackLength);
    connect(_track, &SoundboardTrack::trackPositionChanged, this, &SoundboardTrackFrame::setTrackPosition);
    connect(_track, &AudioTrack::destroyed, this, &SoundboardTrackFrame::handleRemove);
}
