/*
    SPDX-FileCopyrightText: 2026 tsiayigong
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "../shell/toolbarautohide.h"

#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QTest>
#include <QToolBar>

class ToolbarAutoHideTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void overlayAndKeyboard();
    void keepPopupOpen();
};

void ToolbarAutoHideTest::overlayAndKeyboard()
{
    QMainWindow window;
    auto *document = new QWidget;
    document->setFocusPolicy(Qt::StrongFocus);
    window.setCentralWidget(document);
    auto *toolbar = window.addToolBar(QStringLiteral("Reading"));
    auto *pageNumber = new QLineEdit;
    toolbar->addWidget(pageNumber);
    auto *documentTabs = window.addToolBar(QStringLiteral("Documents"));
    documentTabs->addAction(QStringLiteral("Document.pdf"));
    ToolbarAutoHide controller(&window, documentTabs);
    controller.attach(toolbar, document);
    window.resize(800, 600);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    controller.setEnabled(true);
    QCoreApplication::processEvents();
    QVERIFY(toolbar->isHidden());
    const QRect documentGeometry = document->geometry();

    auto *hotZone = window.findChild<QWidget *>(QStringLiteral("toolbarRevealArea"));
    QVERIFY(hotZone->isVisible());
    QTest::mouseMove(hotZone, QPoint(40, 2));
    QTRY_VERIFY(toolbar->isVisible());
    QCOMPARE(document->geometry(), documentGeometry);

    controller.reveal(true);
    QTRY_VERIFY(pageNumber->hasFocus());
    QTest::mouseMove(document, QPoint(400, 400));
    QTest::qWait(300);
    QVERIFY(toolbar->isVisible());
    QTest::keyClick(pageNumber, Qt::Key_Escape);
    QVERIFY(toolbar->isHidden());
    QCOMPARE(document->geometry(), documentGeometry);

    controller.reveal();
    QTest::mouseMove(toolbar, QPoint(40, 10));
    QTest::mouseMove(document, QPoint(400, 400));
    QTRY_VERIFY(toolbar->isHidden());
    controller.setEnabled(false);
    QTRY_VERIFY(toolbar->isVisible());
    QCOMPARE(window.toolBarArea(toolbar), Qt::TopToolBarArea);
    QVERIFY(toolbar->geometry().bottom() < documentTabs->geometry().top());
    QVERIFY(hotZone->isHidden());
}

void ToolbarAutoHideTest::keepPopupOpen()
{
    QMainWindow window;
    auto *document = new QWidget;
    window.setCentralWidget(document);
    auto *toolbar = window.addToolBar(QStringLiteral("Reading"));
    toolbar->addAction(QStringLiteral("Layout"));
    auto *documentTabs = window.addToolBar(QStringLiteral("Documents"));
    documentTabs->addAction(QStringLiteral("Document.pdf"));
    ToolbarAutoHide controller(&window, documentTabs);
    controller.attach(toolbar, document);
    window.resize(800, 600);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    controller.setEnabled(true);
    controller.reveal();
    QTest::mouseMove(toolbar, QPoint(40, 10));

    QMenu menu(toolbar);
    menu.addAction(QStringLiteral("Single page"));
    menu.popup(toolbar->mapToGlobal(QPoint(40, 40)));
    QTest::mouseMove(document, QPoint(400, 400));
    QTest::qWait(300);
    QVERIFY(toolbar->isVisible());
    menu.close();
    QTRY_VERIFY(toolbar->isHidden());
}

QTEST_MAIN(ToolbarAutoHideTest)
#include "toolbarautohidetest.moc"
