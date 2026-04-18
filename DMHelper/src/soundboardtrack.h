#ifndef SOUNDBOARDTRACK_H
#define SOUNDBOARDTRACK_H

#include <QObject>

class AudioTrack;
class QDomElement;
class QDomDocument;
class QDir;

class SoundboardTrack : public QObject
{
    Q_OBJECT
public:
    enum PlaybackMode
    {
        PlaybackMode_Loop = 0,
        PlaybackMode_OneShot = 1
    };

    explicit SoundboardTrack(AudioTrack* track, int volume = 100, bool mute = false, PlaybackMode mode = PlaybackMode_Loop, QObject *parent = nullptr);
    virtual ~SoundboardTrack();

    //Public Interface
    AudioTrack* getTrack() const;
    int getAudioType() const;

    int getVolume() const;
    bool getMute() const;
    PlaybackMode getPlaybackMode() const;

    QString getTrackName() const;
    QString getTrackDetails() const;

signals:
    void trackLengthChanged(int length);
    void trackPositionChanged(int length);
    void volumeChanged(int volume);
    void muteChanged(bool mute);
    void playbackModeChanged(int mode);

public slots:
    void play();
    void stop();
    void setVolume(int volume);
    void setMute(bool mute);
    void setPlaybackMode(int mode);

protected:
    AudioTrack* _track;
    int _volume;
    bool _mute;
    PlaybackMode _playbackMode;

};

#endif // SOUNDBOARDTRACK_H
