#include "soundboardmixer.h"
#include "soundboardtrack.h"
#include "soundboardgroup.h"
#include "audiotrack.h"
#include <QVariantAnimation>
#include <QDebug>

namespace {
// Clamp-and-scale int 0..100 to float 0..1 for AudioTrack::setVolume.
float toFloatVolume(int vol)
{
    if(vol < 0) vol = 0;
    if(vol > 100) vol = 100;
    return static_cast<float>(vol) / 100.f;
}
}

SoundboardMixer::SoundboardMixer(QObject* parent) :
    QObject(parent),
    _crossfadeMs(2000),
    _currentMusic(nullptr),
    _currentAmbient(nullptr),
    _activeSfx(),
    _fades()
{
}

SoundboardMixer::~SoundboardMixer()
{
    // Animations are parented to this; Qt deletes them. Tracks are not owned.
}

int SoundboardMixer::getCrossfadeMs() const
{
    return _crossfadeMs;
}

SoundboardTrack* SoundboardMixer::getCurrentMusic() const
{
    return _currentMusic.data();
}

SoundboardTrack* SoundboardMixer::getCurrentAmbient() const
{
    return _currentAmbient.data();
}

void SoundboardMixer::setCrossfadeMs(int ms)
{
    if(ms < 0)
        ms = 0;
    if(ms > 30000)
        ms = 30000;
    if(_crossfadeMs == ms)
        return;

    _crossfadeMs = ms;
    emit crossfadeMsChanged(_crossfadeMs);
}

void SoundboardMixer::playTrackOnChannel(int channel, SoundboardTrack* track)
{
    if(!track)
        return;

    // Route to SFX immediately, regardless of track's own mode, since
    // this channel choice is an explicit override by the caller.
    if(channel == Channel_SFX)
    {
        AudioTrack* at = track->getTrack();
        if(!at)
            return;

        // Force one-shot behavior for an SFX hit.
        at->setRepeat(false);
        at->setVolume(toFloatVolume(track->getVolume()));

        if(!_activeSfx.contains(track))
        {
            _activeSfx.append(QPointer<SoundboardTrack>(track));
            connect(at, &AudioTrack::trackStatusChanged, this, &SoundboardMixer::handleSfxStatusChanged, Qt::UniqueConnection);
            connect(track, &QObject::destroyed, this, &SoundboardMixer::handleTrackDestroyed, Qt::UniqueConnection);
        }
        at->play();
        return;
    }

    if((channel != Channel_Music) && (channel != Channel_Ambient))
        return;

    QPointer<SoundboardTrack>& slot = (channel == Channel_Music) ? _currentMusic : _currentAmbient;
    SoundboardTrack* outgoing = slot.data();

    if(outgoing == track)
        return; // Already playing that track on this channel.

    // Fade out whatever was there.
    if(outgoing)
        fadeOutAndStop(outgoing, _crossfadeMs);

    // Prepare incoming: loop-mode, start at volume 0, then fade in.
    AudioTrack* incomingAt = track->getTrack();
    if(!incomingAt)
        return;

    incomingAt->setRepeat(track->getPlaybackMode() == SoundboardTrack::PlaybackMode_Loop);
    incomingAt->setVolume(0.f);
    incomingAt->play();

    connect(track, &QObject::destroyed, this, &SoundboardMixer::handleTrackDestroyed, Qt::UniqueConnection);
    setCurrentOnChannel(channel, track);
    fadeIn(track, _crossfadeMs);
}

void SoundboardMixer::playTrack(SoundboardTrack* track, SoundboardGroup* parentGroup)
{
    if(!track)
        return;

    playTrackOnChannel(resolveChannel(track, parentGroup), track);
}

void SoundboardMixer::stopChannel(int channel)
{
    if(channel == Channel_Music)
    {
        if(_currentMusic)
        {
            fadeOutAndStop(_currentMusic.data(), _crossfadeMs);
            setCurrentOnChannel(Channel_Music, nullptr);
        }
    }
    else if(channel == Channel_Ambient)
    {
        if(_currentAmbient)
        {
            fadeOutAndStop(_currentAmbient.data(), _crossfadeMs);
            setCurrentOnChannel(Channel_Ambient, nullptr);
        }
    }
    else if(channel == Channel_SFX)
    {
        for(const QPointer<SoundboardTrack>& sfx : _activeSfx)
        {
            if(sfx && sfx->getTrack())
                sfx->getTrack()->stop();
        }
        _activeSfx.clear();
    }
}

void SoundboardMixer::playGroup(SoundboardGroup* group)
{
    if(!group)
    {
        stopChannel(Channel_Music);
        stopChannel(Channel_Ambient);
        return;
    }

    // Pick the first Loop track whose resolved channel is Music and the
    // first whose resolved channel is Ambient. SFX tracks inside the group
    // are left for the DM to trigger manually.
    SoundboardTrack* nextMusic = nullptr;
    SoundboardTrack* nextAmbient = nullptr;

    for(SoundboardTrack* track : group->getTracks())
    {
        if((!track) || (!track->getTrack()))
            continue;
        if(track->getPlaybackMode() != SoundboardTrack::PlaybackMode_Loop)
            continue;

        Channel ch = resolveChannel(track, group);
        if((ch == Channel_Music) && (!nextMusic))
            nextMusic = track;
        else if((ch == Channel_Ambient) && (!nextAmbient))
            nextAmbient = track;
    }

    if(nextMusic)
        playTrackOnChannel(Channel_Music, nextMusic);
    else
        stopChannel(Channel_Music);

    if(nextAmbient)
        playTrackOnChannel(Channel_Ambient, nextAmbient);
    else
        stopChannel(Channel_Ambient);
}

void SoundboardMixer::stopAll()
{
    stopChannel(Channel_Music);
    stopChannel(Channel_Ambient);
    stopChannel(Channel_SFX);
}

void SoundboardMixer::handleTrackDestroyed(QObject* obj)
{
    // By the time this fires, the concrete SoundboardTrack has already been
    // destroyed - QPointer members auto-null and a reinterpret_cast is not
    // safe. We just scrub stale state on a best-effort basis. Track
    // identity in _fades uses the raw pointer as the key; remove entries
    // whose key matches the destroyed QObject's address.
    void* addr = static_cast<void*>(obj);

    // Clean up SFX list (QPointer will already have auto-nulled).
    for(int i = _activeSfx.count() - 1; i >= 0; --i)
    {
        if(_activeSfx.at(i).isNull())
            _activeSfx.removeAt(i);
    }

    // Emit channel-empty notifications if a current slot went null.
    if(_currentMusic.isNull())
        emit currentMusicChanged(nullptr);
    if(_currentAmbient.isNull())
        emit currentAmbientChanged(nullptr);

    // Drop any fade animation keyed on the destroyed address.
    for(auto it = _fades.begin(); it != _fades.end(); )
    {
        if(static_cast<void*>(it.key()) == addr)
        {
            QVariantAnimation* anim = it.value();
            it = _fades.erase(it);
            if(anim)
                anim->deleteLater();
        }
        else
        {
            ++it;
        }
    }
}

void SoundboardMixer::handleSfxStatusChanged(int status)
{
    if(status != AudioTrack::AudioTrackStatus_Stop)
        return;

    AudioTrack* at = qobject_cast<AudioTrack*>(sender());
    if(!at)
        return;

    for(int i = _activeSfx.count() - 1; i >= 0; --i)
    {
        SoundboardTrack* sfx = _activeSfx.at(i).data();
        if((!sfx) || (sfx->getTrack() == at))
            _activeSfx.removeAt(i);
    }
}

SoundboardMixer::Channel SoundboardMixer::resolveChannel(SoundboardTrack* track, SoundboardGroup* parentGroup) const
{
    if(!track)
        return Channel_SFX;

    if(track->getPlaybackMode() == SoundboardTrack::PlaybackMode_OneShot)
        return Channel_SFX;

    if(parentGroup)
    {
        switch(parentGroup->getRole())
        {
            case SoundboardGroup::GroupRole_Music:   return Channel_Music;
            case SoundboardGroup::GroupRole_Ambient: return Channel_Ambient;
            case SoundboardGroup::GroupRole_SFX:     return Channel_SFX;
            case SoundboardGroup::GroupRole_Mixed:
            default:
                break;
        }
    }

    // Default for looping tracks with no guiding group role: Music.
    return Channel_Music;
}

void SoundboardMixer::setCurrentOnChannel(int channel, SoundboardTrack* track)
{
    if(channel == Channel_Music)
    {
        if(_currentMusic.data() == track)
            return;
        _currentMusic = track;
        emit currentMusicChanged(track);
    }
    else if(channel == Channel_Ambient)
    {
        if(_currentAmbient.data() == track)
            return;
        _currentAmbient = track;
        emit currentAmbientChanged(track);
    }
}

void SoundboardMixer::fadeOutAndStop(SoundboardTrack* track, int durationMs)
{
    if(!track)
        return;

    AudioTrack* at = track->getTrack();
    if(!at)
        return;

    cancelFadeFor(track);

    if(durationMs <= 0)
    {
        at->stop();
        return;
    }

    // Capture the current user-target volume (0..100) as the fade start.
    int startVol = track->getVolume();
    QVariantAnimation* anim = new QVariantAnimation(this);
    anim->setStartValue(toFloatVolume(startVol));
    anim->setEndValue(0.f);
    anim->setDuration(durationMs);

    QPointer<SoundboardTrack> guard(track);
    connect(anim, &QVariantAnimation::valueChanged, this, [guard](const QVariant& v) {
        if((!guard) || (!guard->getTrack()))
            return;
        guard->getTrack()->setVolume(v.toFloat());
    });
    connect(anim, &QVariantAnimation::finished, this, [this, guard]() {
        if(guard && guard->getTrack())
            guard->getTrack()->stop();
        if(guard)
            _fades.remove(guard.data());
    });

    _fades.insert(track, anim);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void SoundboardMixer::fadeIn(SoundboardTrack* track, int durationMs)
{
    if(!track)
        return;

    AudioTrack* at = track->getTrack();
    if(!at)
        return;

    cancelFadeFor(track);

    int targetVol = track->getVolume();
    if(durationMs <= 0)
    {
        at->setVolume(toFloatVolume(targetVol));
        return;
    }

    QVariantAnimation* anim = new QVariantAnimation(this);
    anim->setStartValue(0.f);
    anim->setEndValue(toFloatVolume(targetVol));
    anim->setDuration(durationMs);

    QPointer<SoundboardTrack> guard(track);
    connect(anim, &QVariantAnimation::valueChanged, this, [guard](const QVariant& v) {
        if((!guard) || (!guard->getTrack()))
            return;
        guard->getTrack()->setVolume(v.toFloat());
    });
    connect(anim, &QVariantAnimation::finished, this, [this, guard]() {
        if(guard)
            _fades.remove(guard.data());
    });

    _fades.insert(track, anim);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void SoundboardMixer::cancelFadeFor(SoundboardTrack* track)
{
    QMap<SoundboardTrack*, QVariantAnimation*>::iterator it = _fades.find(track);
    if(it == _fades.end())
        return;

    QVariantAnimation* anim = it.value();
    _fades.erase(it);
    if(anim)
    {
        anim->stop();
        anim->deleteLater();
    }
}
