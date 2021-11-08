#include <math.h>
#include <QMessageBox>
#include <QTextStream>
#include <QApplication>

#include "netfunctionapproximator.h"

inline LineIndexes::LineIndexes(unsigned int begin_input, unsigned int end_input) {
    begin = begin_input;
    end = end_input;
}

inline bool LineIndexes::isIndexInLine(unsigned int index) {
    return ((index >= begin) && (index <= end));
}

inline bool LineIndexes::isIndexOutOfLine(unsigned int index) {
    return ((index < begin) || (index > end));
}

NetFunctionApproximator::NetFunctionApproximator(QWidget *ptr): AbstractFunctionApproximator(ptr) {
}

FunctionApproximation NetFunctionApproximator::approximate(const Function &function) {
    QMessageBox msgBox(parent);
    msgBox.setWindowTitle(QString::fromUtf8("Пожалуйста, подождите"));
    msgBox.setText(QString::fromUtf8("Пожалуйста, подождите. Идут вычисления."));
    msgBox.setStandardButtons(QMessageBox::NoButton);
    msgBox.setModal(true);
    msgBox.show();
    QApplication::processEvents();

    QTextStream out(stdout);


    unsigned int min_local = 0, points_count = function.size();
    for (unsigned int i = 1; i < points_count; ++i) {
        if (function.getValue(i) < function.getValue(min_local)) {
            min_local = i;
        }
    }
    double coefficient = function.getValue(min_local);
    Function normalized_function;
    for (unsigned int i = 0; i < points_count; ++i) {
        normalized_function.add({function.getKey(i), function.getValue(i) - coefficient});
    }

    unsigned int index = points_count / 10;
    double a1, a2, b1, b2, c1, c2, step = function.getStep();
    while (normalized_function.getValue(index) <= normalized_function.getValue(index + 1)) {
        index++;
    }
    c1 = normalized_function.getKey(index);
    a1 = normalized_function.getValue(index);
    b1 = std::abs(-log(normalized_function.getValue(index / 2) / a1) / (pow(normalized_function.getKey(index / 2) - c1, 2)));

    index = int(points_count * 0.9);
    while (normalized_function.getValue(index) <= normalized_function.getValue(index - 1)) {
        index--;
    }
    c2 = normalized_function.getKey(index);
    a2 = normalized_function.getValue(index);
    b2 = std::abs(-log(normalized_function.getValue(index + (points_count - index) / 2) / a2) / (pow(normalized_function.getKey(index + (points_count - index) / 2) - c2, 2)));

    bool are_two_extremums = true;
    if (std::abs(c1 - c2) <= 25 * step) {
        are_two_extremums = false;
    }

    out << "a1 = " << a1 << ", b1 = " << b1 << ", c1 = " << c1 << '\n' << flush;
    if (are_two_extremums) {
        out << "a2 = " << a2 << ", b2 = " << b2 << ", c2 = " << c2 << "\n\n" << flush;
    } else {
        //a2 = b2 = c2 = 0;
        a1 /= 2;
        a2 = a1;
        b2 = b1;
        //c2 = c1;
        out << '\n';
    }

    double step1 = 0.5, step2 = 0.01, step3 = 0.1;
    if ((a1 > 1000) || (a2 > 1000)) {
        step1 = 2;
        step3 = 0.05;
    } else if ((a1 > 100) || (a2 > 100)) {
        step1 = 1;
        step3 = 0.1;
    } else if ((a1 > 10) || (a2 > 10)) {
        step1 = 0.5;
        step3 = 0.5;
    } else {
        step1 = 0.1;
        step3 = 1;
    }
    if ((b1 > 10) || (b2 > 10)) {
        step2 = 0.5;
    } else if ((b1 > 1) || (b2 > 1)) {
        step2 = 0.05;
    } else {
        step2 = 0.01;
    }
    if (!are_two_extremums) {
        step1 *= 2;
        step2 *= 2;
        step3 *= 2;
    }

    uint8_t min_point_indexes[6];
    for (uint8_t i = 0; i < 6; ++i) {
        min_point_indexes[i] = 0;
    }

    long double approximating_function = 0;
    uint8_t little_changes_count = 0, max_little_changes = 50, decrease_net_step = max_little_changes / 10;
    double min_difference = INFINITY, previous_difference = 0, difference = 0;
    while (min_difference > step) {
        if (c1 < normalized_function.getKey(0) - 1000 * step || c1 > normalized_function.getKey(points_count - 1) + 1000 * step) {
            a1 = b1 = c1 = 0;
        }
        if (c2 < normalized_function.getKey(0) - 1000 * step || c2 > normalized_function.getKey(points_count - 1) + 1000 * step) {
            a2 = b2 = c2 = 0;
        }
        if (std::abs(previous_difference - min_difference) < step) {
            little_changes_count++;
            if (little_changes_count % decrease_net_step == 0) {
                step1 /= 10;
                step2 /= 10;
                step3 /= 10;
            }
            if (little_changes_count > max_little_changes) {
                break;
            }
        } else {
            little_changes_count = 0;
        }
        previous_difference = min_difference;
        uint8_t net_step = 5, net_step_center = net_step / 2;
        for (uint8_t i = 0; i < 6; ++i) {
            min_point_indexes[i] = net_step_center;
        }
        for (uint8_t x1 = 0; x1 < net_step; ++x1) {
            for (uint8_t x2 = 0; x2 < net_step; ++x2) {
                for (uint8_t x3 = 0; x3 < net_step; ++x3) {
                    for (uint8_t x4 = 0; x4 < net_step; ++x4) {
                        for (uint8_t x5 = 0; x5 < net_step; ++x5) {
                            for (uint8_t x6 = 0; x6 < net_step; ++x6) {
                                QApplication::processEvents();
                                if (((c1 + (net_step_center - x3) * step3) > normalized_function.getKey(0)) && ((c1 + (net_step_center - x3) * step3) < normalized_function.getKey(points_count - 1)) && ((c2 + (net_step_center - x6) * step3) > normalized_function.getKey(0)) && ((c2 + (net_step_center - x6) * step3) < normalized_function.getKey(points_count - 1))) {
                                    difference = 0;
                                    for (unsigned int i = 0; i < points_count; ++i) {
                                        approximating_function = std::abs(a1 - (net_step_center - x1) * step1) * exp(-std::abs(b1 - (net_step_center - x2) * step2) * pow(normalized_function.getKey(i) - c1 + (net_step_center - x3) * step3, 2)) + std::abs(a2 - (net_step_center - x4) * step1) * exp(-std::abs(b2 - (net_step_center - x5) * step2) * pow(normalized_function.getKey(i) - c2 + (net_step_center - x6) * step3, 2));
                                        difference += std::abs(normalized_function.getValue(i) - approximating_function);
                                    }
                                    if (difference < min_difference) {
                                        min_difference = difference;
                                        min_point_indexes[0] = x1;
                                        min_point_indexes[1] = x2;
                                        min_point_indexes[2] = x3;
                                        min_point_indexes[3] = x4;
                                        min_point_indexes[4] = x5;
                                        min_point_indexes[5] = x6;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        a1 -= (2 - min_point_indexes[0]) * step1;
        b1 -= (2 - min_point_indexes[1]) * step2;
        c1 -= (2 - min_point_indexes[2]) * step3;
        a2 -= (2 - min_point_indexes[3]) * step1;
        b2 -= (2 - min_point_indexes[4]) * step2;
        c2 -= (2 - min_point_indexes[5]) * step3;

        out << "a1 = " << a1 << ", b1 = " << b1 << ", c1 = " << c1 << '\n' << flush;
        out << "a2 = " << a2 << ", b2 = " << b2 << ", c2 = " << c2 << '\n' << flush;
        out << min_difference << ' ' << previous_difference << '\n' << flush;

        for (uint8_t i = 0; i < 6; ++i) {
            out << (int)min_point_indexes[i] << ' ' << flush;
        }
        out << '\n' << step1 << ' ' << step2 << ' ' << step3 << '\n' << flush;
        out << "\n\n" << flush;
    }

    a1 = std::abs(a1);
    a2 = std::abs(a2);
    b1 = std::abs(b1);
    b2 = std::abs(b2);


    if (c2 < function.getKey(0) - 100 * step || c2 > function.getKey(points_count - 1) + 100 * step) {
        a2 = b2 = c2 = 0;
    } else if (c1 > c2) {
        std::swap(a1, a2);
        std::swap(b1, b2);
        std::swap(c1, c2);
    }

    out << "a1 = " << a1 << ", b1 = " << b1 << ", c1 = " << c1 << '\n' << flush;
    out << "a2 = " << a2 << ", b2 = " << b2 << ", c2 = " << c2 << '\n' << flush;

    return FunctionApproximation(a1, b1, c1, a2, b2, c2, coefficient);

    /*unsigned int min_local = 0, n = function.size();
    for (unsigned int i = 1; i < n; ++i) {
        if (function.getValue(i) < function.getValue(min_local)) {
            min_local = i;
        }
    }
    double coefficient = function.getValue(min_local);
    Function normalized_function;
    for (unsigned int i = 0; i < n; ++i) {
        normalized_function.add({function.getKey(i), function.getValue(i) - coefficient});
    }

    unsigned int index = n / 10, peak_beginning = 0;
    bool is_peak_unavailable = false;
    LineIndexes peakLine1, peakLine2;
    double a1, a2, b1, b2, c1, c2, step = function.getStep(), peak_length = 0;
    while (normalized_function.getValue(index) <= normalized_function.getValue(index + 1)) {
        if (function.getValue(index) == normalized_function.getValue(index + 1)) {
            peak_length += function.getKey(index + 1) - function.getKey(index);
            if (peak_length > 0.01) {
                is_peak_unavailable = true;
            }
        } else if (function.getValue(index + 1) == normalized_function.getValue(index + 2)) {
            peak_beginning = index + 1;
        }
        index++;
        if ((function.getValue(index + 1) > function.getValue(index + 2)) && (is_peak_unavailable)) {
            peakLine1 = {peak_beginning, index + 1};
            index -= (index + 1 - peak_beginning) / 2 - 1;
            break;
        }
    }
    c1 = normalized_function.getKey(index);
    a1 = normalized_function.getValue(index);
    b1 = std::abs(-log(normalized_function.getValue(index * 100 / 245) / a1) / (pow(normalized_function.getKey(index * 100 / 245) - c1, 2)));

    if (is_peak_unavailable) {
        a1 += coefficient / 2;
    }

    index = int(n * 0.9);
    is_peak_unavailable = false;
    peak_length = 0;
    while (normalized_function.getValue(index) <= normalized_function.getValue(index - 1)) {
        if (function.getValue(index) == normalized_function.getValue(index - 1)) {
            peak_length += function.getKey(index) - function.getKey(index - 1);
            if (peak_length > 0.01) {
                is_peak_unavailable = true;
            }
        } else if (function.getValue(index - 1) == normalized_function.getValue(index - 2)) {
            peak_beginning = index - 1;
        }
        index--;
        if ((function.getValue(index - 1) > function.getValue(index - 2)) && (is_peak_unavailable)) {
            peakLine2 = {index - 1, peak_beginning};
            index += (peak_beginning - index + 1) / 2 - 1;
            break;
        }
    }
    c2 = normalized_function.getKey(index);
    a2 = normalized_function.getValue(index);
    b2 = std::abs(-log(normalized_function.getValue(index + (n - index) * 100 / 182) / a2) / (pow(normalized_function.getKey(index + (n - index) * 100 / 182) - c2, 2)));

    bool long_line = false;
    if ((c1 > c2) && (!is_peak_unavailable)) {
        long_line = true;
        int max_local = a1;
        index = n / 10;
        while ((max_local - normalized_function.getValue(index) > 4) || (normalized_function.getValue(index) < normalized_function.getValue(index + 1))) {
            index++;
        }
        c1 = normalized_function.getKey(index);

        index = int(n * 0.9);
        while ((max_local - normalized_function.getValue(index) > 4) || (normalized_function.getValue(index) < normalized_function.getValue(index - 1))) {
            index--;
        }
        c2 = normalized_function.getKey(index);
    }

    if (is_peak_unavailable) {
        a2 += coefficient / 2;
    }

    bool are_two_extremums = true;
    if (std::abs(c1 - c2) <= step) {
        unsigned int index = n;
        while (normalized_function.getValue(index - 1) < normalized_function.getValue(index)) {
            index--;
        }
        while ((normalized_function.getValue(index) <= normalized_function.getValue(index - 1)) && (index > (unsigned int)(n * 0.9))) {
            index--;
        }
        if (index <= n * 0.9) {
            index = 0;
            while(normalized_function.getValue(index + 1) < normalized_function.getValue(index)) {
                index++;
            }
            while ((normalized_function.getValue(index) <= normalized_function.getValue(index + 1)) && (index < n / 10)) {
                index++;
            }
            if (index >= n / 10) {
                are_two_extremums = false;
            } else {
                c1 = normalized_function.getKey(index);
                a1 = normalized_function.getValue(index);
                b1 = std::abs(-log(normalized_function.getValue(index / 2) / a1) / (pow(normalized_function.getKey(index / 2) - c1, 2)));
            }
        } else {
            c2 = normalized_function.getKey(index);
            a2 = normalized_function.getValue(index);
            b2 = std::abs(-log(normalized_function.getValue(index + (n - index) / 2) / a2) / (pow(normalized_function.getKey(index + (n - index) / 2) - c2, 2)));
        }
    }


    out << "a1 = " << a1 << ", b1 = " << b1 << ", c1 = " << c1 << '\n' << flush;
    if (are_two_extremums) {
        out << "a2 = " << a2 << ", b2 = " << b2 << ", c2 = " << c2 << "\n\n" << flush;
    } else {
        a1 /= 2;
        a2 /= 2;
        out << '\n';
    }

    double step1 = 0.5, step2 = 0.01, step3 = 0.1, min_value = INFINITY, previous_value, temp, temp2;
    if ((a1 > 1000) || (a2 > 1000)) {
        step1 = 2;
        step3 = 0.05;
    } else if ((a1 > 100) || (a2 > 100)) {
        step1 = 1;
        step3 = 0.1;
    } else if ((a1 > 10) || (a2 > 10)) {
        step1 = 0.5;
        step3 = 0.5;
    } else {
        step1 = 0.1;
        step3 = 1;
    }
    if ((b1 > 10) || (b2 > 10)) {
        step2 = 0.5;
    } else if ((b1 > 1) || (b2 > 1)) {
        step2 = 0.05;
    } else {
        step2 = 0.01;
    }
    if (!are_two_extremums) {
        step1 *= 2;
        step2 *= 2;
        step3 *= 2;
    }
    unsigned int numbers_of_points = n, counter = 0;
    if (n < numbers_of_points) {
        numbers_of_points = n;
    }
    if ((n > 150) && (!long_line)) {
        numbers_of_points = 50;
    } else if (n > 150) {
        numbers_of_points = 100;
    }
    uint8_t min_point_indexes[6];
    for (uint8_t i = 0; i < 6; ++i) {
        min_point_indexes[i] = 0;
    }
    bool q = true, need_center_correction = false;
    uint8_t center_correcter = 1;
    int little_changes_count = 0;
    long double reset[6];
    while (min_value > 5 * step) {
        q = false;
        previous_value = min_value;
        for (uint8_t i = 0; i < 5; ++i) {
            for (uint8_t j = 0; j < 5; ++j) {
                for (uint8_t k = 0; k < 5; ++k) {
                    for (uint8_t l = 0; l < 5; ++l) {
                        for (uint8_t m = 0; m < 5; ++m) {
                            for (uint8_t r = 0; r < 5; ++r) {
                                QApplication::processEvents();
                                temp = 0;
                                for (unsigned int p = 0; p < numbers_of_points; ++p) {
                                    center_correcter = 1;
                                    if ((p > n * 0.25) && (p < n * 0.75)) {
                                        if (need_center_correction) {
                                            center_correcter = 8;
                                        }
                                    }
                                    long double approximating_function = - std::abs(a1 - (2 - i) * step1) * exp(-std::abs(b1 - (2 - j) * step2) * pow(normalized_function.getKey(p * n / numbers_of_points) - c1 + (2 - k) * step3, 2)) - std::abs(a2 - (2 - l) * step1) * exp(-std::abs(b2 - (2 - m) * step2) * pow(normalized_function.getKey(p * n / numbers_of_points) - c2 + (2 - r) * step3, 2));
                                    if (((peakLine1.isIndexOutOfLine(p * n / numbers_of_points)) && (peakLine2.isIndexOutOfLine(p * n / numbers_of_points))) || (function.getValue(p * n / numbers_of_points) + approximating_function > 0)) {
                                        temp += center_correcter * std::abs(normalized_function.getValue(p * n / numbers_of_points) + approximating_function);
                                    }
                                }
                                if (temp < min_value) {
                                    q = true;
                                    min_value = temp;
                                    min_point_indexes[0] = i;
                                    min_point_indexes[1] = j;
                                    min_point_indexes[2] = k;
                                    min_point_indexes[3] = l;
                                    min_point_indexes[4] = m;
                                    min_point_indexes[5] = r;
                                }
                            }
                        }
                    }
                }
            }
        }
        if (!q) {
            step1 /= 10;
            step2 /= 10;
            step3 /= 10;
            if (step1 < 1e-20) {
                step1 = 0.5;
                step2 = 0.01;
                step3 = 0.1;

                if (((min_value > 42 * step) && (!long_line)) || ((long_line) && (min_value > 52 * step))) {
                    temp = min_value + 301;
                    reset[0] = a1;
                    reset[1] = b1;
                    reset[2] = c1;
                    reset[3] = a2;
                    reset[4] = b2;
                    reset[5] = c2;
                    while (temp - min_value > 300) {
                        a1 = reset[0] + (rand() % 100 - 50) / 1.0;
                        b1 = reset[1] + (rand() % 10 - 5) / 10.0;
                        c1 = reset[2] + (rand() % 10 - 5) / 1.0;
                        a2 = reset[3] + (rand() % 100 - 50) / 1.0;
                        b2 = reset[4] + (rand() % 10 - 5) / 10.0;
                        c2 = reset[5] + (rand() % 10 - 5) / 1.0;

                        temp = 0;
                        for (unsigned int p = 0; p < numbers_of_points; ++p) {
                            center_correcter = 1;
                            if ((p > n * 0.25) && (p < n * 0.75)) {
                                if (need_center_correction) {
                                    center_correcter = 8;
                                }
                            }
                            if ((peakLine1.isIndexOutOfLine(p * n / numbers_of_points)) && (peakLine2.isIndexOutOfLine(p * n / numbers_of_points))) {
                                temp += center_correcter * std::abs(normalized_function.getValue(p * n / numbers_of_points) - std::abs(a1) * exp(-std::abs(b1) * pow(normalized_function.getKey(p * n / numbers_of_points) - c1, 2)) - std::abs(a2) * exp(-std::abs(b2) * pow(normalized_function.getKey(p * n / numbers_of_points) - c2, 2)));
                            }
                        }
                    }
                }

                min_value = temp;
                counter = 0;
            }
            continue;
        }
        a1 -= (2 - min_point_indexes[0]) * step1;
        b1 -= (2 - min_point_indexes[1]) * step2;
        c1 -= (2 - min_point_indexes[2]) * step3;
        a2 -= (2 - min_point_indexes[3]) * step1;
        b2 -= (2 - min_point_indexes[4]) * step2;
        c2 -= (2 - min_point_indexes[5]) * step3;
        if ((std::abs(previous_value - min_value) < 0.0001) && (((min_value < 42 * step) && (!long_line)) || ((long_line) && (min_value < 52 * step)))) {
            little_changes_count++;
            if (little_changes_count >= 10) {
                break;
            }
        } else {
            little_changes_count = 0;
        }
        if ((min_point_indexes[0] % 4 != 0) && (min_point_indexes[1] % 4 != 0) && (min_point_indexes[2] % 4 != 0) && (min_point_indexes[3] % 4 != 0) && (min_point_indexes[4] % 4 != 0) && (min_point_indexes[5] % 4 != 0)) {
            step1 /= 10;
            step2 /= 10;
            step3 /= 10;
            //counter = 0;
        } else {
            counter++;
            if ((counter % 10 == 0) && (step2 < 0.01)) {
                step1 = 0.5 * (counter / 10);
                step2 = 0.01 * (counter / 10);
                step3 = 0.1 * (counter / 10);
                //counter = 0;
            }
            if ((counter % 100 == 0) && (min_value > 42 * step)) {
                step1 = 0.5;
                step2 = 0.01;
                step3 = 0.1;

                temp = min_value + 301;
                reset[0] = a1;
                reset[1] = b1;
                reset[2] = c1;
                reset[3] = a2;
                reset[4] = b2;
                reset[5] = c2;
                while (temp - min_value > 300) {
                    a1 = reset[0] + (rand() % 100 - 50) / 1.0;
                    b1 = reset[1] + (rand() % 10 - 5) / 10.0;
                    c1 = reset[2] + (rand() % 10 - 5) / 1.0;
                    a2 = reset[3] + (rand() % 100 - 50) / 1.0;
                    b2 = reset[4] + (rand() % 10 - 5) / 10.0;
                    c2 = reset[5] + (rand() % 10 - 5) / 1.0;

                    temp = 0;
                    for (unsigned int p = 0; p < numbers_of_points; ++p) {
                        center_correcter = 1;
                        if ((p > n * 0.25) && (p < n * 0.75)) {
                            if (need_center_correction) {
                                center_correcter = 8;
                            }
                        }
                        if ((peakLine1.isIndexOutOfLine(p * n / numbers_of_points)) && (peakLine2.isIndexOutOfLine(p * n / numbers_of_points))) {
                            temp += center_correcter * std::abs(normalized_function.getValue(p * n / numbers_of_points) - std::abs(a1) * exp(-std::abs(b1) * pow(normalized_function.getKey(p * n / numbers_of_points) - c1, 2)) - std::abs(a2) * exp(-std::abs(b2) * pow(normalized_function.getKey(p * n / numbers_of_points) - c2, 2)));
                        }
                    }
                }

                min_value = temp;
                counter = 0;
            }
        }
        temp = 0;
        if (!need_center_correction) {
            for (unsigned int p = 0; p < numbers_of_points; ++p) {
                center_correcter = 0;
                if ((p > n * 0.25) && (p < n * 0.75)) {
                    center_correcter = 2;
                }
                if ((peakLine1.isIndexOutOfLine(p * n / numbers_of_points)) && (peakLine2.isIndexOutOfLine(p * n / numbers_of_points))) {
                    temp += center_correcter * std::abs(normalized_function.getValue(p * n / numbers_of_points) - std::abs(a1) * exp(-std::abs(b1) * pow(normalized_function.getKey(p * n / numbers_of_points) - c1, 2)) - std::abs(a2) * exp(-std::abs(b2) * pow(normalized_function.getKey(p * n / numbers_of_points) - c2, 2)));
                }
            }
            if ((min_value < 21 * step) && (temp > 8 * step)) {
                need_center_correction = true;
                min_value = INFINITY;
            }
        }
        center_correcter = 1;
        temp = 0;
        temp2 = 0;

        for (unsigned int p = 0; p < numbers_of_points; ++p) {
            temp += center_correcter * std::abs(normalized_function.getValue(p * n / numbers_of_points) - std::abs(a1) * exp(-std::abs(b1) * pow(normalized_function.getKey(p * n / numbers_of_points) - c1, 2)));
            temp2 += center_correcter * std::abs(normalized_function.getValue(p * n / numbers_of_points) - std::abs(a2) * exp(-std::abs(b2) * pow(normalized_function.getKey(p * n / numbers_of_points) - c2, 2)));
        }
        if (abs(std::max(temp, temp2) - min_value) <= 5 * step) {
            if (temp == std::min(temp, temp2)) {
                a1 = a2 / 2.0;
                b1 = b2;
                c1 = c2;
                a2 = a1;
            } else {
                a2 = a1 / 2.0;
                b2 = b1;
                c2 = c1;
                a1 = a2;
            }
        }

        out << "a1 = " << a1 << ", b1 = " << b1 << ", c1 = " << c1 << '\n' << flush;
        out << "a2 = " << a2 << ", b2 = " << b2 << ", c2 = " << c2 << '\n' << flush;
        out << min_value << ' ' << previous_value << '\n' << flush;

        for (uint8_t i = 0; i < 6; ++i) {
            out << (int)min_point_indexes[i] << ' ' << flush;
        }
        out << '\n' << step1 << ' ' << step2 << ' ' << step3 << '\n' << flush;
        out << "\n\n" << flush;
    }

    a1 = std::abs(a1);
    a2 = std::abs(a2);
    b1 = std::abs(b1);
    b2 = std::abs(b2);


    out << "a1 = " << a1 << ", b1 = " << b1 << ", c1 = " << c1 << '\n' << flush;
    out << "a2 = " << a2 << ", b2 = " << b2 << ", c2 = " << c2 << '\n' << flush;

    return FunctionApproximation(a1, b1, c1, a2, b2, c2, coefficient);*/
}
