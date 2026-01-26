/***************************************************************************
 *   Copyright (C) 2026 Antigravity                                        *
 ***************************************************************************/

#include "addpartdialog.h"

#include "circuit.h"
#include "circuitview.h"
#include "componentselector.h"
#include "mainwindow.h"

#include <QApplication>
#include <QDrag>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMimeData>
#include <QMouseEvent>
#include <QScrollArea>
#include <QVBoxLayout>


ComponentTile::ComponentTile(const PartItem &item, QWidget *parent) :
    QFrame(parent), m_item(item)
{
    setFixedSize(110, 120);
    setFrameShape(QFrame::StyledPanel);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 12, 8, 8);
    layout->setSpacing(8);

    QLabel *iconLabel = new QLabel(this);
    QIcon icon        = item.icon;
    if (icon.isNull()) {
        icon = QIcon(":/null-0.png");
    }
    iconLabel->setPixmap(icon.pixmap(54, 54));
    iconLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(iconLabel);

    QLabel *nameLabel = new QLabel(item.name, this);
    nameLabel->setAlignment(Qt::AlignCenter);
    nameLabel->setWordWrap(true);
    nameLabel->setStyleSheet(
        "font-size: 11px; font-weight: 500; color: #1a1a1a;");
    layout->addWidget(nameLabel, 0, Qt::AlignBottom);

    setCursor(Qt::PointingHandCursor);
    setStyleSheet("ComponentTile {"
                  "  background-color: #ffffff;"
                  "  border: 1px solid #e0e0e0;"
                  "  border-radius: 12px;"
                  "}"
                  "ComponentTile:hover {"
                  "  background-color: #f0f7ff;"
                  "  border: 2px solid #007bff;"
                  "}");
}

void ComponentTile::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit clicked(m_item);
    }
}

AddPartDialog *AddPartDialog::m_instance = nullptr;

void AddPartDialog::showAddPart(QWidget *parent)
{
    if (!m_instance) {
        m_instance = new AddPartDialog(parent);
    }
    m_instance->m_searchEdit->clear();

    m_instance->m_allParts.clear();
    ComponentSelector *selector = ComponentSelector::self();
    if (selector) {
        m_instance->collectItems(selector->invisibleRootItem(),
                                 m_instance->m_allParts);
    }

    m_instance->populateGrid();
    m_instance->show();
    m_instance->raise();
    m_instance->activateWindow();
}

AddPartDialog::AddPartDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Add New Part"));
    setMinimumSize(600, 500);
    setupUi();
}

void AddPartDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(10);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(tr("Search parts..."));
    m_searchEdit->setFixedHeight(35);
    m_searchEdit->setStyleSheet("QLineEdit {"
                                "  padding: 5px 15px;"
                                "  border: 2px solid #ddd;"
                                "  border-radius: 17px;"
                                "  font-size: 14px;"
                                "  background: white;"
                                "}"
                                "QLineEdit:focus {"
                                "  border-color: #4a90e2;"
                                "}");
    mainLayout->addWidget(m_searchEdit);
    connect(m_searchEdit, &QLineEdit::textChanged, this,
            &AddPartDialog::onSearchChanged);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setStyleSheet("background: transparent;");

    m_gridWidget = new QWidget();
    m_gridLayout = new QGridLayout(m_gridWidget);
    m_gridLayout->setSpacing(15);
    m_gridLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    m_scrollArea->setWidget(m_gridWidget);
    mainLayout->addWidget(m_scrollArea);

    setStyleSheet("QDialog { background-color: white; }");
}

void AddPartDialog::collectItems(QTreeWidgetItem *root, QList<PartItem> &list)
{
    for (int i = 0; i < root->childCount(); ++i) {
        QTreeWidgetItem *item = root->child(i);
        QString type          = item->data(0, Qt::UserRole).toString();

        if (!type.isEmpty()) {
            PartItem part;
            part.type = type;
            part.name = item->data(0, Qt::WhatsThisRole).toString();
            if (part.name.isEmpty())
                part.name = item->text(0).split("???").first();
            part.icon = item->icon(0);

            // Check if already in list to avoid duplicates
            bool exists = false;
            for (const auto &p : list)
                if (p.name == part.name && p.type == part.type) {
                    exists = true;
                    break;
                }

            if (!exists)
                list.append(part);
        }
        collectItems(item, list);
    }
}

void AddPartDialog::populateGrid(const QString &filter)
{
    QLayoutItem *item;
    while ((item = m_gridLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }

    int row     = 0;
    int col     = 0;
    int maxCols = 5;

    for (const PartItem &part : m_allParts) {
        if (!filter.isEmpty() &&
            !part.name.contains(filter, Qt::CaseInsensitive)) {
            continue;
        }

        ComponentTile *tile = new ComponentTile(part, m_gridWidget);
        connect(tile, &ComponentTile::clicked, this,
                &AddPartDialog::onTileClicked);
        m_gridLayout->addWidget(tile, row, col);

        col++;
        if (col >= maxCols) {
            col = 0;
            row++;
        }
    }
}

void AddPartDialog::onSearchChanged(const QString &text)
{
    populateGrid(text);
}

void AddPartDialog::onTileClicked(const PartItem &item)
{
    hide();

    QMimeData *mimeData = new QMimeData;
    mimeData->setText(item.name);
    mimeData->setHtml(item.type);

    QDrag *drag = new QDrag(MainWindow::self());
    drag->setMimeData(mimeData);
    drag->setHotSpot(QPoint(24, 24));

    // Use the icon from the part for the drag pixmap
    if (!item.icon.isNull()) {
        drag->setPixmap(item.icon.pixmap(32, 24));
    }

    drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::CopyAction);
}

#include "moc_addpartdialog.cpp"
