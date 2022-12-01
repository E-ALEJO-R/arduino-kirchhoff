//
// Created by edgar on 30-11-22.
//

#ifndef ARDUINO_KIRCHHOFF_FRACTION_H
#define ARDUINO_KIRCHHOFF_FRACTION_H


class Fraction{
    private:
        int numerator;
        int denominator;
    public:
        Fraction(int numerator, int denominator) {
            this->numerator = numerator;
            this->denominator = denominator;
        }

        Fraction operator+(const Fraction &p) const {
            return Fraction(this->numerator + p.numerator, this->denominator + p.numerator);
        }

        Fraction operator-(const Fraction &p) const {
            return Fraction(this->numerator - p.numerator, this->denominator - p.numerator);
        }
};


#endif //ARDUINO_KIRCHHOFF_FRACTION_H
