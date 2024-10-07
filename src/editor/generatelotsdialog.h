#ifndef GENERATELOTSDIALOG_H
#define GENERATELOTSDIALOG_H

#include <QDialog>

class QComboBox;

namespace Ui {
class GenerateLotsDialog;
}

class WorldDocument;

class GenerateLotsDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit GenerateLotsDialog(WorldDocument *worldDoc, QWidget *parent = 0);
    ~GenerateLotsDialog();

private slots:
    void exportBrowse();
    void exportChanged(const QString &text);
    void spawnBrowse();
    void spawnChanged(const QString &text);
    void tileDefBrowse();
    void tileDefChanged(const QString &text);
    void accept();
    void apply();

private:
    void addComboItemIfAbsent(QComboBox *comboBox, const QString &text);
    QStringList comboboxStringList(QComboBox *comboBox) const;
    bool validate();

private:
    Ui::GenerateLotsDialog *ui;
    WorldDocument *mWorldDoc;
    QString mExportDir;
    QString mZombieSpawnMap;
    QString mTileDefFolder;
};

#endif // GENERATELOTSDIALOG_H
