// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/* 
 * File:   CIndicatorES.cpp
 * Author: victorg(mq5), kan(c++)
 * 
 * Created on January 20, 2017, 4:14 PM
 */
#include <cmath>
#include "DelphisRound.h"

#include "CIndicatorES.h"

//-----------------------------------------------------------------------------------
// CalcPar
//-----------------------------------------------------------------------------------

double CIndicatorES::GetPar(const int n) {
    if (n >= 0 and n < 5) {
        return ( Par[ ToSize_t(n) ]);
    } else {
        return ( 0.0);
    }
}

bool CIndicatorES::CalcPar(const TDoubles &dat, const TForecastModelParams &aParams) {
    Dlen = dat.size();
    Dat = dat;

    Par[0] = aParams.S; // Dat[0]; // initial S
    Par[1] = aParams.t; // 0; // initial T
    Par[2] = aParams.Alpha; // 0.5; // initial Alpha
    Par[3] = aParams.Gamma; // 0.5; // initial Gamma
    Par[4] = aParams.Phi; // 0.5; // initial Phi

    return Optimize(Par) > 0.0; // Powell's optimization
}

//------------------------------------------------------------------------------------
// func
//------------------------------------------------------------------------------------

double CIndicatorES::func(const TDoubles &p) {
    double s, t, alp, gam, phi, k1, k2, k3, e, sse, ae, pt;

    s = p[0];
    t = p[1];
    alp = p[2];
    gam = p[3];
    phi = p[4];
    k1 = 1;
    k2 = 1;
    k3 = 1;
    if (alp > 0.95) {
        k1 += (alp - 0.95)*200.0;
        alp = 0.95;
    }// Alpha   > 0.95
    else if (alp < 0.05) {
        k1 += (0.05 - alp)*200.0;
        alp = 0.05;
    } // Alpha   < 0.05

    if (gam > 0.95) {
        k2 += (gam - 0.95)*200.0;
        gam = 0.95;
    }// Gamma   > 0.95
    else if (gam < 0.05) {
        k2 += (0.05 - gam)*200.0;
        gam = 0.05;
    } // Gamma   < 0.05

    if (phi > 1.0) {
        k3 += (phi - 1.0)*200.0;
        phi = 1.0;
    }// Phi     > 1.0
    else if (phi < 0.05) {
        k3 += (0.05 - phi)*200;
        phi = 0.05;
    } // Phi     < 0.05

    sse = 0;
    for (size_t i = 0; i < Dlen; i++) {
        e = Dat[ i ]-(s + phi * t);
        sse += e*e;
        ae = alp*e;
        pt = phi*t;
        s = s + pt + ae;
        t = pt + gam*ae;
    }

    return ( ToDouble(Dlen) * log(k1 * k2 * k3 * sse));
}
//--------------------------------------------------------------------------------