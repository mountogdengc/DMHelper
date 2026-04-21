#include "audiotrackyoutube.h"
#include "dmconstants.h"
#include "dmversion.h"
#include <QDomDocument>
#include <QDomElement>
#include <QMessageBox>
#include <QIcon>
#include <QDebug>

const int AUDIOTRACKYOUTUBE_STOPCALLCOMPLETE = 0x01;
const int AUDIOTRACKYOUTUBE_STOPCALLCONFIRMED = 0x02;
const int AUDIOTRACKYOUTUBE_STOPCOMPLETE = AUDIOTRACKYOUTUBE_STOPCALLCOMPLETE | AUDIOTRACKYOUTUBE_STOPCALLCONFIRMED;

void youtubeEventCallback(const struct libvlc_event_t *p_event, void *p_data);

AudioTrackYoutube::AudioTrackYoutube(const QString& trackName, const QUrl& trackUrl, QObject *parent) :
    AudioTrackUrl(trackName, trackUrl, parent),
    _ytdlpProcess(nullptr),
    _ytdlpTimer(nullptr),
    _urlString(),
    _vlcPlayer(nullptr),
    _stopStatus(0),
    _volume(100),
    _timerId(0),
    _repeat(true),
    _mute(false)
{
}

AudioTrackYoutube::~AudioTrackYoutube()
{
    if(_ytdlpTimer)
    {
        _ytdlpTimer->stop();
        delete _ytdlpTimer;
        _ytdlpTimer = nullptr;
    }

    if(_ytdlpProcess)
    {
        _ytdlpProcess->kill();
        _ytdlpProcess->waitForFinished(1000);
        delete _ytdlpProcess;
        _ytdlpProcess = nullptr;
    }

    if(_timerId)
        killTimer(_timerId);

    AudioTrackYoutube::stop();
}

void AudioTrackYoutube::inputXML(const QDomElement &element, bool isImport)
{
    _volume = element.attribute("volume", QString::number(100)).toInt();
    _repeat = static_cast<bool>(element.attribute("repeat", QString::number(1)).toInt());
    _mute = static_cast<bool>(element.attribute("mute", QString::number(0)).toInt());

    AudioTrackUrl::inputXML(element, isImport);
}

void AudioTrackYoutube::copyValues(const CampaignObjectBase* other)
{
    const AudioTrackYoutube* otherAudioTrack = dynamic_cast<const AudioTrackYoutube*>(other);
    if(!otherAudioTrack)
        return;

    _volume = otherAudioTrack->_volume;
    _repeat = otherAudioTrack->_repeat;
    _mute = otherAudioTrack->_mute;

    AudioTrackUrl::copyValues(other);
}

QIcon AudioTrackYoutube::getDefaultIcon()
{
    return QIcon(":/img/data/icon_playerswindow.png");
}

int AudioTrackYoutube::getAudioType() const
{
    return DMHelper::AudioType_Youtube;
}

void AudioTrackYoutube::eventCallback(const struct libvlc_event_t *p_event)
{
    if(!p_event)
        return;

    if(p_event->type == libvlc_MediaPlayerStopped)
    {
        internalStopCheck(AUDIOTRACKYOUTUBE_STOPCALLCONFIRMED);
    }
    if(p_event->type == libvlc_MediaPlayerPlaying)
    {
        libvlc_time_t length_full = libvlc_media_player_get_length(_vlcPlayer);
        emit trackLengthChanged(length_full / 1000);
    }
}

int AudioTrackYoutube::getTrackStatus() const
{
    if(isPlaying())
        return AudioTrack::AudioTrackStatus_Play;
    else
        return AudioTrack::AudioTrackStatus_Stop;
}

bool AudioTrackYoutube::isPlaying() const
{
    return((_vlcPlayer) && (_stopStatus < AUDIOTRACKYOUTUBE_STOPCALLCOMPLETE));
}

bool AudioTrackYoutube::isRepeat() const
{
    return _repeat;
}

bool AudioTrackYoutube::isMuted() const
{
    return _mute;
}

float AudioTrackYoutube::getVolume() const
{
    return static_cast<float>(_volume) / 100.f;
}

void AudioTrackYoutube::play()
{
    // Check if the track is already playing
    if(isPlaying())
        return;

    if(_urlString.isEmpty())
        findDirectUrl(extractYoutubeIDFromUrl()); //findDirectUrl(QString("9bMTK0ml9ZI"));
    else
        playDirectUrl();
}

void AudioTrackYoutube::stop()
{
    if(!AudioTrackYoutube::isPlaying())
        return;

    _stopStatus = 0;
    libvlc_media_player_stop_async(_vlcPlayer);
    internalStopCheck(AUDIOTRACKYOUTUBE_STOPCALLCOMPLETE);
}

void AudioTrackYoutube::setMute(bool mute)
{
    if(mute == _mute)
        return;

    _mute = mute;

    if(isPlaying())
    {
        //_lastVolume = libvlc_audio_get_volume(_vlcPlayer);
        if(_mute)
            libvlc_audio_set_volume(_vlcPlayer, 0);
        else
            libvlc_audio_set_volume(_vlcPlayer, _volume);
    }

    emit dirty();
}

void AudioTrackYoutube::setVolume(float volume)
{
    float newVolume = static_cast<int>(volume * 100.f);

    if(newVolume == _volume)
        return;

    _volume = newVolume;

    if(isPlaying())
        libvlc_audio_set_volume(_vlcPlayer, _volume);

    emit dirty();
}

void AudioTrackYoutube::setRepeat(bool repeat)
{
    if(repeat == _repeat)
        return;

    _repeat = repeat;
    emit dirty();
}

void AudioTrackYoutube::ytdlpFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if(_ytdlpTimer)
    {
        _ytdlpTimer->stop();
        _ytdlpTimer->deleteLater();
        _ytdlpTimer = nullptr;
    }

    if(!_ytdlpProcess)
        return;

    if(isPlaying())
    {
        _ytdlpProcess->deleteLater();
        _ytdlpProcess = nullptr;
        return;
    }

    if(exitStatus != QProcess::NormalExit || exitCode != 0)
    {
        QString errorOutput = QString::fromUtf8(_ytdlpProcess->readAllStandardError()).trimmed();
        qDebug() << "[AudioTrackYoutube] yt-dlp failed with exit code" << exitCode << ":" << errorOutput;
        QMessageBox::critical(nullptr,
                              QString("DMHelper Audio Error"),
                              QString("Failed to resolve YouTube video URL:") + QChar::LineFeed + QChar::LineFeed + errorOutput);
    }
    else
    {
        _urlString = QString::fromUtf8(_ytdlpProcess->readAllStandardOutput()).trimmed();
        qDebug() << "[AudioTrackYoutube] yt-dlp resolved URL:" << _urlString.left(80) << "...";
        if(!_urlString.isEmpty())
            playDirectUrl();
    }

    _ytdlpProcess->deleteLater();
    _ytdlpProcess = nullptr;
}

void AudioTrackYoutube::ytdlpError(QProcess::ProcessError error)
{
    if(_ytdlpTimer)
    {
        _ytdlpTimer->stop();
        _ytdlpTimer->deleteLater();
        _ytdlpTimer = nullptr;
    }

    if(!_ytdlpProcess)
        return;

    QString errorMsg;
    if(error == QProcess::FailedToStart)
        errorMsg = QString("yt-dlp was not found. Please install yt-dlp to play YouTube audio.");
    else
        errorMsg = QString("yt-dlp process error: %1").arg(error);

    qDebug() << "[AudioTrackYoutube] yt-dlp process error:" << error;
    QMessageBox::critical(nullptr,
                          QString("DMHelper Audio Error"),
                          errorMsg);

    _ytdlpProcess->deleteLater();
    _ytdlpProcess = nullptr;
}

void AudioTrackYoutube::ytdlpTimeout()
{
    if(_ytdlpTimer)
    {
        _ytdlpTimer->deleteLater();
        _ytdlpTimer = nullptr;
    }

    if(!_ytdlpProcess)
        return;

    qDebug() << "[AudioTrackYoutube] yt-dlp timed out after 30 seconds";
    _ytdlpProcess->kill();
    _ytdlpProcess->deleteLater();
    _ytdlpProcess = nullptr;

    QMessageBox::critical(nullptr,
                          QString("DMHelper Audio Error"),
                          QString("YouTube URL resolution timed out. Please check your internet connection and try again."));
}

void AudioTrackYoutube::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("volume", QString::number(_volume));
    element.setAttribute("repeat", QString::number(_repeat));
    element.setAttribute("mute", QString::number(_mute));

    AudioTrackUrl::internalOutputXML(doc, element, targetDirectory, isExport);
}

void AudioTrackYoutube::findDirectUrl(const QString& youtubeId)
{
    Q_UNUSED(youtubeId);

    if(isPlaying())
        return;

    if(_ytdlpProcess)
        return;

    // Pass the original URL directly to yt-dlp — it handles all YouTube URL formats
    QString youtubeUrl = _url.toString();
    qDebug() << "[AudioTrackYoutube] Resolving YouTube URL via yt-dlp:" << youtubeUrl;

    _ytdlpProcess = new QProcess(this);
    connect(_ytdlpProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &AudioTrackYoutube::ytdlpFinished);
    connect(_ytdlpProcess, &QProcess::errorOccurred,
            this, &AudioTrackYoutube::ytdlpError);

    // Start a timeout timer to prevent indefinite hangs
    _ytdlpTimer = new QTimer(this);
    _ytdlpTimer->setSingleShot(true);
    connect(_ytdlpTimer, &QTimer::timeout, this, &AudioTrackYoutube::ytdlpTimeout);
    _ytdlpTimer->start(30000);

    _ytdlpProcess->start("yt-dlp", QStringList() << "--get-url" << "-f" << "b" << "--no-warnings" << "--no-check-certificates" << youtubeUrl);
}

void AudioTrackYoutube::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);

    if((!_vlcPlayer) || (_stopStatus == AUDIOTRACKYOUTUBE_STOPCOMPLETE))
    {
        if(_timerId)
        {
            killTimer(_timerId);
            _timerId = 0;
        }

        return;
    }

    libvlc_time_t currentTime = libvlc_media_player_get_time(_vlcPlayer);
    emit trackPositionChanged(currentTime / 1000);
}

void AudioTrackYoutube::playDirectUrl()
{
    if(isPlaying())
        return;

    libvlc_media_t *vlcMedia = libvlc_media_new_location(_urlString.toUtf8().constData());

    libvlc_media_add_option(vlcMedia, ":network-caching=500");
    libvlc_media_add_option(vlcMedia, ":no-video");

    _vlcPlayer = libvlc_media_player_new_from_media(DMH_VLC::vlcInstance(), vlcMedia);
    if(!_vlcPlayer)
        return;

    libvlc_event_manager_t* eventManager = libvlc_media_player_event_manager(_vlcPlayer);
    if(eventManager)
    {
        libvlc_event_attach(eventManager, libvlc_MediaPlayerOpening, youtubeEventCallback, this);
        libvlc_event_attach(eventManager, libvlc_MediaPlayerBuffering, youtubeEventCallback, this);
        libvlc_event_attach(eventManager, libvlc_MediaPlayerPlaying, youtubeEventCallback, this);
        libvlc_event_attach(eventManager, libvlc_MediaPlayerPaused, youtubeEventCallback, this);
        libvlc_event_attach(eventManager, libvlc_MediaPlayerStopped, youtubeEventCallback, this);
    }

    // Start playback
    _stopStatus = 0;
    if(_mute)
        libvlc_audio_set_volume(_vlcPlayer, 0);
    else
        libvlc_audio_set_volume(_vlcPlayer, _volume);
    libvlc_media_player_play(_vlcPlayer);

    if(_timerId == 0)
        _timerId = startTimer(500);
}

void AudioTrackYoutube::internalStopCheck(int status)
{
    _stopStatus |= status;

    qDebug() << "[AudioTrackYoutube] Internal Stop Check called with status " << status << ", overall status: " << _stopStatus;

    if((_stopStatus == AUDIOTRACKYOUTUBE_STOPCALLCONFIRMED) && (_repeat))
    {
        qDebug() << "[AudioTrackYoutube] Internal Stop Check: Audio ended, restarting playback";
        _stopStatus = 0;
        libvlc_media_player_release(_vlcPlayer);
        _vlcPlayer = nullptr;
        playDirectUrl();
        return;
    }

    if(_stopStatus != AUDIOTRACKYOUTUBE_STOPCOMPLETE)
        return;

    libvlc_media_player_release(_vlcPlayer);
    _vlcPlayer = nullptr;
}

QString AudioTrackYoutube::extractYoutubeIDFromUrl()
{
    if(_url.hasQuery())
    {
        QString urlQuery = _url.query();
        if(urlQuery.left(2) == QString("v="))
        {
            return urlQuery.remove(0, 2);
        }
    }

    return _url.toString();
}


/**
 * Callback function notification
 * \param p_event the event triggering the callback
 */
void youtubeEventCallback(const struct libvlc_event_t *p_event, void *p_data)
{
    if(!p_data)
        return;

    AudioTrackYoutube* player = static_cast<AudioTrackYoutube*>(p_data);
    player->eventCallback(p_event);
}
