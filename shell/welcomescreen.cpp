/*
    SPDX-FileCopyrightText: 2021 Jiří Wolker <woljiri@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "welcomescreen.h"

#include <KConfigGroup>
#include <KIO/OpenFileManagerWindowJob>
#include <KIconLoader>
#include <KSharedConfig>

#include <QAction>
#include <QClipboard>
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QPainter>
#include <QResizeEvent>
#include <QStyledItemDelegate>

#include "gui/recentitemsmodel.h"

class RecentsListItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit RecentsListItemDelegate(WelcomeScreen *welcomeScreen)
        : m_welcomeScreen(welcomeScreen)
    {
    }

    WelcomeScreen *welcomeScreen() const
    {
        return m_welcomeScreen;
    }

    QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const override
    {
        return QSize(320, 72);
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);
        const QRect card = option.rect.adjusted(1, 4, -1, -4);
        QColor background = QApplication::palette().color(QPalette::Base);
        QColor border = option.palette.color(QPalette::WindowText);
        border.setAlpha(25);
        if (option.state.testFlag(QStyle::State_Selected) || option.state.testFlag(QStyle::State_MouseOver)) {
            background = option.palette.color(QPalette::Highlight);
            background.setAlpha(28);
            border = option.palette.color(QPalette::Highlight);
        } else {
            background.setAlpha(200);
        }
        painter->setPen(border);
        painter->setBrush(background);
        painter->drawRoundedRect(card, 4, 4);
        qvariant_cast<QIcon>(index.data(Qt::DecorationRole)).paint(painter, QRect(card.left() + 14, card.center().y() - 14, 28, 28));
        const QRect textArea = card.adjusted(58, 10, -14, -8);
        painter->setFont(option.font);
        painter->setPen(option.palette.color(QPalette::Text));
        painter->drawText(textArea, Qt::AlignTop | Qt::AlignLeft, option.fontMetrics.elidedText(index.data().toString(), Qt::ElideMiddle, textArea.width()));
        const QUrl url(index.data(RecentItemsModel::UrlRole).toString());
        const QString location = url.isLocalFile() ? QDir::toNativeSeparators(QFileInfo(url.toLocalFile()).absolutePath()) : url.host();
        painter->setPen(option.palette.color(QPalette::PlaceholderText));
        painter->drawText(textArea, Qt::AlignBottom | Qt::AlignLeft, option.fontMetrics.elidedText(location, Qt::ElideMiddle, textArea.width()));
        painter->restore();
    }

    bool editorEvent(QEvent *event, QAbstractItemModel *aModel, const QStyleOptionViewItem &styleOptionViewItem, const QModelIndex &index) override
    {
        const RecentItemsModel *model = static_cast<RecentItemsModel *>(aModel);
        const RecentItemsModel::RecentItem *item = model->getItem(index);

        QPoint menuPosition;

        if (item != nullptr) {
            bool willOpenMenu = false;
            if (event->type() == QEvent::ContextMenu) {
                willOpenMenu = true;
                menuPosition = static_cast<QContextMenuEvent *>(event)->globalPos();
            }
            if (event->type() == QEvent::MouseButtonPress) {
                if (static_cast<QMouseEvent *>(event)->button() == Qt::MouseButton::RightButton) {
                    willOpenMenu = true;
                    menuPosition = static_cast<QMouseEvent *>(event)->globalPosition().toPoint();
                }
            }

            if (willOpenMenu) {
                event->accept();

                QMenu menu;

                QAction *copyPathAction = new QAction(i18n("&Copy Path"));
                copyPathAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-copy")));
                connect(copyPathAction, &QAction::triggered, this, [item]() {
                    QString path;
                    if (item->url.isLocalFile()) {
                        path = item->url.toLocalFile();
                    } else {
                        path = item->url.toString();
                    }
                    QGuiApplication::clipboard()->setText(path);
                });
                menu.addAction(copyPathAction);

                QAction *showDirectoryAction = new QAction(i18n("&Open Containing Folder"));
                showDirectoryAction->setIcon(QIcon::fromTheme(QStringLiteral("document-open-folder")));
                connect(showDirectoryAction, &QAction::triggered, this, [item]() {
                    if (item->url.isLocalFile()) {
                        KIO::highlightInFileManager({item->url});
                    }
                });
                menu.addAction(showDirectoryAction);
                if (!item->url.isLocalFile()) {
                    showDirectoryAction->setEnabled(false);
                }

                QAction *forgetItemAction = new QAction(i18nc("recent items context menu", "&Forget This Item"));
                forgetItemAction->setIcon(QIcon::fromTheme(QStringLiteral("edit-clear-history")));
                connect(forgetItemAction, &QAction::triggered, this, [this, item]() { Q_EMIT welcomeScreen()->forgetRecentItem(item->url); });
                menu.addAction(forgetItemAction);

                menu.exec(menuPosition);

                return true;
            }
        }

        return QStyledItemDelegate::editorEvent(event, aModel, styleOptionViewItem, index);
    }

private:
    WelcomeScreen *m_welcomeScreen;
};

WelcomeScreen::WelcomeScreen(QWidget *parent)
    : QWidget(parent)
    , m_recentsModel(new RecentItemsModel)
    , m_recentsItemDelegate(new RecentsListItemDelegate(this))
{
    Q_ASSERT(parent);

    setupUi(this);
    setObjectName(QStringLiteral("welcomeScreen"));

    connect(openButton, &QPushButton::clicked, this, &WelcomeScreen::openClicked);

    recentsListView->setContextMenuPolicy(Qt::DefaultContextMenu);
    recentsListView->setModel(m_recentsModel);
    recentsListView->setItemDelegate(m_recentsItemDelegate);
    recentsListView->setMouseTracking(true);
    connect(recentsListView, &QListView::activated, this, &WelcomeScreen::recentsItemActivated);

    connect(m_recentsModel, &RecentItemsModel::layoutChanged, this, &WelcomeScreen::recentListChanged);

    connect(forgetAllButton, &QToolButton::clicked, this, &WelcomeScreen::forgetAllRecents);
}

WelcomeScreen::~WelcomeScreen()
{
    delete m_recentsModel;
    delete m_recentsItemDelegate;
}

void WelcomeScreen::showEvent(QShowEvent *e)
{
    if (appIcon->pixmap(Qt::ReturnByValue).isNull()) {
        appIcon->setPixmap(QIcon::fromTheme(QStringLiteral("okular")).pixmap(96, 96));
    }

    QWidget::showEvent(e);
}

void WelcomeScreen::loadRecents()
{
    m_recentsModel->loadEntries(KSharedConfig::openConfig()->group(QStringLiteral("Recent Files")));
}

void WelcomeScreen::setMaxRecentItems(const int maxItems)
{
    m_recentsModel->setMaxItems(maxItems);
}

int WelcomeScreen::recentsCount()
{
    return m_recentsModel->rowCount();
}

void WelcomeScreen::recentsItemActivated(const QModelIndex &index)
{
    const RecentItemsModel::RecentItem *item = m_recentsModel->getItem(index);
    if (item != nullptr) {
        Q_EMIT recentItemClicked(item->url);
    }
}

void WelcomeScreen::recentListChanged()
{
    const bool hasRecents = recentsCount() > 0;
    recentPanel->setVisible(hasRecents);
    divider->setVisible(hasRecents);
    emptyDropArea->setVisible(!hasRecents);
    forgetAllButton->setEnabled(hasRecents);
    welcomeTitle->setText(hasRecents ? i18n("Welcome to Okular") : i18n("Start Reading"));
    welcomeDescription->setText(hasRecents ? i18n("Read, annotate and explore your documents.\nOpen a file to get started.")
                                         : i18n("Your recently opened documents will appear here."));
    const auto alignment = hasRecents ? Qt::AlignLeft : Qt::AlignHCenter;
    appIcon->setAlignment(alignment);
    welcomeTitle->setAlignment(alignment);
    welcomeDescription->setAlignment(alignment);
    openShortcut->setAlignment(alignment);
    introLayout->setAlignment(appIcon, alignment);
    introLayout->setAlignment(openButton, alignment);
}

#include "welcomescreen.moc"
