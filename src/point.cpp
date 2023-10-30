#include "point.h"

#include <cmath>
#include <iostream>

double Point::dist(const Point& other) const {
  double dx = other.x - x;
  double dy = other.y - y;
  return std::sqrt(dx * dx + dy * dy);
}

std::ostream& operator<<(std::ostream& os, const Point& p) {
  return os << "(" << p.x << ", " << p.y << ")";
}
