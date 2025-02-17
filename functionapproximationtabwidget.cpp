#include "functionapproximationtabwidget.h"
#include "ui_functionapproximationtabwidget.h"

FunctionApproximationTabWidget::FunctionApproximationTabWidget(QWidget *parent, const FunctionApproximation &function_input) :
    QTabWidget(parent),
    ui(new Ui::FunctionApproximationTabWidget)
{
    ui->setupUi(this);

    QVector<double> coefficients = function_input.getCoefficients();
    bool are_two_peaks = true;
    if (coefficients[3] == 0) {
        are_two_peaks = false;
    }
    double constant = function_input.getConstant();
    double start = findKeyByValue([&function_input](const double &key) {return function_input.getValue(key);}, constant, PRECISION, coefficients[2]);
    double end = findKeyByValue([&function_input](const double &key) {return function_input.getValue(key);}, constant, PRECISION, coefficients[5]);
    if (!are_two_peaks) {
        end = findKeyByValue([&function_input](const double &key) {return function_input.getValue(key);}, constant, PRECISION, coefficients[2] + 0.5);
    }

    Function function, first_component, second_component, first_derivative, second_derivative;
    const unsigned int n = 200;
    double step = (end - start) / (double)n;
    double x = start;
    for (unsigned int i = 0; i < n; i++, x += step) {
        function.add({x, function_input.getValue(x)});
        first_component.add({x, function_input.getFirstComponentValue(x) + constant});
        second_component.add({x, function_input.getSecondComponentValue(x) + constant});
        first_derivative.add({x, function_input.getFirstDerivativeValue(x)});
        second_derivative.add({x, function_input.getSecondDerivativeValue(x)});
    }

    ui->function_plot->setFunction(function);
    ui->function_plot->plotFunction(first_component, QPen(QBrush(QColor(Qt::darkYellow)), 1));
    ui->function_plot->plotFunction(second_component, QPen(QBrush(QColor(Qt::darkGreen)), 1));
    ui->dv1_plot->setFunction(first_derivative);
    ui->dv2_plot->setFunction(second_derivative);

    ui->function_table->setRowCount(n);
    ui->function_table->horizontalHeader()->setStretchLastSection(true);
    for (unsigned int i = 0; i < n; i++, x += step) {
        ui->function_table->setItem(i, 0, new QTableWidgetItem(QString::number(function.getKey(i))));
        ui->function_table->setItem(i, 1, new QTableWidgetItem(QString::number(function.getValue(i))));
        ui->function_table->setItem(i, 2, new QTableWidgetItem(QString::number(first_derivative.getValue(i))));
        ui->function_table->setItem(i, 3, new QTableWidgetItem(QString::number(second_derivative.getValue(i))));
    }

    ui->main_info_text_edit->setText(
        QString::fromUtf8("Результаты вычислений:\n") +
        QString::fromUtf8("Коэффициенты при экспонентах:\na1 = ") + QString::number(coefficients[0]) +
        QString::fromUtf8(", b1 = ") + QString::number(coefficients[1]) +
        QString::fromUtf8(", c1 = ") + QString::number(coefficients[2]) +
        QString::fromUtf8("\na2 = ") + QString::number(coefficients[3]) +
        QString::fromUtf8(", b2 = ") + QString::number(coefficients[4]) +
        QString::fromUtf8(", c2 = ") + QString::number(coefficients[5]) +
        QString::fromUtf8("\nПервая производная:\n") +
        QString::number(-2 * coefficients[0] * coefficients[1]) +
        QString::fromUtf8("(x-") +
        QString::number(coefficients[2]) +
        QString::fromUtf8(")exp{") +
        QString::number(-coefficients[1]) +
        QString::fromUtf8("(x-") +
        QString::number(coefficients[2]) +
        QString::fromUtf8(")^2}") +
        QString::number(-2 * coefficients[3] * coefficients[4]) +
        QString::fromUtf8("(x-") + QString::number(coefficients[5]) +
        QString::fromUtf8(")exp{") +
        QString::number(-coefficients[4]) +
        QString::fromUtf8("(x-") +
        QString::number(coefficients[5]) +
        QString::fromUtf8(")^2}") +
        QString::fromUtf8("\nВторая производная:\n") +
        QString::fromUtf8("2(2*") +
        QString::number(coefficients[0]) +
        QString::fromUtf8("*") +
        QString::number(coefficients[1]) +
        QString::fromUtf8("^(2)*(x-") +
        QString::number(coefficients[2]) +
        QString::fromUtf8(")^(2)*") +
        QString::fromUtf8("exp{") +
        QString::number(-coefficients[1]) +
        QString::fromUtf8("(x-") +
        QString::number(coefficients[2]) +
        QString::fromUtf8(")^2}") +
        QString::fromUtf8("-") +
        QString::number(coefficients[0]) +
        QString::fromUtf8("*") +
        QString::number(coefficients[1]) +
        QString::fromUtf8("*exp{") +
        QString::number(-coefficients[1]) +
        QString::fromUtf8("(x-") +
        QString::number(coefficients[2]) +
        QString::fromUtf8(")^2}") +
        QString::fromUtf8("+2*") +
        QString::number(coefficients[3]) +
        QString::fromUtf8("*") +
        QString::number(coefficients[4]) +
        QString::fromUtf8("^(2)*(x-") +
        QString::number(coefficients[5]) +
        QString::fromUtf8(")^(2)*") +
        QString::fromUtf8("exp{") +
        QString::number(-coefficients[4]) +
        QString::fromUtf8("(x-") +
        QString::number(coefficients[5]) +
        QString::fromUtf8(")^2}") +
        QString::fromUtf8("-") +
        QString::number(coefficients[3]) +
        QString::fromUtf8("*") +
        QString::number(coefficients[4]) +
        QString::fromUtf8("*exp{") +
        QString::number(-coefficients[4]) +
        QString::fromUtf8("(x-") +
        QString::number(coefficients[5]) +
        QString::fromUtf8(")^2})")
    );

    double peak_time_1 = 0, peak_time_2 = 0;
    peak_time_1 = findMaximum([&function_input](const double &key){return function_input.getValue(key);}, PRECISION, coefficients[2]);
    peak_time_2 = findMaximum([&function_input](const double &key){return function_input.getValue(key);}, PRECISION, coefficients[5]);
    if (are_two_peaks && std::abs(peak_time_2 - peak_time_1) < PRECISION * 50) {
        are_two_peaks = false;
        peak_time_2 = peak_time_1;
    } else if (!are_two_peaks) {
        peak_time_2 = peak_time_1;
    }
    double first_peak_maximum = function_input.getValue(peak_time_1), second_peak_maximim = -INFINITY;

    double amplitude = first_peak_maximum - constant;
    double amplitude_2 = first_peak_maximum - constant;

    double time_up_50_percent = -INFINITY;
    double time_down_50_percent = -INFINITY;

    double time_up_90_percent = -INFINITY;
    double time_down_90_percent = -INFINITY;

    double time_up_10_percent = -INFINITY;
    double time_down_10_percent = -INFINITY;

    QString additional_info_text = "";
    additional_info_text +=
        QString::fromUtf8("Момент первого пика: ") +
        QString::number(peak_time_1) +
        QString::fromUtf8("\nЗначение в первом пике: ") +
        QString::number(first_peak_maximum) + '\n';

    if (are_two_peaks) {
        second_peak_maximim = function_input.getValue(peak_time_2);
        additional_info_text +=
            QString::fromUtf8("Момент второго пика: ") +
            QString::number(peak_time_2) +
            QString::fromUtf8("\nЗначение во втором пике: ") +
            QString::number(second_peak_maximim) + '\n';
        amplitude_2 = second_peak_maximim - constant;
        time_down_50_percent = findKeyByValue([&function_input](const double &key) {return function_input.getValue(key);}, second_peak_maximim - amplitude_2 / 2.0, PRECISION, end);
        time_down_90_percent = findKeyByValue([&function_input](const double &key) {return function_input.getValue(key);}, second_peak_maximim - amplitude_2 / 10.0, PRECISION, end);
        time_down_10_percent = findKeyByValue([&function_input](const double &key) {return function_input.getValue(key);}, second_peak_maximim - 9.0 * amplitude_2 / 10.0, PRECISION, end);
    } else {
        if (first_peak_maximum < second_peak_maximim) {
            peak_time_1 = peak_time_2;
            first_peak_maximum = second_peak_maximim;
            amplitude = first_peak_maximum - constant;
            amplitude_2 = first_peak_maximum - constant;
        }
        time_down_50_percent = findKeyByValue([&function_input](const double &key) {return function_input.getValue(key);}, first_peak_maximum - amplitude / 2.0, PRECISION, end);
        time_down_90_percent = findKeyByValue([&function_input](const double &key) {return function_input.getValue(key);}, first_peak_maximum - amplitude / 10.0, PRECISION, end);
        time_down_10_percent = findKeyByValue([&function_input](const double &key) {return function_input.getValue(key);}, first_peak_maximum - 9.0 * amplitude / 10.0, PRECISION, end);
    }

    time_up_50_percent = findKeyByValue([&function_input](const double &key) {return function_input.getValue(key);}, first_peak_maximum - amplitude / 2.0, PRECISION, start);

    time_up_90_percent = findKeyByValue([&function_input](const double &key) {return function_input.getValue(key);}, first_peak_maximum - amplitude / 10.0, PRECISION, start);

    time_up_10_percent = findKeyByValue([&function_input](const double &key) {return function_input.getValue(key);}, first_peak_maximum - 9.0 * amplitude / 10.0, PRECISION, start);

    additional_info_text +=
        QString::fromUtf8("Значение во время простоя: ") +
        QString::number(constant) + '\n';

    additional_info_text +=
        QString::fromUtf8("Амплитуда: ") +
        QString::number(amplitude) + '\n';

    additional_info_text +=
        QString::fromUtf8("Момент 50%-ной амплитуды на подъёме: ") +
        QString::number(time_up_50_percent) + '\n';
    additional_info_text +=
        QString::fromUtf8("Значение 50%-ной амплитуды на подъёме: ") +
        QString::number(function_input.getValue(time_up_50_percent)) + '\n';

    additional_info_text +=
        QString::fromUtf8("Момент 50%-ной амплитуды на спуске: ") +
        QString::number(time_down_50_percent) + '\n';
    additional_info_text +=
        QString::fromUtf8("Значение 50%-ной амплитуды на спуске: ") +
        QString::number(function_input.getValue(time_down_50_percent)) + '\n';

    double velocity_max_up_time = findMaximum([&function_input](const double &key){return function_input.getFirstDerivativeValue(key);}, PRECISION, peak_time_1 - (peak_time_1 - start) / 2.0);

    double velocity_max_down_time = findMaximum([&function_input](const double &key){return -function_input.getFirstDerivativeValue(key);}, PRECISION, peak_time_2 + (end - peak_time_2) / 2.0);

    additional_info_text +=
        QString::fromUtf8("Максимальная скорость на подъёме: ") +
        QString::number(function_input.getFirstDerivativeValue(velocity_max_up_time)) + '\n';
    additional_info_text +=
        QString::fromUtf8("Максимальная скорость на спуске: ") +
        QString::number(-function_input.getFirstDerivativeValue(velocity_max_down_time)) + '\n';

    additional_info_text +=
        QString::fromUtf8("Скорость на подъёме (тип 1): ") +
        QString::number(amplitude / (peak_time_1 - start)) + '\n';

    additional_info_text +=
        QString::fromUtf8("Скорость на спуске (тип 1): ") +
        QString::number(amplitude_2 / (end - peak_time_2)) + '\n';

    double time = end - start;

    additional_info_text +=
        QString::fromUtf8("Полное время: ") +
        QString::number(time) + '\n';

    double integral = findIntegral([&function_input](const double &key) {return function_input.getValue(key);}, start, end, PRECISION);

    additional_info_text +=
        QString::fromUtf8("Интеграл: ") +
        QString::number(integral) + '\n';

    additional_info_text +=
        QString::fromUtf8("Скорость на подъёме (тип 2): ") +
        QString::number((function_input.getValue(time_up_90_percent) - function_input.getValue(time_up_10_percent)) / (time_up_90_percent - time_up_10_percent)) + '\n';

    additional_info_text +=
        QString::fromUtf8("Скорость на спуске (тип 2): ") +
        QString::number((function_input.getValue(time_down_90_percent) - function_input.getValue(time_down_10_percent)) / (time_down_10_percent - time_down_90_percent)) + '\n';

    ui->additional_info_text_edit->setText(additional_info_text);
}

FunctionApproximationTabWidget::~FunctionApproximationTabWidget()
{
    delete ui;
}
