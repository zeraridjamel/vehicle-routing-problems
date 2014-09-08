#ifndef SOLUTION_H
#define SOLUTION_H

#include <deque>
#include <cmath>

#include "prob_pd.h"
#include "vehicle.h"
//#include "bucketn.h"

const double EPSILON = 0.001;

class Solution: public Prob_pd {
  protected:
    std::deque<Vehicle> fleet;


    double totalDistance;
    double totalCost;
    double w1,w2,w3;

  public:
/*    Solution() {
        totalDistance = 0;
        totalCost = 0;
        fleet.clear();
    };
*/
    Solution(const Prob_pd& P):Prob_pd(P){}; 


    void setweights(double _w1,double _w2,double _w3) {w1=_w1;w2=_w2;w3=_w3;};
    void dump();
    void dumproutes();
    void tau() ;
    void plot(std::string file,std::string title);

    void computeCosts();
    double getcost();
    double getDistance();
    double getAverageRouteDurationLength();

    Solution& operator=( const Solution& rhs ) {
        if ( this != &rhs ) {
            totalDistance = rhs.totalDistance;
            totalCost = rhs.totalCost;
            fleet = rhs.fleet;
        }
        return *this;
    };

    bool operator == (Solution &another) const {
        return fleet.size() == another.fleet.size() &&
               std::abs(totalCost - another.totalCost) < EPSILON;
    };

   bool solutionEquivalent (Solution &another)  {
        computeCosts();
        another.computeCosts();
        return fleet.size() == another.fleet.size() &&
               std::abs(totalCost - another.totalCost) < EPSILON;

    };

    bool operator <  (Solution &another) const {
        return fleet.size() < another.fleet.size() || totalCost < another.totalCost;

    

    };
};

#endif
