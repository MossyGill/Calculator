#include "widget.h"
#include "ui_widget.h"

#include <cmath>

namespace {

// 把显示框里的表达式（例如 "12+3*(4-2)"）求值。
// 这里用递归下降法：每一层只处理同一优先级的运算符，
// 所以括号和后乘除、先加减的规则由代码结构本身保证，不需要额外的优先级表。
class ExpressionParser
{
public:
    explicit ExpressionParser(const QString &text)
        : m_text(text)
    {
    }

    // 解析成功时把结果写入 result 并返回 true；
    // 失败时返回 false，原因由 errorMessage() 给出。
    bool evaluate(double *result)
    {
        m_position = 0;
        m_ok = true;
        m_error.clear();

        const double value = parseExpression();

        skipSpaces();
        if (m_ok && m_position < m_text.size())
            fail(QStringLiteral("表达式无效"));

        if (m_ok)
            *result = value;
        return m_ok;
    }

    QString errorMessage() const { return m_error; }

private:
    // 表达式 := 项 { ('+' | '-') 项 }
    double parseExpression()
    {
        double value = parseTerm();

        for (;;) {
            if (!m_ok)
                return 0.0;

            skipSpaces();
            if (m_position >= m_text.size())
                return value;

            const QChar op = m_text.at(m_position);
            if (op != QLatin1Char('+') && op != QLatin1Char('-'))
                return value;

            ++m_position;

            const double rhs = parseTerm();
            value = (op == QLatin1Char('+')) ? value + rhs : value - rhs;
        }
    }

    // 项 := 因子 { ('*' | '/') 因子 }
    double parseTerm()
    {
        double value = parseFactor();

        for (;;) {
            if (!m_ok)
                return 0.0;

            skipSpaces();
            if (m_position >= m_text.size())
                return value;

            const QChar op = m_text.at(m_position);
            if (op != QLatin1Char('*') && op != QLatin1Char('/'))
                return value;

            ++m_position;

            const double rhs = parseFactor();
            if (!m_ok)
                return 0.0;

            if (op == QLatin1Char('*')) {
                value *= rhs;
            } else if (qFuzzyIsNull(rhs)) {
                fail(QStringLiteral("除数不能为 0"));
                return 0.0;
            } else {
                value /= rhs;
            }
        }
    }

    // 因子 := ('+' | '-') 因子 | '(' 表达式 ')' | 数字
    double parseFactor()
    {
        if (!m_ok)
            return 0.0;

        skipSpaces();
        if (m_position >= m_text.size()) {
            fail(QStringLiteral("表达式不完整"));
            return 0.0;
        }

        const QChar current = m_text.at(m_position);

        if (current == QLatin1Char('+')) {
            ++m_position;
            return parseFactor();
        }

        if (current == QLatin1Char('-')) {
            ++m_position;
            return -parseFactor();
        }

        if (current == QLatin1Char('(')) {
            ++m_position;
            const double value = parseExpression();
            if (!m_ok)
                return 0.0;

            skipSpaces();
            if (m_position < m_text.size() && m_text.at(m_position) == QLatin1Char(')')) {
                ++m_position;
            } else {
                fail(QStringLiteral("括号不匹配"));
                return 0.0;
            }
            return value;
        }

        return parseNumber();
    }

    double parseNumber()
    {
        skipSpaces();
        const int start = m_position;
        bool hasPoint = false;

        while (m_position < m_text.size()) {
            const QChar current = m_text.at(m_position);
            if (current.isDigit()) {
                ++m_position;
            } else if (current == QLatin1Char('.') && !hasPoint) {
                hasPoint = true;
                ++m_position;
            } else {
                break;
            }
        }

        if (start == m_position) {
            fail(QStringLiteral("表达式无效"));
            return 0.0;
        }

        bool ok = false;
        const double value = m_text.mid(start, m_position - start).toDouble(&ok);
        if (!ok) {
            fail(QStringLiteral("表达式无效"));
            return 0.0;
        }
        return value;
    }

    void skipSpaces()
    {
        while (m_position < m_text.size() && m_text.at(m_position).isSpace())
            ++m_position;
    }

    void fail(const QString &message)
    {
        if (m_ok) {
            m_ok = false;
            m_error = message;
        }
    }

    const QString &m_text;
    int m_position = 0;
    bool m_ok = true;
    QString m_error;
};

// 把结果转成显示框里的文字：整数不带小数点，小数去掉末尾多余的 0。
// 常规范围内刻意不用科学计数法，否则 "1e+06" 这种结果没法再接着参与运算。
QString formatNumber(double value)
{
    if (!std::isfinite(value))
        return QStringLiteral("错误");

    const double magnitude = std::fabs(value);
    const bool useFixed = (magnitude == 0.0) || (magnitude >= 1e-6 && magnitude < 1e15);

    QString text = useFixed ? QString::number(value, 'f', 12)
                            : QString::number(value, 'g', 12);

    if (text.contains(QLatin1Char('.'))) {
        while (text.endsWith(QLatin1Char('0')))
            text.chop(1);
        if (text.endsWith(QLatin1Char('.')))
            text.chop(1);
    }

    return text;
}

} // namespace

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    // 设置窗口的大小
    this->setMaximumSize(205, 300);
    this->setMinimumSize(205, 300);

    // 设置窗口的名字
    this->setWindowTitle("计算器");

    // 设置数字显示框的字体
    QFont fon("Minecraft", 12);
    ui->lineeditMain->setFont(fon);

    // 设置退格按钮的图片
    QIcon con(":/left.png");
    ui->buttonBackspace->setIcon(con);

    // 设置等于号按钮的外观：粉色底、深色文字
    ui->buttonEqual->setStyleSheet("QPushButton {"
                                   "    background-color: pink;"
                                   "    color: #2B2B2B;"
                                   "}");

}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_button0_clicked()
{
    expression += "0";
    ui->lineeditMain->setText(expression);
}

void Widget::on_button1_clicked()
{
    expression += "1";
    ui->lineeditMain->setText(expression);
}


void Widget::on_button2_clicked()
{
    expression += "2";
    ui->lineeditMain->setText(expression);
}


void Widget::on_button3_clicked()
{
    expression += "3";
    ui->lineeditMain->setText(expression);
}


void Widget::on_button4_clicked()
{
    expression += "4";
    ui->lineeditMain->setText(expression);
}

void Widget::on_button5_clicked()
{
    expression += "5";
    ui->lineeditMain->setText(expression);
}

void Widget::on_button6_clicked()
{
    expression += "6";
    ui->lineeditMain->setText(expression);
}

void Widget::on_button7_clicked()
{
    expression += "7";
    ui->lineeditMain->setText(expression);
}

void Widget::on_button8_clicked()
{
    expression += "8";
    ui->lineeditMain->setText(expression);
}

void Widget::on_button9_clicked()
{
    expression += "9";
    ui->lineeditMain->setText(expression);
}

void Widget::on_buttonLeftBracket_clicked()
{
    expression += "(";
    ui->lineeditMain->setText(expression);
}

void Widget::on_buttonRightBracket_clicked()
{
    expression += ")";
    ui->lineeditMain->setText(expression);
}

void Widget::on_buttonPlus_clicked()
{
    expression += "+";
    ui->lineeditMain->setText(expression);
}

void Widget::on_buttonMinus_clicked()
{
    expression += "-";
    ui->lineeditMain->setText(expression);
}

void Widget::on_buttonMultiplication_clicked()
{
    expression += "*";
    ui->lineeditMain->setText(expression);
}

void Widget::on_buttonDivision_clicked()
{
    expression += "/";
    ui->lineeditMain->setText(expression);
}

void Widget::on_buttonClear_clicked()
{
    expression.clear();
    ui->lineeditMain->setText(expression);
}

void Widget::on_buttonBackspace_clicked()
{
    expression.chop(1);
    ui->lineeditMain->setText(expression);
}

void Widget::on_buttonEqual_clicked()
{
    if (expression.isEmpty())
        return;

    double result = 0.0;
    ExpressionParser parser(expression);

    if (!parser.evaluate(&result)) {
        // 只把出错原因显示出来，expression 本身保留不变，
        // 这样点退格键还能改掉打错的地方，不至于把输入整段丢掉。
        ui->lineeditMain->setText(parser.errorMessage());
        return;
    }

    // 结果写回 expression，这样算完之后可以接着按 "+5" 继续往下算。
    expression = formatNumber(result);
    ui->lineeditMain->setText(expression);
}