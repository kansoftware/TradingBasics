/**
 * \file PowellsMethod.h
 * \brief Модуль с шаблонами для работы с указателями
 * \author https://www.mql5.com/ru/articles/318
 * \since 2017-01-20
 */

#ifndef POWELLSMETHOD_H
#define POWELLSMETHOD_H

#include <vector>
typedef std::vector<double> TDoubles;

class PowellsMethod {
    public:
        PowellsMethod();
        void   SetItMaxPowell(int n)           { ItMaxPowell=n; }
        void   SetFtolPowell(double er)        { FtolPowell=er; }
        void   SetItMaxBrent(int n)            { ItMaxBrent=n;  }
        void   SetFtolBrent(double er)         { FtolBrent=er;  }
        int    Optimize(TDoubles &p, const size_t n=0);
        double GetFret(void)                   { return(Fret);  }
        int    GetIter(void)                   { return(Iter);  }

    protected:
        TDoubles P,Xi;
        TDoubles Pcom,Xicom,Xt;
        TDoubles Pt,Ptt,Xit;
        size_t    N;
        double Fret;
        int    Iter;
        int    ItMaxPowell;
        double FtolPowell;
        int    ItMaxBrent;
        double FtolBrent;
        int    MaxIterFlag;

    private:
        void   powell(void);
        void   linmin(void);
        void   mnbrak(double &ax,double &bx,double &cx,double &fa,double &fb,double &fc);
        double brent(double ax,double bx,double cx,double &xmin);
        double f1dim(double x);
        virtual double func(const TDoubles &) { return(0); }
};

#endif /* POWELLSMETHOD_H */
