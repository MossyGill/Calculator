#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

private slots:
    void on_button0_clicked();

    void on_button1_clicked();

    void on_button2_clicked();

    void on_button3_clicked();

    void on_button4_clicked();

    void on_button5_clicked();

    void on_button6_clicked();

    void on_button7_clicked();

    void on_button8_clicked();

    void on_button9_clicked();

    void on_buttonLeftBracket_clicked();

    void on_buttonRightBracket_clicked();

    void on_buttonPlus_clicked();

    void on_buttonMinus_clicked();

    void on_buttonMultiplication_clicked();

    void on_buttonDivision_clicked();

    void on_buttonClear_clicked();

    void on_buttonBackspace_clicked();

    void on_buttonEqual_clicked();

private:
    Ui::Widget *ui;
    QString expression;

};
#endif // WIDGET_H
