#ifndef SOUNDBOARDGROUP_H
#define SOUNDBOARDGROUP_H

#include <QObject>
#include <QUuid>

class SoundboardTrack;
class AudioTrack;
class Campaign;
class QDomElement;
class QDomDocument;
class QDir;

class SoundboardGroup : public QObject
{
    Q_OBJECT
public:
    enum GroupRole
    {
        GroupRole_Music = 0,
        GroupRole_Ambient = 1,
        GroupRole_SFX = 2,
        GroupRole_Mixed = 3
    };

    explicit SoundboardGroup(const QString& groupName, QObject *parent = nullptr);
    explicit SoundboardGroup(Campaign& campaign, const QDomElement& element, bool isImport);
    virtual ~SoundboardGroup();

    // Parallel to CampaignObjectBase
    void outputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport);

    QUuid getID() const;

    QString getGroupName() const;
    int getVolume() const;
    bool getMute() const;
    GroupRole getRole() const;

    QList<SoundboardTrack*> getTracks() const;
    void addTrack(SoundboardTrack* track);
    void addTrack(AudioTrack* track);
    void removeTrack(SoundboardTrack* track);

signals:
    void groupNameChanged(const QString& groupName);
    void volumeChanged(int volume);
    void muteChanged(bool mute);
    void roleChanged(int role);

public slots:
    void setGroupName(const QString& groupName);
    void setVolume(int volume);
    void setMute(bool mute);
    void setRole(int role);

protected:
    void internalPostProcessXML(Campaign& campaign, const QDomElement& element, bool isImport);

    QUuid _id;
    QString _groupName;
    int _volume;
    bool _mute;
    GroupRole _role;
    QList<SoundboardTrack*> _tracks;
};

#endif // SOUNDBOARDGROUP_H
