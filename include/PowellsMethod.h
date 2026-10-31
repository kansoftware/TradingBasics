/**
 * \file PowellsMethod.h
 * \brief Метод Пауэлла безусловной минимизации функции N переменных
 * \author https://www.mql5.com/ru/articles/318
 * \since 2017-01-20
 *
 * Портирование классического алгоритма Пауэлла (Numerical Recipes, mq5-версия
 * victorg) на C++. Наследник переопределяет виртуальную func() — минимизируемую
 * функцию, и вызывает Optimize() со стартовой точкой. Рабочие буферы (P, Xi, Pt
 * и пр.) — члены класса: один экземпляр не предназначен для параллельного
 * вызова Optimize() из разных потоков.
 *
 * Сообщения об ошибках выводятся через printf в stdout по мере
 * возникновения: "powell exceeding maximum iterations!" (исчерпан лимит
 * итераций Пауэлла) и "Too many iterations in brent" (исчерпан лимит итераций
 * одномерного поиска). Код возврата Optimize() при этом -1, исключений нет.
 */

#ifndef POWELLSMETHOD_H
#define POWELLSMETHOD_H

#include <vector>
typedef std::vector<double> TDoubles;   /*!< Рабочий тип вектора параметров */

/**
 *  \class PowellsMethod
 *  \brief Численная минимизация методом Пауэлла (сопряжённые направления)
 *
 *  Контракт использования:
 *   - наследник переопределяет func(const TDoubles&) — минимизируемую функцию
 *     (по умолчанию возвращает 0);
 *   - при необходимости настраиваются лимиты/точности сеттерами
 *     (SetItMaxPowell/SetFtolPowell/SetItMaxBrent/SetFtolBrent);
 *   - Optimize(p[, n]) запускает оптимизацию от стартовой точки p и записывает
 *     найденный минимум обратно в p.
 *
 *  Коды возврата Optimize():
 *   - 0  — некорректная размерность (пустой вектор, N < 1 или N > p.size());
 *   - -1 — достигнут лимит итераций (MaxIterFlag=1), результат — лучшая
 *          найденная точка; диагностика через printf;
 *   - N > 0 — успех: N = число выполненных итераций Пауэлла (== GetIter()).
 */
class PowellsMethod {
    public:
        PowellsMethod();
        void   SetItMaxPowell(int n)           { ItMaxPowell=n; }   /*!< Лимит итераций Пауэлла (по умолчанию 200) */
        void   SetFtolPowell(double er)        { FtolPowell=er; }   /*!< Относительная точность сходимости Пауэлла (1e-6) */
        void   SetItMaxBrent(int n)            { ItMaxBrent=n;  }   /*!< Лимит итераций одномерного поиска Брента (200) */
        void   SetFtolBrent(double er)         { FtolBrent=er;  }   /*!< Относительная точность Брента (1e-4) */
        int    Optimize(TDoubles &p, const size_t n=0);    /*!< \brief Минимизация от стартовой точки; см. коды возврата в описании класса */
        double GetFret(void)                   { return(Fret);  } /*!< Значение func() в найденной точке */
        int    GetIter(void)                   { return(Iter);  } /*!< Число выполненных итераций Пауэлла */

    protected:
        TDoubles P,Xi;                  /*!< Текущая точка и набор направлений (N x N, столбцами) */
        TDoubles Pcom,Xicom,Xt;         /*!< Рабочие векторы одномерного поиска вдоль направления */
        TDoubles Pt,Ptt,Xit;            /*!< Лучшая точка цикла, экстраполяционная точка, текущее направление */
        size_t    N;                    /*!< Размерность задачи */
        double Fret;                    /*!< Минимальное найденное значение func() */
        int    Iter;                    /*!< Число выполненных итераций Пауэлла */
        int    ItMaxPowell;             /*!< Лимит итераций Пауэлла */
        double FtolPowell;              /*!< Относительная точность сходимости Пауэлла */
        int    ItMaxBrent;              /*!< Лимит итераций Брента */
        double FtolBrent;               /*!< Относительная точность Брента */
        int    MaxIterFlag;             /*!< 1, если лимит итераций исчерпан (код возврата -1) */

    private:
        void   powell(void);            /*!< Основной цикл метода Пауэлла */
        void   linmin(void);            /*!< Минимизация вдоль текущего направления (mnbrak + brent) */
        void   mnbrak(double &ax,double &bx,double &cx,double &fa,double &fb,double &fc);   /*!< Брекетинг минимума (GOLD/CGOLD/GLIMIT) */
        double brent(double ax,double bx,double cx,double &xmin);   /*!< Одномерный поиск Брента на брекете */
        double f1dim(double x);         /*!< func() в точке Pcom + x*Xicom */
        virtual double func(const TDoubles &) { return(0); }    /*!< \brief Минимизируемая функция — переопределяется наследником */
};

#endif /* POWELLSMETHOD_H */
