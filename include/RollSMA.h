/*** 
 * File:   RollSMA.h
 * Author: kan
 *
 * Created on November 13, 2019, 12:55 PM
 * \date 2019-11-26
 * The idea is from https://bitbucket.org/quanttools/quanttools/src/master/inst/include/Indicators/Sma.h
 */

#ifndef ROLLSMA_H
#define ROLLSMA_H

#include <queue>
#include <stdexcept>
#include <cmath>

class RollSMA {
    private:
        double fSum;
        const std::size_t fN;
        std::queue< double > fWindow;

    public:
        RollSMA( const int n ) :
            fSum( 0.0 ),
            fN( static_cast<size_t>(n) ) {
                if (n < 1) throw std::invalid_argument("n must be greater than 0");
            }

        void Add( double value ) {

            fSum += value;
            fWindow.push(value);

            if (fWindow.size() > fN) {
                fSum -= fWindow.front();
                fWindow.pop();
            }
        }

        bool IsFormed() const {
            return fWindow.size() == fN;
        }

        double GetValue() const {
            return ((fWindow.size() == fN) ? (fSum / static_cast<double>(fN)) : NAN);
        }

        void Reset() {
            fSum = 0.0;
            fWindow = {};
        }
};

#endif /* ROLLSMA_H */
