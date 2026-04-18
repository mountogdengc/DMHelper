#ifndef SOUNDBOARDMIXER_H
#define SOUNDBOARDMIXER_H

#include <QObject>
#include <QPointer>
#include <QList>
#include <QMap>

class SoundboardTrack;
class SoundboardGroup;
class QVariantAnimation;

/*
 * SoundboardMixer coordinates simultaneous playback across three logical
 * channels so a DM can layer background music, ambient loops, and one-shot
 * SFX. Each SoundboardTrack already owns its underlying AudioTrack/
 * QMediaPlayer, so the mixer does not hold any media players of its own -
 * it routes play/stop requests to the right track(s) for a channel and
 * runs a volume crossfade when a channel's loop track changes.
 *
 * OneShot tracks bypass crossfade and are added to an active SFX list; they
 * are cleaned out when they stop.
 */
class SoundboardMixer : public QObject
{
    Q_OBJECT
public:
    enum Channel
    {
        Channel_Music = 0,
        Channel_Ambient = 1,
        Channel_SFX = 2
    };

    explicit SoundboardMixer(QObject* parent = nullptr);
    virtual ~SoundboardMixer();

    int getCrossfadeMs() const;

    SoundboardTrack* getCurrentMusic() const;
    SoundboardTrack* getCurrentAmbient() const;

public slots:
    void setCrossfadeMs(int ms);

    // Play a single track on an explicit channel. For Music/Ambient the
    // previous track on that channel is crossfaded out. For SFX the track
    // is added to the active one-shot pool.
    void playTrackOnChannel(int channel, SoundboardTrack* track);

    // Convenience: picks channel from track's playback mode and, if the
    // parent group has a definite role, the group role.
    void playTrack(SoundboardTrack* track, SoundboardGroup* parentGroup = nullptr);

    // Fade out (and then stop) whatever is playing on the channel.
    void stopChannel(int channel);

    // Start the music/ambient loops of a group based on the group's role.
    // SFX-typed tracks inside the group are NOT auto-played.
    void playGroup(SoundboardGroup* group);

    // Stop every channel (with crossfade for loops, immediate for SFX).
    void stopAll();

signals:
    void crossfadeMsChanged(int ms);
    void currentMusicChanged(SoundboardTrack* track);
    void currentAmbientChanged(SoundboardTrack* track);

protected slots:
    void handleTrackDestroyed(QObject* obj);
    void handleSfxStatusChanged(int status);

protected:
    Channel resolveChannel(SoundboardTrack* track, SoundboardGroup* parentGroup) const;
    void setCurrentOnChannel(int channel, SoundboardTrack* track);
    void fadeOutAndStop(SoundboardTrack* track, int durationMs);
    void fadeIn(SoundboardTrack* track, int durationMs);
    void cancelFadeFor(SoundboardTrack* track);

    int _crossfadeMs;
    QPointer<SoundboardTrack> _currentMusic;
    QPointer<SoundboardTrack> _currentAmbient;
    QList<QPointer<SoundboardTrack>> _activeSfx;
    QMap<SoundboardTrack*, QVariantAnimation*> _fades;
};

#endif // SOUNDBOARDMIXER_H
