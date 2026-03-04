/***************************************************************************
 *   Copyright (C) 2012 by santiago González                               *
 *   santigoro@gmail.com                                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>.  *
 *                                                                         *
 ***************************************************************************/

#include "mainwindow.h"

#include "appiface.h"
#include "circuit.h"
#include "circuitwidget.h"
#include "componentselector.h"
#include "editorwindow.h"
#include "filewidget.h"
#include "propertieswidget.h"
#include "simuapi_apppath.h"
#include "utils.h"

MainWindow *MainWindow::m_pSelf = 0l;

MainWindow::MainWindow() :
    QMainWindow(),
    m_settings(QStandardPaths::standardLocations(QStandardPaths::DataLocation)
                       .first() +
                   "/simulide.ini",
               QSettings::IniFormat, this)
{
    setWindowIcon(QIcon(":/simulide.png"));
    m_pSelf   = this;
    m_circuit = 0l;
    m_version = "SimulIDE-" + QString(APP_VERSION);

    this->setWindowTitle(m_version);

    QString userAddonPath =
        SIMUAPI_AppPath::self()->RWDataFolder().absoluteFilePath("addons");

    QDir pluginsDir(userAddonPath);

    if (!pluginsDir.exists())
        pluginsDir.mkpath(userAddonPath);

    m_fontScale = 1.0;
    if (m_settings.contains("fontScale")) {
        m_fontScale = m_settings.value("fontScale").toFloat();
        if (m_fontScale == 0)
            m_fontScale = 1;
    } else {
        double dpiX = qApp->desktop()->logicalDpiX();
        m_fontScale = dpiX / 96.0;
    }

    m_darkMode = m_settings.value("darkMode", false).toBool();
    // qDebug()<<dpiX;
    loadCircHelp();
    createWidgets();
    readSettings();

    setTheme(m_darkMode);

    loadPlugins();

    QString backPath = m_settings.value("backupPath").toString();
    if (!backPath.isEmpty()) {
        // qDebug() << "MainWindow::readSettings" << backPath;
        if (QFile::exists(backPath))
            CircuitWidget::self()->loadCirc(backPath);
    }
}
MainWindow::~MainWindow() {}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!m_editor->close()) {
        event->ignore();
        return;
    }
    if (!m_circuit->newCircuit()) {
        event->ignore();
        return;
    }

    writeSettings();

    event->accept();
}

void MainWindow::readSettings()
{
    restoreGeometry(m_settings.value("geometry").toByteArray());
    restoreState(m_settings.value("windowState").toByteArray());
    m_Centralsplitter->restoreState(
        m_settings.value("Centralsplitter/geometry").toByteArray());

    int autoBck = 15;
    if (m_settings.contains("autoBck"))
        autoBck = m_settings.value("autoBck").toInt();
    Circuit::self()->setAutoBck(autoBck);
}

void MainWindow::writeSettings()
{
    m_settings.setValue("autoBck", m_autoBck);
    m_settings.setValue("fontScale", m_fontScale);
    m_settings.setValue("darkMode", m_darkMode);
    m_settings.setValue("geometry", saveGeometry());
    m_settings.setValue("windowState", saveState());
    m_settings.setValue("Centralsplitter/geometry",
                        m_Centralsplitter->saveState());

    QList<QTreeWidgetItem *> list =
        m_components->findItems("", Qt::MatchStartsWith | Qt::MatchRecursive);

    for (QTreeWidgetItem *item : list)
        m_settings.setValue(item->text(0) + "/collapsed", !item->isExpanded());

    FileWidget::self()->writeSettings();
}

QString MainWindow::loc()
{
    return Circuit::self()->loc();
}

void MainWindow::setLoc(QString loc)
{
    Circuit::self()->setLoc(loc);
}

int MainWindow::autoBck()
{
    return m_autoBck;
}

void MainWindow::setAutoBck(int secs)
{
    m_autoBck = secs;
}

void MainWindow::setTitle(QString title)
{
    setWindowTitle(m_version + "  -  " + title);
}

void MainWindow::createWidgets()
{
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setObjectName("centralWidget");
    setCentralWidget(centralWidget);

    QGridLayout *baseWidgetLayout = new QGridLayout(centralWidget);
    baseWidgetLayout->setSpacing(0);
    baseWidgetLayout->setContentsMargins(0, 0, 0, 0);
    baseWidgetLayout->setObjectName("gridLayout");

    m_Centralsplitter = new QSplitter(this);
    m_Centralsplitter->setObjectName("Centralsplitter");
    m_Centralsplitter->setOrientation(Qt::Horizontal);

    // iconRail removed as requested

    // --- Main Layout Structure ---
    // [Icon Rail] [Splitter]

    // 1. Components List (The "Categories" panel)
    m_components = new ComponentSelector(this);
    m_components->setObjectName("components");
    m_components->show(); // Make sure it's visible now
    m_Centralsplitter->addWidget(m_components);

    // 2. Circuit Canvas (Center)
    m_circuit = new CircuitWidget(this);
    m_circuit->setObjectName("circuit");
    m_Centralsplitter->addWidget(m_circuit);

    // --- Header ---
    QWidget *header = new QWidget(centralWidget);
    header->setObjectName("header");
    header->setFixedHeight(50);
    header->setStyleSheet(
        "QWidget#header { background-color: white; border-bottom: 1px solid "
        "#e0e0e0; }");

    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(15, 0, 15, 0);
    headerLayout->setSpacing(8);

    QLabel *logo = new QLabel(header);
    logo->setPixmap(QIcon(":/simulide.png").pixmap(30, 30));
    headerLayout->addWidget(logo);

    headerLayout->addSpacing(15);

    // -- File Operations Group --
    auto createHeaderBtn = [&](const QString &icon, const QString &text = "",
                               const QString &tooltip = "") {
        QPushButton *btn = new QPushButton(header);
        if (!text.isEmpty())
            btn->setText(text);
        btn->setIcon(QIcon(icon));
        if (!tooltip.isEmpty())
            btn->setToolTip(tooltip);

        QString style =
            "QPushButton { border: 1px solid #e0e0e0; border-radius: 6px; "
            "background-color: white; color: #3c4043; font-size: 13px; "
            "font-weight: 500; height: 32px; padding: 0 10px; } "
            "QPushButton:hover { background-color: #f8f9fa; border-color: "
            "#dadce0; } "
            "QPushButton:pressed { background-color: #f1f3f4; }";
        btn->setStyleSheet(style);
        return btn;
    };

    QPushButton *newBtn =
        createHeaderBtn(":/newcirc.png", "", tr("New Circuit"));
    QPushButton *openBtn =
        createHeaderBtn(":/opencirc.png", "", tr("Open Circuit"));
    QPushButton *saveBtn = createHeaderBtn(":/savecirc.png", tr(" Save"));
    QPushButton *saveAsBtn =
        createHeaderBtn(":/savecircas.png", "", tr("Save Circuit As"));
    headerLayout->addWidget(newBtn);
    headerLayout->addWidget(openBtn);
    headerLayout->addWidget(saveBtn);
    headerLayout->addWidget(saveAsBtn);
    headerLayout->addSpacing(10);

    // Theme Selector (Light/Dark)
    QPushButton *lightBtn = createHeaderBtn(":/settings.png", tr(" Light"));
    QPushButton *darkBtn  = createHeaderBtn(":/settings.png", tr(" Dark"));

    headerLayout->addWidget(lightBtn);
    headerLayout->addWidget(darkBtn);

    auto updateThemeStyles = [this, lightBtn, darkBtn]() {
        bool dark = this->isDarkMode();
        QString activeStyle =
            "QPushButton { border: 2px solid #1a73e8; border-radius: 6px; "
            "background-color: #e8f0fe; color: #1a73e8; font-size: 13px; "
            "font-weight: bold; height: 32px; padding: 0 10px; }";
        QString inactiveStyle =
            "QPushButton { border: 1px solid #e0e0e0; border-radius: 6px; "
            "background-color: white; color: #3c4043; font-size: 13px; "
            "font-weight: 500; height: 32px; padding: 0 10px; } "
            "QPushButton:hover { background-color: #f8f9fa; }";

        lightBtn->setStyleSheet(!dark ? activeStyle : inactiveStyle);
        darkBtn->setStyleSheet(dark ? activeStyle : inactiveStyle);
    };
    updateThemeStyles();

    connect(lightBtn, &QPushButton::clicked, [this, updateThemeStyles]() {
        setTheme(false);
        updateThemeStyles();
    });
    connect(darkBtn, &QPushButton::clicked, [this, updateThemeStyles]() {
        setTheme(true);
        updateThemeStyles();
    });

    headerLayout->addStretch();

    // Rate Label from CircuitWidget
    QLabel *rateLabel = m_circuit->rateLabel();
    if (rateLabel) {
        rateLabel->setParent(header);
        headerLayout->addWidget(rateLabel);
    }

    headerLayout->addStretch();

    // Run Button (Toggles between Run/Pause)
    QPushButton *runBtn = new QPushButton(tr(" Run"), header);
    runBtn->setIcon(QIcon(":/poweroff.png"));
    runBtn->setFixedSize(85, 32);

    // Stop Button (Powers off the circuit)
    QPushButton *stopBtn = new QPushButton(tr(" Stop"), header);
    stopBtn->setIcon(
        QIcon(":/poweroff.png")); // Using same icon but different color
    stopBtn->setFixedSize(85, 32);
    QString stopStyle =
        "QPushButton { border: none; border-radius: 6px; background-color: "
        "#ea4335; color: white; font-size: 13px; font-weight: bold; } "
        "QPushButton:hover { background-color: #d93025; }";
    stopBtn->setStyleSheet(stopStyle);
    runBtn->setStyleSheet(
        "QPushButton { border: none; border-radius: 6px; background-color: "
        "#1a73e8; color: white; font-size: 13px; font-weight: bold; } "
        "QPushButton:hover { background-color: #1557b0; } "
        "QPushButton:pressed { background-color: #174ea6; }");

    headerLayout->addWidget(stopBtn);
    headerLayout->addWidget(runBtn);

    baseWidgetLayout->addWidget(header, 0, 0);
    baseWidgetLayout->addWidget(m_Centralsplitter, 1, 0);

    // Connect header buttons
    connect(newBtn, SIGNAL(clicked()), m_circuit, SLOT(newCircuit()));
    connect(openBtn, SIGNAL(clicked()), m_circuit, SLOT(openCirc()));
    connect(saveBtn, SIGNAL(clicked()), m_circuit, SLOT(saveCirc()));
    connect(saveAsBtn, SIGNAL(clicked()), m_circuit, SLOT(saveCircAs()));
    auto updateRunBtnStyle = [this, runBtn]() {
        bool running = Simulator::self()->isRunning();
        bool paused  = Simulator::self()->isPaused();

        if (running && !paused) {
            runBtn->setText(tr(" Pause"));
            runBtn->setIcon(QIcon(":/pausesim.png"));
            runBtn->setStyleSheet(
                "QPushButton { border: none; border-radius: 6px; "
                "background-color: #fbbc05; color: white; font-size: 13px; "
                "font-weight: bold; } "
                "QPushButton:hover { background-color: #f2a600; }");
        } else {
            runBtn->setText(tr(" Run"));
            runBtn->setIcon(QIcon(":/poweroff.png"));
            runBtn->setStyleSheet(
                "QPushButton { border: none; border-radius: 6px; "
                "background-color: #1a73e8; color: white; font-size: 13px; "
                "font-weight: bold; } "
                "QPushButton:hover { background-color: #1557b0; }");
        }
    };

    connect(runBtn, &QPushButton::clicked, [this, updateRunBtnStyle]() {
        if (!Simulator::self()->isRunning() && !Simulator::self()->isPaused()) {
            m_circuit->powerCircOn();
        } else {
            m_circuit->pauseSim();
        }
        updateRunBtnStyle();
    });

    connect(stopBtn, &QPushButton::clicked, [this, updateRunBtnStyle]() {
        m_circuit->powerCircOff();
        updateRunBtnStyle();
    });

    // 3. Code Editor (Right)
    m_editor = new EditorWindow(this);
    m_editor->setObjectName(QString::fromUtf8("editor"));
    m_Centralsplitter->addWidget(m_editor);

    // Set initial sizes to look like the image
    QList<int> sizes;
    sizes << 250 << 600 << 400; // Components, Circuit, Editor
    m_Centralsplitter->setSizes(sizes);

    // --- Styling ---
    // Apply a global stylesheet to mimic the clean white/flat look
    QString qss = R"(
        QMainWindow { background-color: white; }
        QSplitter::handle { background-color: #e0e0e0; width: 1px; }
        QTreeWidget { border: none; background-color: #fafafa; }
        QTreeWidget::item { padding: 5px; }
        QWidget#circuit { background-color: #f5f5f5; } /* Dot grid handled by circuit drawing */
    )";
    this->setStyleSheet(qss);

    // Old SidePanel stuff (Hidden/Unused)
    m_sidepanel = new QTabWidget(this); // Keep instantiation to prevent crash
                                        // if ref'd elsewhere, but hide
    m_sidepanel->hide();

    // These are initialized but hidden as requested
    m_ramTabWidget       = new QWidget(this);
    m_ramTabWidgetLayout = new QGridLayout(m_ramTabWidget);
    m_ramTabWidget->hide();
    m_itemprop = new PropertiesWidget(this);
    m_itemprop->hide();
    m_fileSystemTree = new FileWidget(this);
    m_fileSystemTree->hide();

    connect(m_sidepanel, SIGNAL(currentChanged(int)), this,
            SLOT(updateViewMenu(int)));

    m_sidepanel->hide();

    createMenus();
    this->showMaximized();
}

void MainWindow::updateViewMenu(int index)
{
    if (m_sidepanel->isHidden())
        return;

    /*// compViewAct->setChecked(index == 0);
    ramViewAct->setChecked(index == 0);
    propViewAct->setChecked(index == 1);
    fileViewAct->setChecked(index == 2);*/
}

void MainWindow::toggleSidePanel(bool show)
{
    /*QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;

    int index = -1;
    //if (action == compViewAct)
    //    index = 0;
    //else
    if (action == ramViewAct)
        index = 0;
    else if (action == propViewAct)
        index = 1;
    else if (action == fileViewAct)
        index = 2;

    if (index >= 0) {
        if (show) {
            // Uncheck others to avoid multiple visible (since it's a tab
            // widget)
            // compViewAct->blockSignals(true);
            ramViewAct->blockSignals(true);
            propViewAct->blockSignals(true);
            fileViewAct->blockSignals(true);

            // compViewAct->setChecked(index == 0);
            ramViewAct->setChecked(index == 0);
            propViewAct->setChecked(index == 1);
            fileViewAct->setChecked(index == 2);

            // compViewAct->blockSignals(false);
            ramViewAct->blockSignals(false);
            propViewAct->blockSignals(false);
            fileViewAct->blockSignals(false);

            m_sidepanel->setCurrentIndex(index);
            m_sidepanel->show();
        } else {
            m_sidepanel->hide();
        }
    }*/
}

void MainWindow::createMenus()
{
    /*QMenu *viewMenu = menuBar()->addMenu(tr("&View"));

    //compViewAct = new QAction(tr("Components"), this);
    //compViewAct->setCheckable(true);
    //compViewAct->setChecked(true);
    //connect(compViewAct, SIGNAL(toggled(bool)), this,
    //        SLOT(toggleSidePanel(bool)));
    //viewMenu->addAction(compViewAct);

    ramViewAct = new QAction(tr("RamTable"), this);
    ramViewAct->setCheckable(true);
    ramViewAct->setChecked(true);
    connect(ramViewAct, SIGNAL(toggled(bool)), this,
            SLOT(toggleSidePanel(bool)));
    viewMenu->addAction(ramViewAct);

    propViewAct = new QAction(tr("Properties"), this);
    propViewAct->setCheckable(true);
    propViewAct->setChecked(true);
    connect(propViewAct, SIGNAL(toggled(bool)), this,
            SLOT(toggleSidePanel(bool)));
    viewMenu->addAction(propViewAct);

    fileViewAct = new QAction(tr("File Explorer"), this);
    fileViewAct->setCheckable(true);
    fileViewAct->setChecked(true);
    connect(fileViewAct, SIGNAL(toggled(bool)), this,
            SLOT(toggleSidePanel(bool)));
    viewMenu->addAction(fileViewAct);*/
}

void MainWindow::loadCircHelp()
{
    QString locale = "_" + QLocale::system().name().split("_").first();
    QString dfPath = SIMUAPI_AppPath::self()->availableDataFilePath(
        "help/" + locale + "/circuit" + locale + ".txt");

    if (dfPath == "")
        dfPath =
            SIMUAPI_AppPath::self()->availableDataFilePath("help/circuit.txt");

    if (dfPath != "") {
        QFile file(dfPath);

        if (file.open(QFile::ReadOnly | QFile::Text)) // Get Text from Help File
        {
            QTextStream s1(&file);
            s1.setCodec("UTF-8");

            m_circHelp = "";
            m_circHelp.append(s1.readAll());

            file.close();
        }
    }
}

QString *MainWindow::circHelp()
{
    return &m_circHelp;
}

void MainWindow::loadPlugins()
{
    // Load main Plugins
    QDir pluginsDir(qApp->applicationDirPath());

    pluginsDir.cd("../lib/simulide/plugins");

    loadPluginsAt(pluginsDir);

    // Load main Component Sets
    QDir compSetDir = SIMUAPI_AppPath::self()->RODataFolder();

    if (compSetDir.exists())
        ComponentSelector::self()->LoadCompSetAt(compSetDir);

    // Load Addons
    QString userPluginsPath =
        SIMUAPI_AppPath::self()->RWDataFolder().absoluteFilePath("addons");

    pluginsDir.setPath(userPluginsPath);

    if (!pluginsDir.exists())
        return;

    for (QString pluginFolder : pluginsDir.entryList(QDir::Dirs)) {
        if (pluginFolder.contains("."))
            continue;
        // qDebug() << pluginFolder;
        pluginsDir.cd(pluginFolder);

        ComponentSelector::self()->LoadCompSetAt(pluginsDir);

        if (pluginsDir.entryList(QDir::Dirs).contains("lib")) {
            pluginsDir.cd("lib");
            loadPluginsAt(pluginsDir);
            pluginsDir.cd("../");
        }
        pluginsDir.cd("../");
    }
}

void MainWindow::loadPluginsAt(QDir pluginsDir)
{
    QString pluginName = "*plugin";

#ifndef Q_OS_UNIX
    pluginName += ".dll";
#else
    pluginName += ".so";
#endif

    pluginsDir.setNameFilters(QStringList(pluginName));

    QStringList fileList = pluginsDir.entryList(QDir::Files);

    if (fileList.isEmpty())
        return; // No plugins to load

    qDebug() << "\n    Loading Plugins at:\n"
             << pluginsDir.absolutePath() << "\n";

    for (QString libName : fileList) {
        pluginName =
            libName.split(".").first().remove("lib").remove("plugin").toUpper();

        if (m_plugins.contains(pluginName))
            continue;

        QPluginLoader *pluginLoader =
            new QPluginLoader(pluginsDir.absoluteFilePath(libName));
        QObject *plugin = pluginLoader->instance();

        if (plugin) {
            AppIface *item = qobject_cast<AppIface *>(plugin);

            if (item) {
                item->initialize();
                m_plugins[pluginName] = pluginLoader;
                qDebug() << "        Plugin Loaded Successfully:\t"
                         << pluginName;
            } else {
                pluginLoader->unload();
                delete pluginLoader;
            }
        } else {
            QString errorMsg = pluginLoader->errorString();
            qDebug() << "        " << pluginName
                     << "\tplugin FAILED: " << errorMsg;

            if (errorMsg.contains("libQt5SerialPort"))
                errorMsg = tr(" Qt5SerialPort is not installed in your "
                              "system\n\n    Mcu SerialPort will not work\n    "
                              "Just Install libQt5SerialPort package\n    To "
                              "have Mcu Serial Port Working");

            QMessageBox::warning(0, tr("Plugin Error:"), errorMsg);
        }
    }
    qDebug() << "\n";
}

void MainWindow::unLoadPugin(QString pluginName)
{
    if (m_plugins.contains(pluginName)) {
        QPluginLoader *pluginLoader = m_plugins[pluginName];
        QObject *plugin             = pluginLoader->instance();
        AppIface *item              = qobject_cast<AppIface *>(plugin);
        item->terminate();
        pluginLoader->unload();
        m_plugins.remove(pluginName);
        delete pluginLoader;
    }
}

void MainWindow::applyStyle()
{
    QFile file(":/simulide.qss");
    file.open(QFile::ReadOnly);

    m_styleSheet = QLatin1String(file.readAll());

    qApp->setStyleSheet(m_styleSheet);
}

QSettings *MainWindow::settings()
{
    return &m_settings;
}

void MainWindow::setTheme(bool dark)
{
    m_darkMode = dark;
    qApp->setPalette(qApp->style()->standardPalette());
    if (Circuit::self()) {
        for (Component *comp : *Circuit::self()->compList())
            comp->updateLabelsTheme();
        for (Component *comp : *Circuit::self()->conList())
            comp->updateLabelsTheme();
        Circuit::self()->update();
    }
}

#include "moc_mainwindow.cpp"
