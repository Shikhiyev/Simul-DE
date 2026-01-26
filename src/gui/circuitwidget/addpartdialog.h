/***************************************************************************
 *   Copyright (C) 2026 Antigravity                                        *
 ***************************************************************************/

#ifndef ADDPARTDIALOG_H
#define ADDPARTDIALOG_H

#include <QtWidgets>

// Ensure MAINMODULE_EXPORT is defined
#ifndef MAINMODULE_EXPORT
#    define MAINMODULE_EXPORT
#endif

struct PartItem {
    QString name;
    QString type;
    QIcon icon;
};

class ComponentTile : public QFrame
{
    Q_OBJECT
  public:
    ComponentTile(const PartItem &item, QWidget *parent = nullptr);
    PartItem item() const
    {
        return m_item;
    }

  signals:
    void clicked(const PartItem &item);

  protected:
    void mousePressEvent(QMouseEvent *event) override;

  private:
    PartItem m_item;
};

class MAINMODULE_EXPORT AddPartDialog : public QDialog
{
    Q_OBJECT
  public:
    static void showAddPart(QWidget *parent);

  private slots:
    void onSearchChanged(const QString &text);
    void onTileClicked(const PartItem &item);

  private:
    AddPartDialog(QWidget *parent = nullptr);
    void setupUi();
    void populateGrid(const QString &filter = "");
    void collectItems(QTreeWidgetItem *root, QList<PartItem> &list);

    QLineEdit *m_searchEdit;
    QScrollArea *m_scrollArea;
    QWidget *m_gridWidget;
    QGridLayout *m_gridLayout;
    QList<PartItem> m_allParts;

    static AddPartDialog *m_instance;
};

#endif // ADDPARTDIALOG_H
