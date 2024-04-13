#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QDataStream>
#include <QTimer>
#include <QRadioButton>

class FileModifier : public QWidget {
    Q_OBJECT
public:
    explicit FileModifier(QWidget *parent = nullptr) : QWidget(parent) {
        setupUI();
        connectSignals();
    }

private:
    QLabel *inputFileLabel;
    QLineEdit *inputFileLineEdit;
    QPushButton *browseButton;
    QLabel *outputDirectoryLabel;
    QLineEdit *outputDirectoryLineEdit;
    QPushButton *outputDirectoryBrowseButton;
    QPushButton *startButton;
    QRadioButton *overwriteRadioButton;
    QRadioButton *modifyRadioButton;
    QRadioButton *timerRadioButton;
    QLineEdit *timerIntervalLineEdit;
    QTimer *timer;
    QRadioButton *deleteInputFileCheckBox;

    void setupUI() {
        inputFileLabel = new QLabel("Выберите файл для модификации:", this);
        inputFileLineEdit = new QLineEdit(this);
        browseButton = new QPushButton("Обзор", this);

        outputDirectoryLabel = new QLabel("Выберите директорию для сохранения файлов:", this);
        outputDirectoryLineEdit = new QLineEdit(this);
        outputDirectoryBrowseButton = new QPushButton("Обзор", this);

        startButton = new QPushButton("Старт", this);

        overwriteRadioButton = new QRadioButton("Перезапись", this);
        modifyRadioButton = new QRadioButton("Модификация", this);
        modifyRadioButton->setChecked(true);

        timerRadioButton = new QRadioButton("Таймер", this);
        timerIntervalLineEdit = new QLineEdit(this);
        timerIntervalLineEdit->setPlaceholderText("Интервал в мс");

        deleteInputFileCheckBox = new QRadioButton("Удалить входные файлы", this);

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->addWidget(inputFileLabel);
        QHBoxLayout *inputLayout = new QHBoxLayout();
        inputLayout->addWidget(inputFileLineEdit);
        inputLayout->addWidget(browseButton);
        mainLayout->addLayout(inputLayout);
        mainLayout->addWidget(outputDirectoryLabel);
        QHBoxLayout *outputLayout = new QHBoxLayout();
        outputLayout->addWidget(outputDirectoryLineEdit);
        outputLayout->addWidget(outputDirectoryBrowseButton);
        mainLayout->addLayout(outputLayout);
        mainLayout->addWidget(overwriteRadioButton);
        mainLayout->addWidget(modifyRadioButton);
        mainLayout->addWidget(timerRadioButton);
        mainLayout->addWidget(timerIntervalLineEdit);
        mainLayout->addWidget(deleteInputFileCheckBox);
        mainLayout->addWidget(startButton);

        setLayout(mainLayout);
    }

    void connectSignals() {
        connect(browseButton, &QPushButton::clicked, this, &FileModifier::browseFile);
        connect(outputDirectoryBrowseButton, &QPushButton::clicked, this, &FileModifier::browseOutputDirectory);
        connect(startButton, &QPushButton::clicked, this, &FileModifier::startModification);

        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &FileModifier::startModification);
    }

    void browseFile() {
        QString filePath = QFileDialog::getOpenFileName(this, "Выберите файл");
        inputFileLineEdit->setText(filePath);
    }

    void browseOutputDirectory() {
        QString dirPath = QFileDialog::getExistingDirectory(this, "Выберите директорию для сохранения файлов");
        outputDirectoryLineEdit->setText(dirPath);
    }

    void startModification() {
        QString inputFilePath = inputFileLineEdit->text();
        QString outputDirectory = outputDirectoryLineEdit->text();

        if (inputFilePath.isEmpty() || outputDirectory.isEmpty()) {
            QMessageBox::warning(this, "Предупреждение", "Пожалуйста, выберите файл и директорию для сохранения");
            return;
        }

        QFile inputFile(inputFilePath);
        if (!inputFile.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось открыть входной файл для чтения");
            return;
        }

        QString outputFilePath;
        if (overwriteRadioButton->isChecked()) {
            outputFilePath = outputDirectory + "/" + QFileInfo(inputFilePath).fileName();
        } else {
            outputFilePath = outputDirectory + "/" + QFileInfo(inputFilePath).fileName() + "_modified";
        }

        if (QFile::exists(outputFilePath)) {
            QMessageBox::StandardButton response = QMessageBox::question(this, "Файл уже существует", "Файл уже существует. Перезаписать?", QMessageBox::Yes | QMessageBox::No);
            if (response == QMessageBox::No) {
                inputFile.close();
                return;
            }
        }

        QFile outputFile(outputFilePath);
        if (!outputFile.open(QIODevice::WriteOnly)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось открыть выходной файл для записи");
            inputFile.close();
            return;
        }

        // Операция модификации файла (например, XOR с 8-байтной переменной)
        QByteArray xorKey = QByteArrayLiteral("abcdefgh");
        QByteArray buffer;
        while (!inputFile.atEnd()) {
            buffer = inputFile.read(1024); // Читаем файл блоками по 1024 байта
            for (int i = 0; i < buffer.size(); ++i) {
                buffer[i] = buffer.at(i) ^ xorKey.at(i % xorKey.size());
            }
            outputFile.write(buffer); // Записываем измененный блок в выходной файл
        }

        inputFile.close();
        outputFile.close();

        if (deleteInputFileCheckBox->isChecked()) {
            inputFile.remove();
        }

        QMessageBox::information(this, "Готово", "Файл успешно модифицирован и сохранен: " + outputFilePath);

        if (timerRadioButton->isChecked()) {
            int interval = timerIntervalLineEdit->text().toInt();
            timer->start(interval);
        }
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    FileModifier modifier;
    modifier.setWindowTitle("Программа модификации файлов");
    modifier.show();

    return app.exec();
}

#include "main.moc"
