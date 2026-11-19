#include "MenkiaModsPage.h"

#include "minecraft/MinecraftInstance.h"
#include "minecraft/PackProfile.h"
#include "Application.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QSaveFile>
#include <QUrlQuery>
#include <QDesktopServices>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QCryptographicHash>

MenkiaModsPage::MenkiaModsPage(MinecraftInstance *instance, QWidget *parent)
    : QMainWindow(parent), m_instance(instance), m_network(this)
{
    auto *root = new QWidget(this);
    auto *layout = new QVBoxLayout(root);
    auto *toolbar = new QHBoxLayout();

    m_search = new QLineEdit(root);
    m_search->setPlaceholderText(tr("Buscar mods, optimización o HUD..."));
    toolbar->addWidget(m_search, 1);

    m_loader = new QComboBox(root);
    const QString detectedLoader = loaderId();
    if (detectedLoader.isEmpty()) m_loader->addItem(tr("Sin loader compatible detectado"), QString());
    else m_loader->addItem(detectedLoader, detectedLoader);
    m_loader->setEnabled(false);
    m_loader->setToolTip(tr("El loader se detecta desde la configuración real de esta instancia."));
    toolbar->addWidget(m_loader);

    auto *searchButton = new QPushButton(tr("Buscar"), root);
    toolbar->addWidget(searchButton);
    layout->addLayout(toolbar);

    auto *presets = new QHBoxLayout();
    const QList<QPair<QString, QString>> choices = {
        {tr("Optimización"), "performance"},
        {tr("Keystrokes"), "keystrokes"},
        {tr("CPS"), "cps"},
        {tr("FPS / Ping"), "fps ping"},
        {tr("HUD"), "hud"}
    };
    for (const auto &choice : choices) {
        auto *button = new QPushButton(choice.first, root);
        connect(button, &QPushButton::clicked, this, [this, choice]() { preset(choice.second); });
        presets->addWidget(button);
    }
    layout->addLayout(presets);

    m_results = new QTreeWidget(root);
    m_results->setColumnCount(3);
    m_results->setHeaderLabels({tr("Mod"), tr("Autor"), tr("Descargas")});
    m_results->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_results->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_results->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    layout->addWidget(m_results, 1);

    auto *footer = new QHBoxLayout();
    m_status = new QLabel(tr("Los mods se instalarán únicamente en esta instancia."), root);
    footer->addWidget(m_status, 1);
    m_install = new QPushButton(tr("Instalar seleccionado"), root);
    m_install->setEnabled(false);
    footer->addWidget(m_install);
    layout->addLayout(footer);

    setCentralWidget(root);
    connect(searchButton, &QPushButton::clicked, this, &MenkiaModsPage::search);
    connect(m_search, &QLineEdit::returnPressed, this, &MenkiaModsPage::search);
    connect(m_results, &QTreeWidget::itemSelectionChanged, this, [this]() {
        m_install->setEnabled(!m_results->selectedItems().isEmpty());
    });
    connect(m_install, &QPushButton::clicked, this, &MenkiaModsPage::installSelected);
    connect(&m_network, &QNetworkAccessManager::finished, this, [this](QNetworkReply *reply) {
        if (reply != m_activeReply) return;
        if (reply->property("menkia_stage").toString() == "search") onSearchFinished();
        else if (reply->property("menkia_stage").toString() == "versions") onVersionsFinished();
        else if (reply->property("menkia_stage").toString() == "download") onDownloadFinished();
    });
}

bool MenkiaModsPage::shouldDisplay() const
{
    return m_instance != nullptr;
}

void MenkiaModsPage::openedImpl()
{
    if (m_results->topLevelItemCount() == 0) search();
}

QString MenkiaModsPage::minecraftVersion() const
{
    if (!m_instance || !m_instance->getPackProfile()) return QString();
    return m_instance->getPackProfile()->getComponentVersion("net.minecraft");
}

QString MenkiaModsPage::loaderId() const
{
    auto profile = m_instance ? m_instance->getPackProfile() : nullptr;
    if (!profile) return QString();
    if (profile->getComponent("net.fabricmc.fabric-loader")) return "fabric";
    if (profile->getComponent("org.quiltmc.quilt-loader")) return "quilt";
    if (profile->getComponent("net.neoforged")) return "neoforge";
    if (profile->getComponent("net.minecraftforge")) return "forge";
    return QString();
}

void MenkiaModsPage::setStatus(const QString &message)
{
    m_status->setText(message);
}

void MenkiaModsPage::preset(const QString &term)
{
    m_search->setText(term);
    search();
}

void MenkiaModsPage::search()
{
    if (m_activeReply) {
        m_activeReply->abort();
        m_activeReply = nullptr;
    }
    m_results->clear();
    const QString version = minecraftVersion();
    if (version.isEmpty()) {
        setStatus(tr("No se pudo detectar la versión de Minecraft de esta instancia."));
        return;
    }

    QUrl url("https://api.modrinth.com/v2/search");
    QUrlQuery query;
    query.addQueryItem("query", m_search->text().trimmed());
    query.addQueryItem("limit", "50");
    query.addQueryItem("index", "relevance");
    QString facets = QString(R"([["project_type:mod"],["versions:%1"]])").arg(version);
    const QString detectedLoader = loaderId();
    if (!detectedLoader.isEmpty())
        facets = QString(R"([["project_type:mod"],["versions:%1"],["categories:%2"]])").arg(version, detectedLoader);
    query.addQueryItem("facets", facets);
    url.setQuery(query);
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "MENKIA-Client/1.0 (https://github.com/Laky912/MENKIA-LAUNCHER)");
    m_activeReply = m_network.get(request);
    m_activeReply->setProperty("menkia_stage", "search");
    setStatus(tr("Buscando mods compatibles con Minecraft %1...").arg(version));
}

void MenkiaModsPage::onSearchFinished()
{
    auto *reply = m_activeReply;
    m_activeReply = nullptr;
    if (!reply) return;
    const auto error = reply->error();
    const QByteArray data = reply->readAll();
    const QString errorText = reply->errorString();
    reply->deleteLater();
    if (error != QNetworkReply::NoError) {
        setStatus(tr("Error al consultar Modrinth: %1").arg(errorText));
        return;
    }
    QJsonParseError parseError;
    const auto document = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        setStatus(tr("Modrinth devolvió una respuesta no válida."));
        return;
    }
    const auto hits = document.object().value("hits").toArray();
    for (const auto &value : hits) {
        const auto mod = value.toObject();
        auto *item = new QTreeWidgetItem(m_results);
        item->setText(0, mod.value("title").toString());
        item->setText(1, mod.value("author").toString());
        item->setText(2, QString::number(mod.value("downloads").toVariant().toLongLong()));
        item->setData(0, Qt::UserRole, mod.value("project_id").toString());
        item->setToolTip(0, mod.value("description").toString());
    }
    setStatus(tr("%1 mods encontrados para Minecraft %2. Selecciona uno para instalar.")
              .arg(hits.size()).arg(minecraftVersion()));
}

void MenkiaModsPage::installSelected()
{
    const auto selected = m_results->selectedItems();
    if (selected.isEmpty()) return;
    const QString projectId = selected.first()->data(0, Qt::UserRole).toString();
    if (projectId.isEmpty()) return;
    requestCompatibleVersion(projectId);
}

void MenkiaModsPage::requestCompatibleVersion(const QString &projectId)
{
    QUrl url(QString("https://api.modrinth.com/v2/project/%1/version").arg(projectId));
    QUrlQuery query;
    query.addQueryItem("game_versions", QString("[\"%1\"]").arg(minecraftVersion()));
    const QString loader = loaderId();
    if (loader.isEmpty()) {
        setStatus(tr("No se instalarán mods: esta instancia no tiene un loader compatible detectado. Instala Forge, Fabric, Quilt o NeoForge primero."));
        return;
    }
    query.addQueryItem("loaders", QString(R"(["%1"])").arg(loader));
    url.setQuery(query);
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "MENKIA-Client/1.0 (https://github.com/Laky912/MENKIA-LAUNCHER)");
    m_activeReply = m_network.get(request);
    m_activeReply->setProperty("menkia_stage", "versions");
    setStatus(tr("Comprobando versiones, loader y archivos compatibles..."));
}

void MenkiaModsPage::onVersionsFinished()
{
    auto *reply = m_activeReply;
    m_activeReply = nullptr;
    if (!reply) return;
    const auto error = reply->error();
    const QByteArray data = reply->readAll();
    const QString errorText = reply->errorString();
    reply->deleteLater();
    if (error != QNetworkReply::NoError) {
        setStatus(tr("No se pudieron consultar las versiones: %1").arg(errorText));
        return;
    }
    const auto versions = QJsonDocument::fromJson(data).array();
    for (const auto &versionValue : versions) {
        const auto version = versionValue.toObject();
        bool gameCompatible = false, loaderCompatible = false;
        for (const auto &v : version.value("game_versions").toArray()) if (v.toString() == minecraftVersion()) gameCompatible = true;
        for (const auto &v : version.value("loaders").toArray()) if (v.toString() == loaderId()) loaderCompatible = true;
        if (!gameCompatible || !loaderCompatible) continue;
        for (const auto &dependency : version.value("dependencies").toArray()) {
            if (dependency.toObject().value("dependency_type").toString() == "required") {
                setStatus(tr("Este mod declara dependencias obligatorias. Instálalas manualmente desde Modrinth antes de usarlo."));
                return;
            }
        }
        const auto files = version.value("files").toArray();
        for (const auto &fileValue : files) {
            const auto file = fileValue.toObject();
            if (!file.value("primary").toBool()) continue;
            const QString suppliedName = file.value("filename").toString();
            const QString filename = QFileInfo(suppliedName).fileName();
            const QString sha512 = file.value("hashes").toObject().value("sha512").toString().toLower();
            const QUrl download(file.value("url").toString());
            if (filename == suppliedName && filename.endsWith(".jar", Qt::CaseInsensitive)
                && !sha512.isEmpty() && download.isValid() && download.scheme() == "https") {
                downloadFile(download, filename, sha512);
                return;
            }
        }
    }
    setStatus(tr("No existe una versión instalable para esta combinación de Minecraft y loader."));
}

void MenkiaModsPage::downloadFile(const QUrl &url, const QString &filename, const QString &sha512)
{
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "MENKIA-Client/1.0");
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    m_pendingFilename = filename;
    m_pendingSha512 = sha512;
    m_pendingDownload = url;
    m_activeReply = m_network.get(request);
    m_activeReply->setProperty("menkia_stage", "download");
    connect(m_activeReply, &QNetworkReply::downloadProgress, this, [this](qint64 received, qint64) {
        if (received > 100LL * 1024LL * 1024LL && m_activeReply) {
            m_activeReply->abort();
            setStatus(tr("Descarga cancelada: el archivo supera el límite de 100 MiB."));
        }
    });
    setStatus(tr("Descargando %1 (máximo 100 MiB)...").arg(filename));
}

void MenkiaModsPage::onDownloadFinished()
{
    auto *reply = m_activeReply;
    m_activeReply = nullptr;
    if (!reply) return;
    const auto error = reply->error();
    const QByteArray bytes = reply->readAll();
    const QString errorText = reply->errorString();
    reply->deleteLater();
    if (error != QNetworkReply::NoError || bytes.isEmpty()) {
        setStatus(tr("Falló la descarga: %1").arg(errorText));
        return;
    }
    if (bytes.size() > 100LL * 1024LL * 1024LL) {
        setStatus(tr("Descarga rechazada: supera el límite de 100 MiB."));
        return;
    }
    const QByteArray actualHash = QCryptographicHash::hash(bytes, QCryptographicHash::Sha512).toHex();
    if (QString::fromLatin1(actualHash) != m_pendingSha512) {
        setStatus(tr("Descarga rechazada: la verificación SHA-512 no coincide con Modrinth."));
        return;
    }
    if (!m_instance || m_instance->isRunning()) {
        setStatus(tr("Cierra Minecraft antes de instalar mods."));
        return;
    }
    QDir modsDir(m_instance->modsRoot());
    if (!modsDir.exists() && !modsDir.mkpath(".")) {
        setStatus(tr("No se pudo crear la carpeta mods de esta instancia."));
        return;
    }
    const QString target = modsDir.filePath(m_pendingFilename);
    if (QFileInfo::exists(target)) {
        const auto answer = QMessageBox::question(this, tr("Reemplazar mod"),
            tr("Ya existe %1 en esta instancia. ¿Quieres reemplazarlo?").arg(m_pendingFilename),
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (answer != QMessageBox::Yes) {
            setStatus(tr("Instalación cancelada; el archivo existente se conservó."));
            return;
        }
    }
    QSaveFile output(target);
    if (!output.open(QIODevice::WriteOnly) || output.write(bytes) != bytes.size() || !output.commit()) {
        setStatus(tr("No se pudo guardar el mod en la instancia."));
        return;
    }
    setStatus(tr("Instalado: %1. Revisa sus dependencias antes de iniciar.").arg(m_pendingFilename));
}
