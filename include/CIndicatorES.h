/* 
 * File:   CIndicatorES.h
 * Author: https://www.mql5.com/ru/articles/318
 *
 * Created on January 20, 2017, 4:14 PM
 */

#ifndef CINDICATORES_H
#define CINDICATORES_H

#include "PowellsMethod.h"

struct TForecastModelParams{
    double S;
    double t;
    double Alpha;
    double Gamma;
    double Phi;
    
    void SetDefault( const double aS ) {
        S = aS;
        t = 0.0;
        Alpha = 0.5;
        Gamma = 0.5;
        Phi = 0.5;    
    }
};

//-----------------------------------------------------------------------------------
// Class CIndicatorES
//-----------------------------------------------------------------------------------
class CIndicatorES : public PowellsMethod {
    protected:
        TDoubles Dat;                                              // Input data
        size_t Dlen;                                               // Data length
        TDoubles Par;                                             // Parameters
    public:
        CIndicatorES(): PowellsMethod(), Par( 5.0 ) { }
        bool CalcPar( const TDoubles &dat, const TForecastModelParams &aParams );
        double GetPar( const int n );
        
    private:
        double func( const TDoubles &p ) override;
};

#endif /* CINDICATORES_H */
