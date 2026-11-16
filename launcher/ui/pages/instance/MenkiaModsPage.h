#pragma once

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QTreeWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QNetworkReply>
#include "ui/pages/BasePage.h"

class MinecraftInstance;

/*
 * MENKIA's per-instance Modrinth browser.
 * Every result is resolved against the selected Minecraft version and loader
 * before its jar is written to that instance's mods directory.
 */
class MenkiaModsPage : public QMainWindow, public BasePage
{
    Q_OBJECT
public:
    explicit MenkiaModsPage(MinecraftInstance *instance, QWidget *parent = nullptr);
    ~MenkiaModsPage() override = default;

    QString id() const override { return "menkia-mods"; }
    QString displayName() const override { return tr("MENKIA Mods & HUD"); }
    QIcon icon() const override { return QIcon(); }
    QString helpPage() const override { return QString(); }
    bool shouldDisplay() const override;
    void openedImpl() override;

private slots:
    void search();
    void installSelected();
    void preset(const QString &term);
    void onSearchFinished();
    void onVersionsFinished();
    void onDownloadFinished();

private:
    void setStatus(const QString &message);
    QString minecraftVersion() const;
    QString loaderId() const;
    void requestCompatibleVersion(const QString &projectId);
    void downloadFile(const QUrl &url, const QString &filename);

    MinecraftInstance *m_instance = nullptr;
    QNetworkAccessManager m_network;
    QLineEdit *m_search = nullptr;
    QComboBox *m_loader = nullptr;
    QTreeWidget *m_results = nullptr;
    QLabel *m_status = nullptr;
    QPushButton *m_install = nullptr;
    QByteArray m_pendingProjectId;
    QString m_pendingFilename;
    QUrl m_pendingDownload;
    QNetworkReply *m_activeReply = nullptr;
};
