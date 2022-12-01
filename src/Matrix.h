//
// Created by edgar on 01-12-22.
//

#ifndef ARDUINO_KIRCHHOFF_MATRIX_H
#define ARDUINO_KIRCHHOFF_MATRIX_H

template <typename T>
class Matrix{
    private:
        T **array;
        T **identity;
        const unsigned int rows;
        const unsigned int columns;

    public:
        Matrix(const unsigned int rows, const unsigned int columns) {
            this->array = new T *[rows];
            for (short int i = 0; i < rows; ++i)
                this->array[i] = new T[columns];
            this->rows = rows;
            this->columns = columns;
        }

        virtual ~Matrix() {
            for (int i = 0; i < rows; ++i)
                delete[] array[i];
            delete[] array;
        }

        void set(short int row, short int column, T *value) {
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < columns; ++j) {
                    if (row == i && column == j) {
                        array[i][j] = &value;
                        break;
                    }
                }
            }
        }

        T get(short int row, short int column) {
            for (int i = 0; i < rows; ++i) {
                for (int j = 0; j < columns; ++j) {
                    if (row == i && column == j) {
                        return array[i][j];
                    }
                }
            }
        }

        void solve() {
            T pivote, aux;
            // reducción por reglones
            for (short int i = 0; i < rows; i++) {
                pivote = array[i][i];
                // convertir el pivote matrix uno y aplicar las operaciones sobre toda la fila.
                for (short int k = 0; k < columns; k++) {
                    array[i][k] = array[i][k] / pivote;
                }
                for (short int j = 0; j < rows; j++) {
                    if (i != j) { // si no es la diagonal.
                        aux = array[j][i];
                        for (short int k = 0; k < columns; k++) { // convertir a cero toda la columna del pivote.
                            array[j][k] = array[j][k] - aux * array[i][k];
                        }
                    }
                }
            }
        }
};


#endif //ARDUINO_KIRCHHOFF_MATRIX_H
