#pragma once

#include <ostream>

class Point {
public:
  Point() : x(0), y(0) {}
  Point(double _x, double _y) : x(_x), y(_y) {}

  double dist(const Point& other) const;

  double x;
  double y;
};

std::ostream& operator<<(std::ostream& os, const Point& point);
