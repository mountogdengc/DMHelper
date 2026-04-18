#include "soundboardtrack.h"
#include "audiotrack.h"
#include "dmconstants.h"

SoundboardTrack::SoundboardTrack(AudioTrack* track, int volume, bool mute, PlaybackMode mode, QObject *parent):
    QObject{parent},
    _track{track},
    _volume{volume},
    _mute{mute},
    _playbackMode{mode}
{
    if(_track)
    {
        connect(_track, &AudioTrack::trackLengthChanged, this, &SoundboardTrack::trackLengthChanged);
        connect(_track, &AudioTrack::trackPositionChanged, this, &SoundboardTrack::trackPositionChanged);
    }
}

SoundboardTrack::~SoundboardTrack()
{
}

AudioTrack* SoundboardTrack::getTrack() const
{
    return _track;
}

int SoundboardTrack::getAudioType() const
{
    if(!_track)
        return DMHelper::AudioType_Unknown;

    return _track->getAudioType();
}

int SoundboardTrack::getVolume() const
{
    return _volume;
}

bool SoundboardTrack::getMute() const
{
    return _mute;
}

SoundboardTrack::PlaybackMode SoundboardTrack::getPlaybackMode() const
{
    return _playbackMode;
}

QString SoundboardTrack::getTrackName() const
{
    if(!_track)
        return QString();

    return _track->getName();
}

QString SoundboardTrack::getTrackDetails() const
{
    if(!_track)
        return QString();

    return _track->getUrl().toDisplayString();
}

void SoundboardTrack::play()
{
    if(_track)
    {
        _track->setRepeat(_playbackMode == PlaybackMode_Loop);
        _track->play();
    }
}

void SoundboardTrack::stop()
{
    if(_track)
        _track->stop();
}

void SoundboardTrack::setVolume(int volume)
{
    if(_volume == volume)
        return;

    if((volume < 0) || (volume > 100))
        return;

    _volume = volume;
    emit volumeChanged(volume);

    if(_track)
        _track->setVolume(static_cast<float>(volume) / 100.f);
}

void SoundboardTrack::setMute(bool mute)
{
    if(_mute == mute)
        return;

    _mute = mute;
    emit muteChanged(mute);

    if(_track)
        _track->setMute(mute);
}

void SoundboardTrack::setPlaybackMode(int mode)
{
    PlaybackMode newMode = (mode == PlaybackMode_OneShot) ? PlaybackMode_OneShot : PlaybackMode_Loop;
    if(_playbackMode == newMode)
        return;

    _playbackMode = newMode;
    emit playbackModeChanged(newMode);

    if(_track)
        _track->setRepeat(_playbackMode == PlaybackMode_Loop);
}
